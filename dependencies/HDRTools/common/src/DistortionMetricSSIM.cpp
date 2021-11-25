/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Apple Inc.
 * <ORGANIZATION> = Apple Inc.
 * <YEAR> = 2015
 *
 * Copyright (c) 2015, Apple Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the <ORGANIZATION> nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!
 *************************************************************************************
 * \file DistortionMetricSSIM.cpp
 *
 * \brief
 *    SSIM (Structural Similarity Index Metric) distortion computation Class
 *    Code is based on the JM implementation written by Dr. Peshala Pahalawatta
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "DistortionMetricSSIM.H"
#include <string.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

namespace hdrtoolslib {

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

DistortionMetricSSIM::DistortionMetricSSIM(const FrameFormat *format, SSIMParams *params, double maxSampleValue, bool enableHexMetric)
: DistortionMetric()
{
  // Metric parameters
  m_K1 = params->m_K1;
  m_K2 = params->m_K2;
  
  m_blockDistance   = params->m_blockDistance;
  m_blockSizeX      = params->m_blockSizeX;
  m_blockSizeY      = params->m_blockSizeY;
  m_useLogSSIM      = params->m_useLog;
  m_enableHexMetric = enableHexMetric;
  
#ifdef UNBIASED_VARIANCE
  m_bias = 1;
#else
  m_bias = 0;
#endif

  m_colorSpace    = format->m_colorSpace;
  
  for (int c = 0; c < T_COMP; c++) {
    m_metric[c] = 0.0;
    m_metricStats[c].reset();
    m_maxValue[c] = (double) maxSampleValue;
    m_hexMetricSSIM[c] = 0;
  }
}

DistortionMetricSSIM::~DistortionMetricSSIM()
{
}

//-----------------------------------------------------------------------------
// Private methods (non-MMX)
//-----------------------------------------------------------------------------
float DistortionMetricSSIM::compute (float *inp0Data, float *inp1Data, int height, int width, int windowHeight, int windowWidth, int comp, float maxPixelValue)
{
  double maxPixelValueSquared = (double) (maxPixelValue * maxPixelValue);
  double C1 = m_K1 * m_K1 * maxPixelValueSquared;
  double C2 = m_K2 * m_K2 * maxPixelValueSquared;
  double windowPixels = (double) (windowWidth * windowHeight);
  double windowPixelsBias = windowPixels - m_bias;
  
  double blockSSIM = 0.0, meanInp0 = 0.0, meanInp1 = 0.0;
  double varianceInp0 = 0.0, varianceInp1 = 0.0, covariance = 0.0;
 
  double distortion = 0.0;
  int i, j, n, m, windowCounter = 0;
  float *inp0Line = NULL;
  float *inp1Line = NULL;
  
  double inp0, inp1;
  
  for (j = 0; j <= height - windowHeight; j += m_blockDistance) {
    for (i = 0; i <= width - windowWidth; i += m_blockDistance) {
      double sumInp0 = 0.0;
      double sumInp1 = 0.0;
      double sumSquareInp0  = 0.0;
      double sumSquareInp1  = 0.0;
      double sumMultiInp0Inp1 = 0.0;
      
      for ( n = j; n < j + windowHeight;n++) {
        inp0Line = &inp0Data[n * width + i];
        inp1Line = &inp1Data[n * width + i];
        for (m = ZERO; m < windowWidth; m++) {
          inp0              = (double) *inp0Line++;
          inp1              = (double) *inp1Line++;
          sumInp0          += inp0;
          sumInp1          += inp1;
          sumSquareInp0    += inp0 * inp0;
          sumSquareInp1    += inp1 * inp1;
          sumMultiInp0Inp1 += inp0 * inp1;
        }
      }
      
      meanInp0 = sumInp0 / windowPixels;
      meanInp1 = sumInp1 / windowPixels;

      varianceInp0 = (sumSquareInp0    - sumInp0 * meanInp0) / windowPixelsBias;
      varianceInp1 = (sumSquareInp1    - sumInp1 * meanInp1) / windowPixelsBias;
      covariance   = (sumMultiInp0Inp1 - sumInp0 * meanInp1) / windowPixelsBias;

      blockSSIM  = (2.0 * meanInp0 * meanInp1 + C1) * (2.0 * covariance + C2);
      blockSSIM /=  ((meanInp0 * meanInp0 + meanInp1 * meanInp1 + C1) * (varianceInp0 + varianceInp1 + C2));
      
      distortion += blockSSIM;
      windowCounter++;
    }
  }

  distortion /= dMax(1.0, (double) windowCounter);
  
  if (distortion >= 1.0 && distortion < 1.01) // avoid float accuracy problem at very low QP(e.g.2)
    distortion = 1.0;
  
  if (m_useLogSSIM)
    return (float) ssimSNR(distortion);
  else
    return (float) distortion;
}


float DistortionMetricSSIM::compute (uint8 *inp0Data, uint8 *inp1Data, int height, int width, int windowHeight, int windowWidth, int comp, int maxPixelValue)
{
  double maxPixelValueSquared = (double) (maxPixelValue * maxPixelValue);
  double C1 = m_K1 * m_K1 * maxPixelValueSquared;
  double C2 = m_K2 * m_K2 * maxPixelValueSquared;
  double windowPixels = (double) (windowWidth * windowHeight);
  double windowPixelsBias = windowPixels - m_bias;
  
  double blockSSIM = 0.0, meanInp0 = 0.0, meanInp1 = 0.0;
  double varianceInp0 = 0.0, varianceInp1 = 0.0, covariance = 0.0;
 
  double distortion = 0.0;
  int i, j, n, m, windowCounter = 0;
  uint8 *inp0Line = NULL;
  uint8 *inp1Line = NULL;
  
  double inp0, inp1;
  
  for (j = 0; j <= height - windowHeight; j += m_blockDistance) {
    for (i = 0; i <= width - windowWidth; i += m_blockDistance) {
      double sumInp0 = 0.0;
      double sumInp1 = 0.0;
      double sumSquareInp0  = 0.0;
      double sumSquareInp1  = 0.0;
      double sumMultiInp0Inp1 = 0.0;
      
      for ( n = j; n < j + windowHeight;n++) {
        inp0Line = &inp0Data[n * width + i];
        inp1Line = &inp1Data[n * width + i];
        for (m = ZERO; m < windowWidth; m++) {
          inp0              = (double) *inp0Line++;
          inp1              = (double) *inp1Line++;
          sumInp0          += inp0;
          sumInp1          += inp1;
          sumSquareInp0    += inp0 * inp0;
          sumSquareInp1    += inp1 * inp1;
          sumMultiInp0Inp1 += inp0 * inp1;
        }
      }
      
      meanInp0 = sumInp0 / windowPixels;
      meanInp1 = sumInp1 / windowPixels;

      varianceInp0 = (sumSquareInp0    - sumInp0 * meanInp0) / windowPixelsBias;
      varianceInp1 = (sumSquareInp1    - sumInp1 * meanInp1) / windowPixelsBias;
      covariance   = (sumMultiInp0Inp1 - sumInp0 * meanInp1) / windowPixelsBias;

      blockSSIM  = (2.0 * meanInp0 * meanInp1 + C1) * (2.0 * covariance + C2);
      blockSSIM /=  ((meanInp0 * meanInp0 + meanInp1 * meanInp1 + C1) * (varianceInp0 + varianceInp1 + C2));
      
      distortion += blockSSIM;
      windowCounter++;
    }
  }

  distortion /= dMax(1.0, (double) windowCounter);
  
  if (distortion >= 1.0 && distortion < 1.01) // avoid float accuracy problem at very low QP(e.g.2)
    distortion = 1.0;
  
  if (m_useLogSSIM)
    return (float) ssimSNR(distortion);
  else
    return (float) distortion;
}


float DistortionMetricSSIM::compute (uint16 *inp0Data, uint16 *inp1Data, int height, int width, int windowHeight, int windowWidth, int comp, int maxPixelValue)
{
  double maxPixelValueSquared = (double) (maxPixelValue * maxPixelValue);
  double C1 = m_K1 * m_K1 * maxPixelValueSquared;
  double C2 = m_K2 * m_K2 * maxPixelValueSquared;
  double windowPixels = (double) (windowWidth * windowHeight);
  double windowPixelsBias = windowPixels - m_bias;
  
  double blockSSIM = 0.0, meanInp0 = 0.0, meanInp1 = 0.0;
  double varianceInp0 = 0.0, varianceInp1 = 0.0, covariance = 0.0;
 
  double distortion = 0.0;
  int i, j, n, m, windowCounter = 0;
  uint16 *inp0Line = NULL;
  uint16 *inp1Line = NULL;
  
  double inp0, inp1;
  
  for (j = 0; j <= height - windowHeight; j += m_blockDistance) {
    for (i = 0; i <= width - windowWidth; i += m_blockDistance) {
      double sumInp0 = 0.0;
      double sumInp1 = 0.0;
      double sumSquareInp0  = 0.0;
      double sumSquareInp1  = 0.0;
      double sumMultiInp0Inp1 = 0.0;
      
      for ( n = j; n < j + windowHeight;n++) {
        inp0Line = &inp0Data[n * width + i];
        inp1Line = &inp1Data[n * width + i];
        for (m = ZERO; m < windowWidth; m++) {
          inp0              = (double) *inp0Line++;
          inp1              = (double) *inp1Line++;
          sumInp0          += inp0;
          sumInp1          += inp1;
          sumSquareInp0    += inp0 * inp0;
          sumSquareInp1    += inp1 * inp1;
          sumMultiInp0Inp1 += inp0 * inp1;
        }
      }
      
      meanInp0 = sumInp0 / windowPixels;
      meanInp1 = sumInp1 / windowPixels;

      varianceInp0 = (sumSquareInp0    - sumInp0 * meanInp0) / windowPixelsBias;
      varianceInp1 = (sumSquareInp1    - sumInp1 * meanInp1) / windowPixelsBias;
      covariance   = (sumMultiInp0Inp1 - sumInp0 * meanInp1) / windowPixelsBias;

      blockSSIM  = (2.0 * meanInp0 * meanInp1 + C1) * (2.0 * covariance + C2);
      blockSSIM /=  ((meanInp0 * meanInp0 + meanInp1 * meanInp1 + C1) * (varianceInp0 + varianceInp1 + C2));
      
      distortion += blockSSIM;
      windowCounter++;
    }
  }

  distortion /= dMax(1.0, (double) windowCounter);
  
  if (distortion >= 1.0 && distortion < 1.01) // avoid float accuracy problem at very low QP(e.g.2)
    distortion = 1.0;
  
  if (m_useLogSSIM)
    return (float) ssimSNR(distortion);
  else
    return (float) distortion;
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void DistortionMetricSSIM::computeMetric (Frame* inp0, Frame* inp1)
{
  // it is assumed here that the frames are of the same type
  if (inp0->equalType(inp1)) {
    if (inp0->m_isFloat == TRUE) {    // floating point data
      for (int c = Y_COMP; c < inp0->m_noComponents; c++) {
        m_metric[c] = (double) compute(inp0->m_floatComp[c], inp1->m_floatComp[c], inp0->m_height[c], inp0->m_width[c], m_blockSizeY, m_blockSizeX, c, (float) m_maxValue[c]);
        m_metricStats[c].updateStats(m_metric[c]);
      }
    }
    else if (inp0->m_bitDepth == 8) {   // 8 bit data
      for (int c = Y_COMP; c < inp0->m_noComponents; c++) {
        m_metric[c] = (double) compute(inp0->m_comp[c], inp1->m_comp[c], inp0->m_height[c], inp0->m_width[c], m_blockSizeY, m_blockSizeX, c, (int) m_maxValue[c]);

        m_metricStats[c].updateStats(m_metric[c]);
      }
    }
    else { // 16 bit data
      for (int c = Y_COMP; c < inp0->m_noComponents; c++) {
        m_metric[c] = (double) compute(inp0->m_ui16Comp[c], inp1->m_ui16Comp[c], inp0->m_height[c], inp0->m_width[c], m_blockSizeY, m_blockSizeX, c, (int) m_maxValue[c]);
        m_metricStats[c].updateStats(m_metric[c]);
      }
    }
  }
  else {
    printf("Frames of different type being compared. Computation will not be performed for this frame.\n");
  }
}


void DistortionMetricSSIM::computeMetric (Frame* inp0, Frame* inp1, int component)
{
  // it is assumed here that the frames are of the same type
  // Currently no TF is applied on the data. However, we may wish to apply some TF, e.g. PQ or combination of PQ and PH.
  
  if (inp0->equalType(inp1)) {
    if (inp0->m_isFloat == TRUE) {    // floating point data
      m_metric[component] = (double) compute(inp0->m_floatComp[component], inp1->m_floatComp[component], inp0->m_height[component], inp0->m_width[component], m_blockSizeY, m_blockSizeX, component, (float) m_maxValue[component]);
      m_metricStats[component].updateStats(m_metric[component]);
    }
    else if (inp0->m_bitDepth == 8) {   // 8 bit data
      m_metric[component] = (double) compute(inp0->m_comp[component], inp1->m_comp[component], inp0->m_height[component], inp0->m_width[component], m_blockSizeY, m_blockSizeX, component, (int) m_maxValue[component]);
      
      m_metricStats[component].updateStats(m_metric[component]);
    }
    else { // 16 bit data
      m_metric[component] = (double) compute(inp0->m_ui16Comp[component], inp1->m_ui16Comp[component], inp0->m_height[component], inp0->m_width[component], m_blockSizeY, m_blockSizeX, component, (int) m_maxValue[component]);
      m_metricStats[component].updateStats(m_metric[component]);
    }
  }
  else {
    printf("Frames of different type being compared. Computation will not be performed for this frame.\n");
  }
}

void DistortionMetricSSIM::reportMetric  ()
{
  printf("%10.5f %10.5f %10.5f ", m_metric[Y_COMP], m_metric[U_COMP], m_metric[V_COMP]);
  if (m_enableHexMetric == TRUE) {
    for (int c = 0; c < T_COMP; c++) {
      std::copy(reinterpret_cast<uint8_t *>(&m_metric[c]),
                reinterpret_cast<uint8_t *>(&m_metric[c]) + sizeof(m_metric[c]),
                reinterpret_cast<uint8_t *>(&m_hexMetricSSIM[c]));
    }
    printf(" %16" PRIx64 " %16" PRIx64 " %16" PRIx64, m_hexMetricSSIM[Y_COMP], m_hexMetricSSIM[U_COMP], m_hexMetricSSIM[V_COMP]);
  }
}

void DistortionMetricSSIM::reportSummary  ()
{
  printf("%10.5f %10.5f %10.5f ", m_metricStats[Y_COMP].getAverage(), m_metricStats[U_COMP].getAverage(), m_metricStats[V_COMP].getAverage());
  if (m_enableHexMetric == TRUE) {
    for (int c = 0; c < T_COMP; c++) {
      double val = m_metricStats[c].getAverage();
      std::copy(reinterpret_cast<uint8_t *>(&val),
                reinterpret_cast<uint8_t *>(&val) + sizeof(val),
                reinterpret_cast<uint8_t *>(&m_hexMetricSSIM[c]));
    }
    printf(" %16" PRIx64 " %16" PRIx64 " %16" PRIx64, m_hexMetricSSIM[Y_COMP], m_hexMetricSSIM[U_COMP], m_hexMetricSSIM[V_COMP]);
  }
}

void DistortionMetricSSIM::reportMinimum  ()
{
  printf("%10.5f %10.5f %10.5f ", m_metricStats[Y_COMP].minimum, m_metricStats[U_COMP].minimum, m_metricStats[V_COMP].minimum);
  if (m_enableHexMetric == TRUE) {
    for (int c = 0; c < T_COMP; c++) {
      std::copy(reinterpret_cast<uint8_t *>(&m_metricStats[c].minimum),
                reinterpret_cast<uint8_t *>(&m_metricStats[c].minimum) + sizeof(m_metricStats[c].minimum),
                reinterpret_cast<uint8_t *>(&m_hexMetricSSIM[c]));
    }
    printf(" %16" PRIx64 " %16" PRIx64 " %16" PRIx64, m_hexMetricSSIM[Y_COMP], m_hexMetricSSIM[U_COMP], m_hexMetricSSIM[V_COMP]);
  }
}

void DistortionMetricSSIM::reportMaximum  ()
{
  printf("%10.5f %10.5f %10.5f ", m_metricStats[Y_COMP].maximum, m_metricStats[U_COMP].maximum, m_metricStats[V_COMP].maximum);
  if (m_enableHexMetric == TRUE) {
    for (int c = 0; c < T_COMP; c++) {
      std::copy(reinterpret_cast<uint8_t *>(&m_metricStats[c].maximum),
                reinterpret_cast<uint8_t *>(&m_metricStats[c].maximum) + sizeof(m_metricStats[c].maximum),
                reinterpret_cast<uint8_t *>(&m_hexMetricSSIM[c]));
    }
    printf(" %16" PRIx64 " %16" PRIx64 " %16" PRIx64, m_hexMetricSSIM[Y_COMP], m_hexMetricSSIM[U_COMP], m_hexMetricSSIM[V_COMP]);
  }
}

void DistortionMetricSSIM::printHeader()
{
  if (m_isWindow == FALSE ) {
    switch (m_colorSpace) {
      case CM_YCbCr:
        printf("   SSIM-Y  "); // 11
        printf("   SSIM-U  "); // 11
        printf("   SSIM-V  "); // 11
        if (m_enableHexMetric == TRUE) {
          printf("    hexSSIM-Y    "); // 17
          printf("    hexSSIM-U    "); // 17
          printf("    hexSSIM-V    "); // 17
        }
        break;
      case CM_RGB:
        printf("   SSIM-R  "); // 11
        printf("   SSIM-G  "); // 11
        printf("   SSIM-B  "); // 11
        if (m_enableHexMetric == TRUE) {
          printf("    hexSSIM-R    "); // 17
          printf("    hexSSIM-G    "); // 17
          printf("    hexSSIM-B    "); // 17
        }
        break;
      case CM_XYZ:
        printf("   SSIM-X  "); // 11
        printf("   SSIM-Y  "); // 11
        printf("   SSIM-Z  "); // 11
        if (m_enableHexMetric == TRUE) {
          printf("   hexMSSSIM-Y   "); // 17
          printf("   hexMSSSIM-U   "); // 17
          printf("   hexMSSSIM-V   "); // 17
        }
        break;
      default:
        printf("   SSIM-C0 "); // 11
        printf("   SSIM-C1 "); // 11
        printf("   SSIM-C2 "); // 11
        if (m_enableHexMetric == TRUE) {
          printf("    hexSSIM-C0   "); // 17
          printf("    hexSSIM-C1   "); // 17
          printf("    hexSSIM-C2   "); // 17
        }
        break;
    }
  }
  else {
    switch (m_colorSpace) {
      case CM_YCbCr:
        printf("  wSSIM-Y  "); // 11
        printf("  wSSIM-U  "); // 11
        printf("  wSSIM-V  "); // 11
        if (m_enableHexMetric == TRUE) {
          printf("    hexwSSIM-Y   "); // 17
          printf("    hexwSSIM-U   "); // 17
          printf("    hexwSSIM-V   "); // 17
        }
        break;
      case CM_RGB:
        printf("  wSSIM-R  "); // 11
        printf("  wSSIM-G  "); // 11
        printf("  wSSIM-B  "); // 11
        if (m_enableHexMetric == TRUE) {
          printf("    hexwSSIM-R   "); // 17
          printf("    hexwSSIM-G   "); // 17
          printf("    hexwSSIM-B   "); // 17
        }
        break;
      case CM_XYZ:
        printf("  wSSIM-X  "); // 11
        printf("  wSSIM-Y  "); // 11
        printf("  wSSIM-Z  "); // 11
        if (m_enableHexMetric == TRUE) {
          printf("    hexwSSIM-X   "); // 17
          printf("    hexwSSIM-Y   "); // 17
          printf("    hexwSSIM-Z   "); // 17
        }
        break;
      default:
        printf("  wSSIM-C0 "); // 11
        printf("  wSSIM-C1 "); // 11
        printf("  wSSIM-C2 "); // 11
        if (m_enableHexMetric == TRUE) {
          printf("   hexwSSIM-C0   "); // 17
          printf("   hexwSSIM-C1   "); // 17
          printf("   hexwSSIM-C2   "); // 17
        }
        break;
    }
  }
}

void DistortionMetricSSIM::printSeparator(){
  printf("-----------");
  printf("-----------");
  printf("-----------");
  if (m_enableHexMetric == TRUE) {
    printf("-----------------");
    printf("-----------------");
    printf("-----------------");
  }
}
} //namespace hdrtoolslib 
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
