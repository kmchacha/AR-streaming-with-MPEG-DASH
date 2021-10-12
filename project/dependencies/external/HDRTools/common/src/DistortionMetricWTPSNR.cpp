/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Apple Inc., Qualcomm
 * <ORGANIZATION> = Apple Inc., Qualcomm
 * <YEAR> = 2017
 *
 * Copyright (c) 2017, Apple Inc., Qualcomm
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
 * \file DistortionMetricWTPSNR.cpp
 *
 * \brief
 *    Weighted PSNR distortion computation Class
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *     - Adarsh Krishnan Ramasubramonian <aramasub@qti.qualcomm.com>
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "DistortionMetricWTPSNR.H"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

namespace hdrtoolslib {

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

DistortionMetricWTPSNR::DistortionMetricWTPSNR(const FrameFormat *format, bool enableShowMSE, bool enableJVETSNR, double maxSampleValue, float *weights, bool enableHexMetric)
 : DistortionMetric()
{
  m_colorSpace      = format->m_colorSpace;
  m_enableShowMSE   = enableShowMSE;
  m_enableJVETSNR   = enableJVETSNR;
  m_enableHexMetric = enableHexMetric;

  for (int c = 0; c < T_COMP; c++) {
    m_mse[c] = 0.0;
    m_sse[c] = 0.0;
    m_mseStats[c].reset();
    m_sseStats[c].reset();
    m_hexMetricWTPSNR[c] = 0;
    
    m_maxValue[c] = (double) maxSampleValue;
  }
  for (int c = 0; c < WT_DISTORTION_COUNT; c++) {
   m_weights[c] = (double) weights[c];
  }
}

DistortionMetricWTPSNR::~DistortionMetricWTPSNR()
{
}

//-----------------------------------------------------------------------------
// Private methods (non-MMX)
//-----------------------------------------------------------------------------
double DistortionMetricWTPSNR::compute(const float *iComp0, const float *iComp1, const float *YComp0, int height, int width, int lHeight, int lWidth, double maxValue)
{
  double diff   = 0.0;
  double sum    = 0.0;
  const float* p3 = YComp0;
  int jOffset = lWidth / width;
  int lIncrement = lWidth * (lHeight / height);
  double range = (double) WT_DISTORTION_COUNT - 1;

  if (m_clipInputValues) { // Add clipping of distortion values as per Jacob's request
    for (int i = 0; i < height; i++) {
      for(int j = 0, jLuma = 0; j < width; j++, jLuma += jOffset) {
        diff   = (dMin((double) *iComp0++, maxValue) - dMin((double) *iComp1++, maxValue));
        sum   += m_weights[iMin(range, (int) dRound(range * p3[jLuma]))] * diff * diff;
      }
      p3 += lIncrement;
    }
  }
  else {
      for ( int i = 0; i < height; i++) {
        for(int j = 0, jLuma = 0; j < width; j++, jLuma += jOffset) {
          diff   = ((double) *iComp0++ - (double) *iComp1++);
          sum   += m_weights[iMin(range, (int) dRound(range * p3[jLuma]))] * diff * diff;
        }
        p3 += lIncrement;
     }
  }
  return sum;
}

double DistortionMetricWTPSNR::compute(const uint16 *iComp0, const uint16 *iComp1, const uint16 *YComp0, int height, int width, int lHeight, int lWidth, int shift)
{
  double diff   = 0.0;
  double sum    = 0.0;
  int jOffset = lWidth / width;
  int lIncrement = lWidth * (lHeight / height);
  double weights = 1.0;
  if (shift > 0) {
    for (int i = 0; i < height; i++) {
      for(int j = 0, jLuma = 0; j < width; j++, jLuma += jOffset) {
        diff   = (double) ((int32) *iComp0++ - (int32) *iComp1++);
        weights = m_weights[YComp0[jLuma] >> shift];
        sum   += weights * diff * diff;
      }
      YComp0 += lIncrement;
    }
  }
  else {
    shift = -shift;
    for (int i = 0; i < height; i++) {
      for(int j = 0, jLuma = 0; j < width; j++, jLuma += jOffset) {
        diff   = (double) ((int32) *iComp0++ - (int32) *iComp1++);
        weights = m_weights[YComp0[jLuma] << shift];
        sum   += weights * diff * diff;
      }
      YComp0 += lIncrement;
    }
  }
  return sum;
}

double DistortionMetricWTPSNR::compute(const uint8 *iComp0, const uint8 *iComp1, const uint8 *YComp0, int height, int width, int lHeight, int lWidth)
{
  double diff   = 0.0;
  double sum    = 0.0;
  int jOffset = lWidth / width;
  int lIncrement = lWidth * (lHeight / height);
  double weights = 1.0;
  for (int i = 0; i < height; i++) {
    for(int j = 0, jLuma = 0; j < width; j++, jLuma += jOffset) {
      diff   = (double) ((int32) *iComp0++ - (int32) *iComp1++);
      // scale luma to 10 bits by shifting
      weights = m_weights[YComp0[jLuma] << 2];
      sum   += weights * diff * diff;
    }
    YComp0 += lIncrement;
  }
  return sum;
}




//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void DistortionMetricWTPSNR::computeMetric (Frame* inp0, Frame* inp1)
{
  // it is assumed here that the frames are of the same type
  if (inp0->equalType(inp1)) {
    if (inp0->m_isFloat == TRUE) {    // floating point data
        for (int c = Y_COMP; c < inp0->m_noComponents; c++) {
          m_sse[c] = compute(inp0->m_floatComp[c], inp1->m_floatComp[c], inp0->m_floatComp[Y_COMP], inp0->m_height[c], inp0->m_width[c], inp0->m_height[Y_COMP], inp0->m_width[Y_COMP], m_maxValue[c]);
          m_sseStats[c].updateStats(m_sse[c]);
          m_mse[c] = m_sse[c] / (double) inp0->m_compSize[c];
          m_mseStats[c].updateStats(m_mse[c]);
          if (m_enableJVETSNR)
            m_metric[c] = m_sse[c] ? psnr(m_maxValue[c], inp0->m_compSize[c], m_sse[c]) : 999.99;
          else
            m_metric[c] = psnr(m_maxValue[c], inp0->m_compSize[c], m_sse[c]);
          m_metricStats[c].updateStats(m_metric[c]);
      }
    }
    else if (inp0->m_bitDepth == 8) {   // 8 bit data
        for (int c = Y_COMP; c < inp0->m_noComponents; c++) {
          m_sse[c] = (double) compute(inp0->m_comp[c], inp1->m_comp[c], inp0->m_comp[Y_COMP], inp0->m_height[c], inp0->m_width[c], inp0->m_height[Y_COMP], inp0->m_width[Y_COMP]);
          
          m_sseStats[c].updateStats(m_sse[c]);
          m_mse[c] = m_sse[c] / (double) inp0->m_compSize[c];
          m_mseStats[c].updateStats(m_mse[c]);
          if (m_enableJVETSNR)
            m_metric[c] = m_sse[c] ? psnr(inp0->m_maxPelValue[c], inp0->m_compSize[c], m_sse[c]) : 999.99;
          else
            m_metric[c] = psnr(inp0->m_maxPelValue[c], inp0->m_compSize[c], m_sse[c]);
          m_metricStats[c].updateStats(m_metric[c]);
        }
    }
    else { // 16 bit data
      int maxValue = 0;
      for (int c = Y_COMP; c < inp0->m_noComponents; c++) {
        if (m_enableJVETSNR == TRUE) {
          maxValue = 255 << (inp0->m_bitDepth - 8);
        }
        else {
          maxValue = inp0->m_maxPelValue[c];
        }
        m_sse[c] = (double) compute(inp0->m_ui16Comp[c], inp1->m_ui16Comp[c], inp0->m_ui16Comp[Y_COMP], inp0->m_height[c], inp0->m_width[c], inp0->m_height[Y_COMP], inp0->m_width[Y_COMP], inp0->m_bitDepth - 10);
        m_sseStats[c].updateStats(m_sse[c]);
        m_mse[c] = m_sse[c] / (double) inp0->m_compSize[c];
        m_mseStats[c].updateStats(m_mse[c]);
        if (m_enableJVETSNR)
          m_metric[c] = m_sse[c] ? psnr(inp0->m_maxPelValue[c], inp0->m_compSize[c], m_sse[c]) : 999.99;
        else
          m_metric[c] = psnr(inp0->m_maxPelValue[c], inp0->m_compSize[c], m_sse[c]);
        m_metricStats[c].updateStats(m_metric[c]);

      }
    }
  }
  else {
    printf("Frames of different type being compared. Computation will not be performed for this frame.\n");
  }
}


void DistortionMetricWTPSNR::computeMetric (Frame* inp0, Frame* inp1, int component)
{
  // it is assumed here that the frames are of the same type
  if (inp0->equalType(inp1)) {
    if (inp0->m_isFloat == TRUE) {    // floating point data
      m_sse[component] = compute(inp0->m_floatComp[component], inp1->m_floatComp[component], inp0->m_floatComp[Y_COMP], inp0->m_height[component], inp0->m_width[component], inp0->m_height[Y_COMP], inp0->m_width[Y_COMP], m_maxValue[component]);
      m_sseStats[component].updateStats(m_sse[component]);
      m_mse[component] = m_sse[component] / (double) inp0->m_compSize[component];
      m_mseStats[component].updateStats(m_mse[component]);
      if (m_enableJVETSNR)
        m_metric[component] = m_sse[component] ? psnr(m_maxValue[component], inp0->m_compSize[component], m_sse[component]) : 999.99;
      else
        m_metric[component] = psnr(m_maxValue[component], inp0->m_compSize[component], m_sse[component]);
      m_metricStats[component].updateStats(m_metric[component]);
    }
    else if (inp0->m_bitDepth == 8) {   // 8 bit data
      m_sse[component] = compute(inp0->m_comp[component], inp1->m_comp[component], inp0->m_comp[Y_COMP], inp0->m_height[component], inp0->m_width[component], inp0->m_height[Y_COMP], inp0->m_width[Y_COMP]);
      m_sseStats[component].updateStats(m_sse[component]);
      m_mse[component] = m_sse[component] / (double) inp0->m_compSize[component];
      m_mseStats[component].updateStats(m_mse[component]);
      if (m_enableJVETSNR)
        m_metric[component] = m_sse[component] ? psnr(inp0->m_maxPelValue[component], inp0->m_compSize[component], m_sse[component]) : 999.99;
      else
        m_metric[component] = psnr(inp0->m_maxPelValue[component], inp0->m_compSize[component], m_sse[component]);
      m_metricStats[component].updateStats(m_metric[component]);
    }
    else { // 16 bit data
      int maxValue = 0;
      if (m_enableJVETSNR == TRUE) {
        maxValue = 255 << (inp0->m_bitDepth - 8);
      }
      else {
        maxValue = inp0->m_maxPelValue[component];
      }

      m_sse[component] = compute(inp0->m_ui16Comp[component], inp1->m_ui16Comp[component], inp0->m_ui16Comp[Y_COMP], inp0->m_height[component], inp0->m_width[component], inp0->m_height[Y_COMP], inp0->m_width[Y_COMP], inp0->m_bitDepth - 10);
      m_sseStats[component].updateStats(m_sse[component]);
      m_mse[component] = m_sse[component] / (double) inp0->m_compSize[component];
      m_mseStats[component].updateStats(m_mse[component]);
      if (m_enableJVETSNR)
        m_metric[component] = m_sse[component] ? psnr(maxValue, inp0->m_compSize[component], m_sse[component]) : 999.99;
      else      
        m_metric[component] = psnr(maxValue, inp0->m_compSize[component], m_sse[component]);
      m_metricStats[component].updateStats(m_metric[component]);
    }
  }
  else {
    printf("Frames of different type being compared. Computation will not be performed for this frame.\n");
  }
}

void DistortionMetricWTPSNR::reportMetric  ()
{
  printf("%9.3f %9.3f %9.3f ", m_metric[Y_COMP], m_metric[U_COMP], m_metric[V_COMP]);
  if (m_enableShowMSE == TRUE)
    printf("%10.3f %10.3f %10.3f ", m_mse[Y_COMP], m_mse[U_COMP], m_mse[V_COMP]);
  if (m_enableHexMetric == TRUE) {
    for (int c = 0; c < T_COMP; c++) {
      std::copy(reinterpret_cast<uint8_t *>(&m_metric[c]),
                reinterpret_cast<uint8_t *>(&m_metric[c]) + sizeof(m_metric[c]),
                reinterpret_cast<uint8_t *>(&m_hexMetricWTPSNR[c]));
    }
    printf(" %16" PRIx64 " %16" PRIx64 " %16" PRIx64, m_hexMetricWTPSNR[Y_COMP], m_hexMetricWTPSNR[U_COMP], m_hexMetricWTPSNR[V_COMP]);
  }
}

void DistortionMetricWTPSNR::reportSummary  ()
{
  printf("%9.3f %9.3f %9.3f ", m_metricStats[Y_COMP].getAverage(), m_metricStats[U_COMP].getAverage(), m_metricStats[V_COMP].getAverage());
  if (m_enableShowMSE == TRUE)
    printf("%10.3f %10.3f %10.3f ", m_mseStats[Y_COMP].getAverage(), m_mseStats[U_COMP].getAverage(), m_mseStats[V_COMP].getAverage());
  if (m_enableHexMetric == TRUE) {
    for (int c = 0; c < T_COMP; c++) {
      double val = m_metricStats[c].getAverage();
      std::copy(reinterpret_cast<uint8_t *>(&val),
                reinterpret_cast<uint8_t *>(&val) + sizeof(val),
                reinterpret_cast<uint8_t *>(&m_hexMetricWTPSNR[c]));
    }
    printf(" %16" PRIx64 " %16" PRIx64 " %16" PRIx64, m_hexMetricWTPSNR[Y_COMP], m_hexMetricWTPSNR[U_COMP], m_hexMetricWTPSNR[V_COMP]);
  }
}

void DistortionMetricWTPSNR::reportMinimum  ()
{
  printf("%9.3f %9.3f %9.3f ", m_metricStats[Y_COMP].minimum, m_metricStats[U_COMP].minimum, m_metricStats[V_COMP].minimum);
  if (m_enableShowMSE == TRUE)
    printf("%10.3f %10.3f %10.3f ", m_mseStats[Y_COMP].minimum, m_mseStats[U_COMP].minimum, m_mseStats[V_COMP].minimum);
  if (m_enableHexMetric == TRUE) {
    for (int c = 0; c < T_COMP; c++) {
      std::copy(reinterpret_cast<uint8_t *>(&m_metricStats[c].minimum),
                reinterpret_cast<uint8_t *>(&m_metricStats[c].minimum) + sizeof(m_metricStats[c].minimum),
                reinterpret_cast<uint8_t *>(&m_hexMetricWTPSNR[c]));
    }
    printf(" %16" PRIx64 " %16" PRIx64 " %16" PRIx64, m_hexMetricWTPSNR[Y_COMP], m_hexMetricWTPSNR[U_COMP], m_hexMetricWTPSNR[V_COMP]);
  }
}

void DistortionMetricWTPSNR::reportMaximum  ()
{
  printf("%9.3f %9.3f %9.3f ", m_metricStats[Y_COMP].maximum, m_metricStats[U_COMP].maximum, m_metricStats[V_COMP].maximum);
  if (m_enableShowMSE == TRUE)
    printf("%10.3f %10.3f %10.3f ", m_mseStats[Y_COMP].maximum, m_mseStats[U_COMP].maximum, m_mseStats[V_COMP].maximum);
  if (m_enableHexMetric == TRUE) {
    for (int c = 0; c < T_COMP; c++) {
      std::copy(reinterpret_cast<uint8_t *>(&m_metricStats[c].maximum),
                reinterpret_cast<uint8_t *>(&m_metricStats[c].maximum) + sizeof(m_metricStats[c].maximum),
                reinterpret_cast<uint8_t *>(&m_hexMetricWTPSNR[c]));
    }
    printf(" %16" PRIx64 " %16" PRIx64 " %16" PRIx64, m_hexMetricWTPSNR[Y_COMP], m_hexMetricWTPSNR[U_COMP], m_hexMetricWTPSNR[V_COMP]);
  }
}

void DistortionMetricWTPSNR::printHeader()
{
  if (m_isWindow == FALSE ) {
  switch (m_colorSpace) {
    case CM_YCbCr:
      printf(" wtPSNR-Y "); // 10
      printf(" wtPSNR-U "); // 10
      printf(" wtPSNR-V "); // 10
      if (m_enableShowMSE == TRUE) {
        printf(" wtdMSE-Y  "); // 11
        printf(" wtdMSE-U  "); // 11
        printf(" wtdMSE-V  "); // 11
      }
      if (m_enableHexMetric == TRUE) {
        printf("   hexwtPSNR-Y   "); // 17
        printf("   hexwtPSNR-U   "); // 17
        printf("   hexwtPSNR-V   "); // 17
      }
      break;
    case CM_RGB:
      printf(" wtPSNR-R "); // 10
      printf(" wtPSNR-G "); // 10
      printf(" wtPSNR-B "); // 10
      if (m_enableShowMSE == TRUE) {
        printf(" wtdMSE-R  "); // 11
        printf(" wtdMSE-G  "); // 11
        printf(" wtdMSE-B  "); // 11
      }
      if (m_enableHexMetric == TRUE) {
        printf("   hexwtPSNR-R   "); // 17
        printf("   hexwtPSNR-G   "); // 17
        printf("   hexwtPSNR-B   "); // 17
      }
      break;
    case CM_XYZ:
      printf(" wtPSNR-X "); // 10
      printf(" wtPSNR-Y "); // 10
      printf(" wtPSNR-Z "); // 10
      if (m_enableShowMSE == TRUE) {
        printf(" wtdMSE-X  "); // 11
        printf(" wtdMSE-Y  "); // 11
        printf(" wtdMSE-Z  "); // 11
      }
      if (m_enableHexMetric == TRUE) {
        printf("   hexwtPSNR-X   "); // 17
        printf("   hexwtPSNR-Y   "); // 17
        printf("   hexwtPSNR-Z   "); // 17
      }
      break;
    default:
      printf("wtPSNR-C0 "); // 10
      printf("wtPSNR-C1 "); // 10
      printf("wtPSNR-C2 "); // 10
      if (m_enableShowMSE == TRUE) {
        printf(" wtMSE-C0  "); // 11
        printf(" wtMSE-C1  "); // 11
        printf(" wtMSE-C2  "); // 11
      }
      if (m_enableHexMetric == TRUE) {
        printf("   hexwtPSNR-C0  "); // 17
        printf("   hexwtPSNR-C1  "); // 17
        printf("   hexwtPSNR-C2  "); // 17
      }
      break;
  }
}
  else {
    switch (m_colorSpace) {
      case CM_YCbCr:
        printf("wWtPSNR-Y "); // 10
        printf("wWtPSNR-U "); // 10
        printf("wWtPSNR-V "); // 10
        if (m_enableShowMSE == TRUE) {
          printf(" wWtMSE-Y  "); // 11
          printf(" wWtMSE-U  "); // 11
          printf(" wWtMSE-V  "); // 11
        }
        if (m_enableHexMetric == TRUE) {
          printf("   hexwWtPSNR-Y  "); // 17
          printf("   hexwWtPSNR-U  "); // 17
          printf("   hexwWtPSNR-V  "); // 17
        }
        break;
      case CM_RGB:
        printf("wWtPSNR-R "); // 10
        printf("wWtPSNR-G "); // 10
        printf("wWtPSNR-B "); // 10
        if (m_enableShowMSE == TRUE) {
          printf(" wWtMSE-R  "); // 11
          printf(" wWtMSE-G  "); // 11
          printf(" wWtMSE-B  "); // 11
        }
        if (m_enableHexMetric == TRUE) {
          printf("   hexwWtPSNR-R  "); // 17
          printf("   hexwWtPSNR-G  "); // 17
          printf("   hexwWtPSNR-B  "); // 17
        }
        break;
      case CM_XYZ:
        printf("wWtPSNR-X "); // 10
        printf("wWtPSNR-Y "); // 10
        printf("wWtPSNR-Z "); // 10
        if (m_enableShowMSE == TRUE) {
          printf(" wWtMSE-X  "); // 11
          printf(" wWtMSE-Y  "); // 11
          printf(" wWtMSE-Z  "); // 11
        }
        if (m_enableHexMetric == TRUE) {
          printf("   hexwWtPSNR-X  "); // 17
          printf("   hexwWtPSNR-Y  "); // 17
          printf("   hexwWtPSNR-Z  "); // 17
        }
        break;
      default:
        printf("wWtPSNRC0 "); // 10
        printf("wWtPSNRC1 "); // 10
        printf("wWtPSNRC2 "); // 10
        if (m_enableShowMSE == TRUE) {
          printf(" wWtMSE-C0 "); // 11
          printf(" wWtMSE-C1 "); // 11
          printf(" wWtMSE-C2 "); // 11
        }
        if (m_enableHexMetric == TRUE) {
          printf("   hexwWtPSNR-C0 "); // 17
          printf("   hexwWtPSNR-C1 "); // 17
          printf("   hexwWtPSNR-C2 "); // 17
        }
        break;
    }
  }
}

void DistortionMetricWTPSNR::printSeparator(){
  printf("----------");
  printf("----------");
  printf("----------");
  if (m_enableShowMSE == TRUE) {
    printf("-----------");
    printf("-----------");
    printf("-----------");
  }
  if (m_enableHexMetric == TRUE) {
    printf("-----------------");
    printf("-----------------");
    printf("-----------------");
  }
}
} // namespace hdrtoolslib
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
