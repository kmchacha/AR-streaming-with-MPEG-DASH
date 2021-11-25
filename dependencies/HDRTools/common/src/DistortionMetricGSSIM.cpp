/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Apple Inc.
 * <ORGANIZATION> = Apple Inc.
 * <YEAR> = 2020
 *
 * Copyright (c) 2020, Apple Inc.
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
 * \file DistortionMetricGSSIM.cpp
 *
 * \brief
  SSIM distortion computation class using a Gaussian weighting function
 *    Code is based on the matlab SSIM implementation, which in turn is based on
 *    Z. Wang, A. C. Bovik, H. R. Sheikh, and E. P. Simoncelli, 
 *    "Image Quality Assessment: From Error Visibility to Structural Similarity,"
 *     IEEE Transactions on Image Processing, Volume 13, Issue 4, 
 *     pp. 600- 612, 2004. 
 * 
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "DistortionMetricGSSIM.H"
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

DistortionMetricGSSIM::DistortionMetricGSSIM(const FrameFormat *format, SSIMParams *params, double maxSampleValue, bool enableHexMetric)
: DistortionMetric()
{
  // Metric parameters
  m_K1 = params->m_K1;
  m_K2 = params->m_K2;
  
  m_C1 = pow(m_K1 * 255.0, 2.0);  
  m_C2 = pow(m_K2 * 255.0, 2.0);  
  m_useLogSSIM      = params->m_useLog;
  m_enableHexMetric = enableHexMetric;
  m_width = 16;
  m_height = 16;
  m_size = m_width * m_height;
  
  m_colorSpace    = format->m_colorSpace;
  m_filter = new GaussianFilter(11, 11, 1.5);
  
  for (int c = 0; c < T_COMP; c++) {
    m_metric[c] = 0.0;
    m_metricStats[c].reset();
    m_maxValue[c] = (double) maxSampleValue;
    m_hexMetricSSIM[c] = 0;
  }
  
  m_muX2.resize(m_size);
  m_muY2.resize(m_size);
  m_muXY.resize(m_size);
  m_squareX.resize(m_size);
  m_squareY.resize(m_size);
  m_mulXY.resize(m_size);
  m_sigmaX2.resize(m_size);
  m_sigmaY2.resize(m_size);
  m_sigmaXY.resize(m_size);
  //m_num.resize(m_size);
  //m_den.resize(m_size);
  //m_ssimMap.resize(m_size);
}

DistortionMetricGSSIM::~DistortionMetricGSSIM()
{
}

//-----------------------------------------------------------------------------
// Private methods (non-MMX)
//-----------------------------------------------------------------------------
float DistortionMetricGSSIM::compute (float *inp0Data, float *inp1Data, int height, int width, int comp, float maxPixelValue)
{
  int i;
  double ssimValue = 0.0;
  double numerator, denominator;

  if (m_size  < height * width) {
    m_size = height * width;
    m_muX2.resize(m_size);
    m_muY2.resize(m_size);
    m_muXY.resize(m_size);
    m_squareX.resize(m_size);
    m_squareY.resize(m_size);
    m_mulXY.resize(m_size);
    m_sigmaX2.resize(m_size);
    m_sigmaY2.resize(m_size);
    m_sigmaXY.resize(m_size);
  }

  m_filter->compute(inp0Data, &m_muX2[0], width, height);
  m_filter->compute(inp1Data, &m_muY2[0], width, height);

  for (i = 0; i < height * width; i++) {
    m_muXY[i] = ((double) m_muX2[i] * (double) m_muY2[i]);
    m_muX2[i] = ((double) m_muX2[i] * (double) m_muX2[i]);
    m_muY2[i] = ((double) m_muY2[i] * (double) m_muY2[i]);
    m_squareX[i] = ((double) inp0Data[i] * (double) inp0Data[i]);
    m_squareY[i] = ((double) inp1Data[i] * (double) inp1Data[i]);
    m_mulXY[i] = ((double) inp0Data[i] * (double) inp1Data[i]);
  }

  m_filter->compute(&m_squareX[0], &m_sigmaX2[0], width, height);
  m_filter->compute(&m_squareY[0], &m_sigmaY2[0], width, height);
  m_filter->compute(&m_mulXY[0], &m_sigmaXY[0], width, height);

  for (i = 0; i < height * width; i++) {
    m_sigmaX2[i] = ((double) m_sigmaX2[i] - (double) m_muX2[i]);
    m_sigmaY2[i] = ((double) m_sigmaY2[i] - (double) m_muY2[i]);
    m_sigmaXY[i] = ((double) m_sigmaXY[i] - (double) m_muXY[i]);
  }  

  for (i = 0; i < height * width; i++) {
    numerator = (2.0 * (double) m_muXY[i] + m_C1) * (2.0 * (double) m_sigmaXY[i] + m_C2);
    denominator = ((double) m_muX2[i] + (double) m_muY2[i] + m_C1)*((double) m_sigmaX2[i] + (double) m_sigmaY2[i] + m_C2);
    ssimValue += numerator / denominator;
  }
  ssimValue /= (double) height * width;

  return (float) ssimValue;
}


float DistortionMetricGSSIM::compute (uint8 *inp0Data, uint8 *inp1Data, int height, int width, int comp, int maxPixelValue)
{
  int i;
  double ssimValue = 0.0;
  double numerator, denominator;
  m_C1 = pow(m_K1 * (double) maxPixelValue, 2.0);  
  m_C2 = pow(m_K2 * (double) maxPixelValue, 2.0);  
  
  if (m_size  < height * width) {
    m_size = height * width;
    m_muX2.resize(m_size);
    m_muY2.resize(m_size);
    m_muXY.resize(m_size);
    m_squareX.resize(m_size);
    m_squareY.resize(m_size);
    m_mulXY.resize(m_size);
    m_sigmaX2.resize(m_size);
    m_sigmaY2.resize(m_size);
    m_sigmaXY.resize(m_size);
  }

  m_filter->compute(inp0Data, &m_muX2[0], width, height);
  m_filter->compute(inp1Data, &m_muY2[0], width, height);

  for (i = 0; i < height * width; i++) {
    m_muXY[i] = (m_muX2[i] * m_muY2[i]);
    m_muX2[i] = (m_muX2[i] * m_muX2[i]);
    m_muY2[i] = (m_muY2[i] * m_muY2[i]);

    
    m_squareX[i] = ((double) inp0Data[i] * (double) inp0Data[i]);
    m_squareY[i] = ((double) inp1Data[i] * (double) inp1Data[i]);
    m_mulXY[i] = ((double) inp0Data[i] * (double) inp1Data[i]);
  }

  m_filter->compute(&m_squareX[0], &m_sigmaX2[0], width, height);
  m_filter->compute(&m_squareY[0], &m_sigmaY2[0], width, height);
  m_filter->compute(&m_mulXY[0], &m_sigmaXY[0], width, height);

  for (i = 0; i < height * width; i++) {
    m_sigmaX2[i] = (m_sigmaX2[i] - m_muX2[i]);
    m_sigmaY2[i] = (m_sigmaY2[i] - m_muY2[i]);
    m_sigmaXY[i] = (m_sigmaXY[i] - m_muXY[i]);
  }  

  for (i = 0; i < height * width; i++) {
    numerator = (2.0 * m_muXY[i] + m_C1) * (2.0 * m_sigmaXY[i] + m_C2);
    denominator = (m_muX2[i] + m_muY2[i] + m_C1) * (m_sigmaX2[i] + m_sigmaY2[i] + m_C2);
    ssimValue += numerator / denominator;
  }
  ssimValue /= (double) (height * width);

  return (float) ssimValue;
}


float DistortionMetricGSSIM::compute (uint16 *inp0Data, uint16 *inp1Data, int height, int width, int comp, int maxPixelValue)
{
  int i;
  double ssimValue = 0.0;
  double numerator, denominator;
  m_C1 = pow(m_K1 * (double) maxPixelValue, 2.0);  
  m_C2 = pow(m_K2 * (double) maxPixelValue, 2.0);  

  if (m_size  < height * width) {
    m_size = height * width;
    m_muX2.resize(m_size);
    m_muY2.resize(m_size);
    m_muXY.resize(m_size);
    m_squareX.resize(m_size);
    m_squareY.resize(m_size);
    m_mulXY.resize(m_size);
    m_sigmaX2.resize(m_size);
    m_sigmaY2.resize(m_size);
    m_sigmaXY.resize(m_size);
    //m_num.resize(m_size);
    //m_den.resize(m_size);
    //m_ssimMap.resize(m_size);
  }
  m_filter->compute(inp0Data, &m_muX2[0], width, height);
  m_filter->compute(inp1Data, &m_muY2[0], width, height);

  for (i = 0; i < height * width; i++) {
    m_muXY[i] = ((double) m_muX2[i] * (double) m_muY2[i]);
    m_muX2[i] = ((double) m_muX2[i] * (double) m_muX2[i]);
    m_muY2[i] = ((double) m_muY2[i] * (double) m_muY2[i]);
    m_squareX[i] = ((double) inp0Data[i] * (double) inp0Data[i]);
    m_squareY[i] = ((double) inp1Data[i] * (double) inp1Data[i]);
    m_mulXY[i] = ((double) inp0Data[i] * (double) inp1Data[i]);
  }

  m_filter->compute(&m_squareX[0], &m_sigmaX2[0], width, height);
  m_filter->compute(&m_squareY[0], &m_sigmaY2[0], width, height);
  m_filter->compute(&m_mulXY[0], &m_sigmaXY[0], width, height);

  for (i = 0; i < height * width; i++) {
    m_sigmaX2[i] = ((double) m_sigmaX2[i] - (double) m_muX2[i]);
    m_sigmaY2[i] = ((double) m_sigmaY2[i] - (double) m_muY2[i]);
    m_sigmaXY[i] = ((double) m_sigmaXY[i] - (double) m_muXY[i]);
  }  

  for (i = 0; i < height * width; i++) {
    //m_num[i] = (float) (2.0 * (double) m_muXY[i] + m_C1) * (2.0 * (double) m_sigmaXY[i] + m_C2);
    //m_den[i] = (float) ((double) m_muX2[i] + (double) m_muY2[i] + m_C1)*((double) m_sigmaX2[i] + (double) m_sigmaY2[i] + m_C2);
    //m_ssimMap[i] = (float) ((double) m_num[i] / (double) m_den[i]);
        //m_ssimMap[i] = (float) ((double) m_num[i] / (double) m_den[i]);
    //ssimValue += (double) m_ssimMap[i];
    numerator = (2.0 * (double) m_muXY[i] + m_C1) * (2.0 * (double) m_sigmaXY[i] + m_C2);
    denominator = ((double) m_muX2[i] + (double) m_muY2[i] + m_C1)*((double) m_sigmaX2[i] + (double) m_sigmaY2[i] + m_C2);
    ssimValue += numerator / denominator;
  }
  ssimValue /= (double) height * width;

  return (float) ssimValue;
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void DistortionMetricGSSIM::computeMetric (Frame* inp0, Frame* inp1)
{
  // it is assumed here that the frames are of the same type
  if (inp0->equalType(inp1)) {
    if (inp0->m_isFloat == TRUE) {    // floating point data
      for (int c = Y_COMP; c < inp0->m_noComponents; c++) {
       m_metric[c] = (double) compute(inp0->m_floatComp[c], inp1->m_floatComp[c], inp0->m_height[c], inp0->m_width[c],  c, (float) m_maxValue[c]);
        m_metricStats[c].updateStats(m_metric[c]);
      }
    }
    else if (inp0->m_bitDepth == 8) {   // 8 bit data
      for (int c = Y_COMP; c < inp0->m_noComponents; c++) {
        m_metric[c] = (double) compute(inp0->m_comp[c], inp1->m_comp[c], inp0->m_height[c], inp0->m_width[c], c, inp0->m_maxPelValue[c]);
        m_metricStats[c].updateStats(m_metric[c]);
      }
    }
    else { // 16 bit data
      for (int c = Y_COMP; c < inp0->m_noComponents; c++) {
        m_metric[c] = (double) compute(inp0->m_ui16Comp[c], inp1->m_ui16Comp[c], inp0->m_height[c], inp0->m_width[c], c, inp0->m_maxPelValue[c]);
        m_metricStats[c].updateStats(m_metric[c]);
      }
    }
  }
  else {
    printf("Frames of different type being compared. Computation will not be performed for this frame.\n");
  }
}


void DistortionMetricGSSIM::computeMetric (Frame* inp0, Frame* inp1, int component)
{
  // it is assumed here that the frames are of the same type
  // Currently no TF is applied on the data. However, we may wish to apply some TF, e.g. PQ or combination of PQ and PH.
  
  if (inp0->equalType(inp1)) {
    if (inp0->m_isFloat == TRUE) {    // floating point data
      m_metric[component] = (double) compute(inp0->m_floatComp[component], inp1->m_floatComp[component], inp0->m_height[component], inp0->m_width[component], component, (float) m_maxValue[component]);
      m_metricStats[component].updateStats(m_metric[component]);
    }
    else if (inp0->m_bitDepth == 8) {   // 8 bit data
      m_metric[component] = (double) compute(inp0->m_comp[component], inp1->m_comp[component], inp0->m_height[component], inp0->m_width[component],  component, inp0->m_maxPelValue[component]);
      
      m_metricStats[component].updateStats(m_metric[component]);
    }
    else { // 16 bit data
      m_metric[component] = (double) compute(inp0->m_ui16Comp[component], inp1->m_ui16Comp[component], inp0->m_height[component], inp0->m_width[component], component, inp0->m_maxPelValue[component]);
      m_metricStats[component].updateStats(m_metric[component]);
    }
  }
  else {
    printf("Frames of different type being compared. Computation will not be performed for this frame.\n");
  }
}

void DistortionMetricGSSIM::reportMetric  ()
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

void DistortionMetricGSSIM::reportSummary  ()
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

void DistortionMetricGSSIM::reportMinimum  ()
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

void DistortionMetricGSSIM::reportMaximum  ()
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

void DistortionMetricGSSIM::printHeader()
{
  if (m_isWindow == FALSE ) {
    switch (m_colorSpace) {
      case CM_YCbCr:
        printf("  GSSIM-Y  "); // 11
        printf("  GSSIM-U  "); // 11
        printf("  GSSIM-V  "); // 11
        if (m_enableHexMetric == TRUE) {
          printf("   hexGSSIM-Y    "); // 17
          printf("   hexGSSIM-U    "); // 17
          printf("   hexGSSIM-V    "); // 17
        }
        break;
      case CM_RGB:
        printf("  GSSIM-R  "); // 11
        printf("  GSSIM-G  "); // 11
        printf("  GSSIM-B  "); // 11
        if (m_enableHexMetric == TRUE) {
          printf("   hexGSSIM-R    "); // 17
          printf("   hexGSSIM-G    "); // 17
          printf("   hexGSSIM-B    "); // 17
        }
        break;
      case CM_XYZ:
        printf("  GSSIM-X  "); // 11
        printf("  GSSIM-Y  "); // 11
        printf("  GSSIM-Z  "); // 11
        if (m_enableHexMetric == TRUE) {
          printf("  hexGSSIM-Y   "); // 17
          printf("  hexGSSIM-U   "); // 17
          printf("  hexGSSIM-V   "); // 17
        }
        break;
      default:
        printf("  GSSIM-C0 "); // 11
        printf("  GSSIM-C1 "); // 11
        printf("  GSSIM-C2 "); // 11
        if (m_enableHexMetric == TRUE) {
          printf("   hexGSSIM-C0   "); // 17
          printf("   hexGSSIM-C1   "); // 17
          printf("   hexGSSIM-C2   "); // 17
        }
        break;
    }
  }
  else {
    switch (m_colorSpace) {
      case CM_YCbCr:
        printf(" wGSSIM-Y  "); // 11
        printf(" wGSSIM-U  "); // 11
        printf(" wGSSIM-V  "); // 11
        if (m_enableHexMetric == TRUE) {
          printf("   hexwGSSIM-Y   "); // 17
          printf("   hexwGSSIM-U   "); // 17
          printf("   hexwGSSIM-V   "); // 17
        }
        break;
      case CM_RGB:
        printf(" wGSSIM-R  "); // 11
        printf(" wGSSIM-G  "); // 11
        printf(" wGSSIM-B  "); // 11
        if (m_enableHexMetric == TRUE) {
          printf("   hexwGSSIM-R   "); // 17
          printf("   hexwGSSIM-G   "); // 17
          printf("   hexwGSSIM-B   "); // 17
        }
        break;
      case CM_XYZ:
        printf(" wGSSIM-X  "); // 11
        printf(" wGSSIM-Y  "); // 11
        printf(" wGSSIM-Z  "); // 11
        if (m_enableHexMetric == TRUE) {
          printf("   hexwGSSIM-X   "); // 17
          printf("   hexwGSSIM-Y   "); // 17
          printf("   hexwGSSIM-Z   "); // 17
        }
        break;
      default:
        printf(" wGSSIM-C0 "); // 11
        printf(" wGSSIM-C1 "); // 11
        printf(" wGSSIM-C2 "); // 11
        if (m_enableHexMetric == TRUE) {
          printf("  hexwGSSIM-C0   "); // 17
          printf("  hexwGSSIM-C1   "); // 17
          printf("  hexwGSSIM-C2   "); // 17
        }
        break;
    }
  }
}

void DistortionMetricGSSIM::printSeparator(){
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
