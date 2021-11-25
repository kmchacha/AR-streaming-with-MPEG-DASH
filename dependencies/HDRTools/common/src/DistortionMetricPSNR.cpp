/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Apple Inc.
 * <ORGANIZATION> = Apple Inc.
 * <YEAR> = 2014
 *
 * Copyright (c) 2014, Apple Inc.
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
 * \file DistortionMetricPSNR.cpp
 *
 * \brief
 *    PSNR distortion computation Class
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "DistortionMetricPSNR.H"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

namespace hdrtoolslib {

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

DistortionMetricPSNR::DistortionMetricPSNR(const FrameFormat *format, bool enableShowMSE, bool enableJVETSNR, double maxSampleValue, bool enablexPSNR, double *xPSNRweights, bool enableHexMetric)
 : DistortionMetric()
{
  m_colorSpace      = format->m_colorSpace;
  m_enableShowMSE   = enableShowMSE;
  m_enablexPSNR     = enablexPSNR;
  m_enableJVETSNR   = enableJVETSNR;
  m_enableHexMetric = enableHexMetric;

  for (int c = 0; c < T_COMP; c++) {
    m_mse[c] = 0.0;
    m_sse[c] = 0.0;
    m_mseStats[c].reset();
    m_sseStats[c].reset();
    m_maxValue[c] = (double) maxSampleValue;
    m_xPSNRweights[c] = xPSNRweights[c];
    m_hexMetricPSNR[c] = 0;
  }
  m_oError = 0.0;
  m_xPSNR = 0.0;
  m_xPSNRStats.reset();
}

DistortionMetricPSNR::~DistortionMetricPSNR()
{
}

//-----------------------------------------------------------------------------
// Private methods (non-MMX)
//-----------------------------------------------------------------------------
double DistortionMetricPSNR::computeLumaError(const float *iComp0, const float *iComp1, int width, int height, int shiftwidth, int shiftheight, double maxValue, double weight)
{
  double diff = 0.0;
  double sum = 0.0;
  m_oError = 0.0;
  
  if (m_clipInputValues) {
    for (int j = 0; j < height; j++) {
      for (int i = 0; i < width; i++) {
        diff = (dMin(iComp0[j * width + i], maxValue) - dMin(iComp1[j * width + i], maxValue));
        diff *= diff;
        m_oError += weight * diff + m_chromaError[(j >> shiftheight) * (width >> shiftwidth) + (i >> shiftwidth)];
        sum += diff;
      }
    }
  }
  else {
    for (int j = 0; j < height; j++) {
      for (int i = 0; i < width; i++) {
        diff = (iComp0[j * width + i] - iComp1[j * width + i]);
        diff *= diff;
        m_oError += weight * diff + m_chromaError[(j >> shiftheight) * (width >> shiftwidth) + (i >> shiftwidth)];
        sum += diff;
      }
    }
  }
  return sum;
}


uint64 DistortionMetricPSNR::computeLumaError(const uint16 *iComp0, const uint16 *iComp1, int width, int height, int shiftwidth, int shiftheight, double weight)
{
  int32 diff = 0;
  uint64 sum = 0;
  m_oError = 0.0;
  
    for (int j = 0; j < height; j++) {
      for (int i = 0; i < width; i++) {
        diff = ((int32) iComp0[j * width + i] - (int32) iComp1[j * width + i]);
        diff *= diff;
        m_oError += weight * (double) diff + m_chromaError[(j >> shiftheight) * (width >> shiftwidth) + (i >> shiftwidth)];
        sum += diff;
      }
    }
  return sum;
}

uint64 DistortionMetricPSNR::computeLumaError(const uint8 *iComp0, const uint8 *iComp1, int width, int height, int shiftwidth, int shiftheight, double weight)
{
  int32 diff = 0;
  uint64 sum = 0;
  m_oError = 0.0;
  
    for (int j = 0; j < height; j++) {
      for (int i = 0; i < width; i++) {
        diff = ((int32) iComp0[j * width + i] - (int32) iComp1[j * width + i]);
        diff *= diff;
        m_oError += weight * (double) diff + m_chromaError[(j >> shiftheight) * (width >> shiftwidth) + (i >> shiftwidth)];
        sum += diff;
      }
    }
  return sum;
}


double DistortionMetricPSNR::computeChromaError(const float *iComp0, const float *iComp1, int size, double maxValue, bool addError, double weight)
{
  double diff = 0.0;
  double sum = 0.0;
  if (addError == FALSE) {
    if (m_clipInputValues) {
      for (int i = 0; i < size; i++) {
        diff = (dMin(iComp0[i], maxValue) - dMin(iComp1[i], maxValue));
        diff *= diff;
        m_chromaError[i] = weight * diff;
        sum += diff;
      }
    }
    else {
      for (int i = 0; i < size; i++) {
        diff = (iComp0[i] - iComp1[i]);
        diff *= diff;
        m_chromaError[i] = weight * diff;
        sum += diff;
      }
    }
  }
  else {
    if (m_clipInputValues) {
      for (int i = 0; i < size; i++) {
        diff = (dMin(iComp0[i], maxValue) - dMin(iComp1[i], maxValue));
        diff *= diff;
        m_chromaError[i] += weight * diff;
        sum += diff;
      }
    }
    else {
      for (int i = 0; i < size; i++) {
        diff = (iComp0[i] - iComp1[i]);
        diff *= diff;
        m_chromaError[i] += weight * diff;
        sum += diff;
      }
    }
  }
  
  return sum;
}


uint64 DistortionMetricPSNR::computeChromaError(const uint16 *iComp0, const uint16 *iComp1, int size, bool addError, double weight)
{
  int32 diff = 0;
  uint64 sum = 0;
  if (addError == FALSE) {
    for (int i = 0; i < size; i++) {
      diff = ((int32) iComp0[i] - (int32) iComp1[i]);
      diff *= diff;
      m_chromaError[i] = weight * (double) diff;
      sum += diff;
    }
  }
  else {
    for (int i = 0; i < size; i++) {
      diff = ((int32) iComp0[i] - (int32) iComp1[i]);
      diff *= diff;
      m_chromaError[i] += weight * (double) diff;
      sum += diff;
    }
  }
  
  return sum;
}


uint64 DistortionMetricPSNR::computeChromaError(const uint8 *iComp0, const uint8 *iComp1, int size, bool addError, double weight)
{
  int32 diff = 0;
  uint64 sum = 0;
  if (addError == FALSE) {
    for (int i = 0; i < size; i++) {
      diff = ((int32) iComp0[i] - (int32) iComp1[i]);
      diff *= diff;
      m_chromaError[i] = weight * (double) diff;
      sum += diff;
    }
  }
  else {
    for (int i = 0; i < size; i++) {
      diff = ((int32) iComp0[i] - (int32) iComp1[i]);
      diff *= diff;
      m_chromaError[i] += weight * (double) diff;
      sum += diff;
    }
  }
  
  return sum;
}


double DistortionMetricPSNR::compute(const float *iComp0, const float *iComp1, int size, double maxValue)
{
  double diff = 0.0;
  double sum = 0.0;
  if (m_clipInputValues) { // Add clipping of distortion values as per Jacob's request
    for (int i = 0; i < size; i++) {
      diff = (dMin(iComp0[i], maxValue) - dMin(iComp1[i], maxValue));
      sum += diff * diff;
    }
  }
  else {
    for (int i = 0; i < size; i++) {
      diff = (iComp0[i] - iComp1[i]);
      sum += diff * diff;
    }
  }
  return sum;
}

uint64 DistortionMetricPSNR::compute(const uint16 *iComp0, const uint16 *iComp1, int size)
{
  int32  diff = 0;
  uint64 sum = 0;
  for (int i = 0; i < size; i++) {
    diff = ((int32) iComp0[i] - (int32) iComp1[i]);
    sum += diff * diff;
  }
  return sum;
}

uint64 DistortionMetricPSNR::compute(const uint8 *iComp0, const uint8 *iComp1, int size)
{
  int32  diff = 0;
  uint64  sum = 0;
  for (int i = 0; i < size; i++) {
    diff = ((int32) iComp0[i] - (int32) iComp1[i]);
    sum += diff * diff;
  }
  return sum;
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void DistortionMetricPSNR::computeMetric (Frame* inp0, Frame* inp1)
{
  int shiftheight = 0;
  int shiftwidth  = 0;

  // it is assumed here that the frames are of the same type
  if (inp0->equalType(inp1)) {
    if (m_enablexPSNR == TRUE) {
      m_width  = inp0->m_width[U_COMP];
      m_height = inp0->m_height[U_COMP];
      
      if (2 * m_width == inp0->m_width[Y_COMP])
        shiftwidth = 1;
      if (2 * m_height == inp0->m_height[Y_COMP])
        shiftheight = 1;
      
      m_chromaError.resize  ( inp0->m_width[U_COMP] *  inp0->m_height[U_COMP]);
    }

    if (inp0->m_isFloat == TRUE) {    // floating point data
      if (m_enablexPSNR == TRUE && inp0->m_noComponents > 1) {
        m_sse[U_COMP] = computeChromaError(inp0->m_floatComp[U_COMP], inp1->m_floatComp[U_COMP], inp0->m_compSize[U_COMP], m_maxValue[U_COMP], FALSE, m_xPSNRweights[U_COMP]);
        m_sse[V_COMP] = computeChromaError(inp0->m_floatComp[V_COMP], inp1->m_floatComp[V_COMP], inp0->m_compSize[V_COMP], m_maxValue[V_COMP], TRUE,  m_xPSNRweights[V_COMP]);
        m_sse[Y_COMP] = computeLumaError(inp0->m_floatComp[Y_COMP], inp1->m_floatComp[Y_COMP], inp0->m_width[Y_COMP], inp0->m_height[Y_COMP], shiftwidth, shiftheight, m_maxValue[Y_COMP], m_xPSNRweights[Y_COMP]);
        
        for (int c = Y_COMP; c < inp0->m_noComponents; c++) {
          m_sseStats[c].updateStats(m_sse[c]);
          m_mse[c] = m_sse[c] / (double) inp0->m_compSize[c];
          m_mseStats[c].updateStats(m_mse[c]);
          if (m_enableJVETSNR)
            m_metric[c] = m_sse[c] ? psnr(m_maxValue[c], inp0->m_compSize[c], m_sse[c]) : 999.99;
          else  
            m_metric[c] = psnr(m_maxValue[c], inp0->m_compSize[c], m_sse[c]);
          m_metricStats[c].updateStats(m_metric[c]);
        }
        m_xPSNR = psnr(m_maxValue[Y_COMP], (int) inp0->m_size, m_oError);
        m_xPSNRStats.updateStats(m_xPSNR);
      }
      else {
        for (int c = Y_COMP; c < inp0->m_noComponents; c++) {
          m_sse[c] = compute(inp0->m_floatComp[c], inp1->m_floatComp[c], inp0->m_compSize[c], m_maxValue[c]);
          m_sseStats[c].updateStats(m_sse[c]);
          m_mse[c] = m_sse[c] / (double) inp0->m_compSize[c];
          m_mseStats[c].updateStats(m_mse[c]);
          if (m_enableJVETSNR)
            m_metric[c] = m_sse[c] ? psnr(m_maxValue[c], inp0->m_compSize[c], m_sse[c]) : 999.99;
          else  
            m_metric[c] = psnr(m_maxValue[c], inp0->m_compSize[c], m_sse[c]);
          //printf("value %10.7f %10.7f\n", m_metric[c], m_maxValue[c]);

          m_metricStats[c].updateStats(m_metric[c]);
        }
      }
    }
    else if (inp0->m_bitDepth == 8) {   // 8 bit data
      if (m_enablexPSNR == TRUE && inp0->m_noComponents > 1) {
        m_sse[U_COMP] = (double) computeChromaError(inp0->m_comp[U_COMP], inp1->m_comp[U_COMP], inp0->m_compSize[U_COMP], FALSE, m_xPSNRweights[U_COMP]);
        m_sse[V_COMP] = (double) computeChromaError(inp0->m_comp[V_COMP], inp1->m_comp[V_COMP], inp0->m_compSize[V_COMP], TRUE,  m_xPSNRweights[V_COMP]);
        m_sse[Y_COMP] = (double) computeLumaError(inp0->m_comp[Y_COMP], inp1->m_comp[Y_COMP], inp0->m_width[Y_COMP], inp0->m_height[Y_COMP], shiftwidth, shiftheight, m_xPSNRweights[Y_COMP]);
        
        for (int c = Y_COMP; c < inp0->m_noComponents; c++) {
          m_sseStats[c].updateStats(m_sse[c]);
          m_mse[c] = m_sse[c] / (double) inp0->m_compSize[c];
          m_mseStats[c].updateStats(m_mse[c]);
          if (m_enableJVETSNR)
            m_metric[c] = m_sse[c] ? psnr(inp0->m_maxPelValue[c], inp0->m_compSize[c], m_sse[c]) : 999.99;
          else  
            m_metric[c] = psnr(inp0->m_maxPelValue[c], inp0->m_compSize[c], m_sse[c]);
          m_metricStats[c].updateStats(m_metric[c]);
        }
        m_xPSNR = psnr(inp0->m_maxPelValue[Y_COMP], (int) inp0->m_size, m_oError);
        m_xPSNRStats.updateStats(m_xPSNR);
      }
      else {
        for (int c = Y_COMP; c < inp0->m_noComponents; c++) {
          m_sse[c] = (double) compute(inp0->m_comp[c], inp1->m_comp[c], inp0->m_compSize[c]);
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
    else { // 16 bit data
      int maxValue = m_enableJVETSNR? (255 << (inp0->m_bitDepth - 8)) : inp0->m_maxPelValue[Y_COMP];
      if (m_enablexPSNR == TRUE && inp0->m_noComponents > 1) {
        m_sse[U_COMP] = (double) computeChromaError(inp0->m_ui16Comp[U_COMP], inp1->m_ui16Comp[U_COMP], inp0->m_compSize[U_COMP], FALSE, m_xPSNRweights[U_COMP]);
        m_sse[V_COMP] = (double) computeChromaError(inp0->m_ui16Comp[V_COMP], inp1->m_ui16Comp[V_COMP], inp0->m_compSize[V_COMP], TRUE,  m_xPSNRweights[V_COMP]);
        m_sse[Y_COMP] = (double) computeLumaError(inp0->m_ui16Comp[Y_COMP], inp1->m_ui16Comp[Y_COMP], inp0->m_width[Y_COMP], inp0->m_height[Y_COMP], shiftwidth, shiftheight, m_xPSNRweights[Y_COMP]);

        for (int c = Y_COMP; c < inp0->m_noComponents; c++) {
          m_sseStats[c].updateStats(m_sse[c]);
          m_mse[c] = m_sse[c] / (double) inp0->m_compSize[c];
          m_mseStats[c].updateStats(m_mse[c]);
          if (m_enableJVETSNR)
            m_metric[c] = m_sse[c] ? psnr(maxValue, inp0->m_compSize[c], m_sse[c]) : 999.99;
          else  
            m_metric[c] = psnr(inp0->m_maxPelValue[c], inp0->m_compSize[c], m_sse[c]);
          m_metricStats[c].updateStats(m_metric[c]);
        }
        m_xPSNR = psnr(maxValue, (int) inp0->m_size, m_oError);
        m_xPSNRStats.updateStats(m_xPSNR);
      }
      else {
        
        for (int c = Y_COMP; c < inp0->m_noComponents; c++) {
          m_sse[c] = (double) compute(inp0->m_ui16Comp[c], inp1->m_ui16Comp[c], inp0->m_compSize[c]);
          
          m_sseStats[c].updateStats(m_sse[c]);
          m_mse[c] = m_sse[c] / (double) inp0->m_compSize[c];
          m_mseStats[c].updateStats(m_mse[c]);
          if (m_enableJVETSNR)
            m_metric[c] = m_sse[c] ? psnr(maxValue, inp0->m_compSize[c], m_sse[c]) : 999.99;
          else  
            m_metric[c] = psnr(inp0->m_maxPelValue[c], inp0->m_compSize[c], m_sse[c]);
          m_metricStats[c].updateStats(m_metric[c]);
        }
      }
    }
  }
  else {
    printf("Frames of different type being compared. Computation will not be performed for this frame.\n");
  }
}


void DistortionMetricPSNR::computeMetric (Frame* inp0, Frame* inp1, int component)
{
  // it is assumed here that the frames are of the same type
  if (inp0->equalType(inp1)) {
    if (inp0->m_isFloat == TRUE) {    // floating point data
      m_sse[component] = compute(inp0->m_floatComp[component], inp1->m_floatComp[component], inp0->m_compSize[component], m_maxValue[component]);
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
      m_sse[component] = (double) compute(inp0->m_comp[component], inp1->m_comp[component], inp0->m_compSize[component]);
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
      int maxValue = m_enableJVETSNR? (255 << (inp0->m_bitDepth - 8)) : inp0->m_maxPelValue[component];
      m_sse[component] = (double) compute(inp0->m_ui16Comp[component], inp1->m_ui16Comp[component], inp0->m_compSize[component]);
      m_sseStats[component].updateStats(m_sse[component]);
      m_mse[component] = m_sse[component] / (double) inp0->m_compSize[component];
      m_mseStats[component].updateStats(m_mse[component]);
      if (m_enableJVETSNR)
        m_metric[component] = m_sse[component] ? psnr(inp0->m_maxPelValue[component], inp0->m_compSize[component], m_sse[component]) : 999.99;
      else  
        m_metric[component] = psnr(maxValue, inp0->m_compSize[component], m_sse[component]);
      m_metricStats[component].updateStats(m_metric[component]);
    }
  }
  else {
    printf("Frames of different type being compared. Computation will not be performed for this frame.\n");
  }
}

void DistortionMetricPSNR::reportMetric  ()
{
  printf("%7.3f %7.3f %7.3f ", m_metric[Y_COMP], m_metric[U_COMP], m_metric[V_COMP]);
  if (m_enableShowMSE == TRUE)
    printf("%10.3f %10.3f %10.3f ", m_mse[Y_COMP], m_mse[U_COMP], m_mse[V_COMP]);
  if (m_enablexPSNR == TRUE)
    printf("%10.3f ", m_xPSNR);
  if (m_enableHexMetric == TRUE) {
    for (int c = 0; c < T_COMP; c++) {
        std::copy(reinterpret_cast<uint8_t *>(&m_metric[c]),
                  reinterpret_cast<uint8_t *>(&m_metric[c]) + sizeof(m_metric[c]),
                  reinterpret_cast<uint8_t *>(&m_hexMetricPSNR[c]));
    }
    printf(" %16" PRIx64 " %16" PRIx64 " %16" PRIx64, m_hexMetricPSNR[Y_COMP], m_hexMetricPSNR[U_COMP], m_hexMetricPSNR[V_COMP]);
    if (m_enablexPSNR == TRUE) {
      std::copy(reinterpret_cast<uint8_t *>(&m_xPSNRStats),
                reinterpret_cast<uint8_t *>(&m_xPSNRStats) + sizeof(m_xPSNRStats),
                reinterpret_cast<uint8_t *>(&m_hexMetricPSNR[0]));
      printf(" %16" PRIx64, m_hexMetricPSNR[0]);
    }
  }
}

void DistortionMetricPSNR::reportSummary  ()
{
  printf("%7.3f %7.3f %7.3f ", m_metricStats[Y_COMP].getAverage(), m_metricStats[U_COMP].getAverage(), m_metricStats[V_COMP].getAverage());
  if (m_enableShowMSE == TRUE)
    printf("%10.3f %10.3f %10.3f ", m_mseStats[Y_COMP].getAverage(), m_mseStats[U_COMP].getAverage(), m_mseStats[V_COMP].getAverage());
  if (m_enablexPSNR == TRUE)
    printf("%10.3f ", m_xPSNRStats.getAverage());
  if (m_enableHexMetric == TRUE) {
    for (int c = 0; c < T_COMP; c++) {
      double val = m_metricStats[c].getAverage();
      std::copy(reinterpret_cast<uint8_t *>(&val),
                reinterpret_cast<uint8_t *>(&val) + sizeof(val),
                reinterpret_cast<uint8_t *>(&m_hexMetricPSNR[c]));
    }
    printf(" %16" PRIx64 " %16" PRIx64 " %16" PRIx64, m_hexMetricPSNR[Y_COMP], m_hexMetricPSNR[U_COMP], m_hexMetricPSNR[V_COMP]);
    if (m_enablexPSNR == TRUE) {
      double val = m_xPSNRStats.getAverage();
      std::copy(reinterpret_cast<uint8_t *>(&val),
                reinterpret_cast<uint8_t *>(&val) + sizeof(val),
                reinterpret_cast<uint8_t *>(&m_hexMetricPSNR[0]));
      printf(" %16" PRIx64, m_hexMetricPSNR[0]);
    }
  }
}

void DistortionMetricPSNR::reportMinimum  ()
{
  printf("%7.3f %7.3f %7.3f ", m_metricStats[Y_COMP].minimum, m_metricStats[U_COMP].minimum, m_metricStats[V_COMP].minimum);
  if (m_enableShowMSE == TRUE)
    printf("%10.3f %10.3f %10.3f ", m_mseStats[Y_COMP].minimum, m_mseStats[U_COMP].minimum, m_mseStats[V_COMP].minimum);
  if (m_enablexPSNR == TRUE)
    printf("%10.3f ", m_xPSNRStats.minimum);
  if (m_enableHexMetric == TRUE) {
    for (int c = 0; c < T_COMP; c++) {
      std::copy(reinterpret_cast<uint8_t *>(&m_metricStats[c].minimum),
                reinterpret_cast<uint8_t *>(&m_metricStats[c].minimum) + sizeof(m_metricStats[c].minimum),
                reinterpret_cast<uint8_t *>(&m_hexMetricPSNR[c]));
    }
    printf(" %16" PRIx64 " %16" PRIx64 " %16" PRIx64, m_hexMetricPSNR[Y_COMP], m_hexMetricPSNR[U_COMP], m_hexMetricPSNR[V_COMP]);
    if (m_enablexPSNR == TRUE) {
      std::copy(reinterpret_cast<uint8_t *>(&m_xPSNRStats.minimum),
                reinterpret_cast<uint8_t *>(&m_xPSNRStats.minimum) + sizeof(m_xPSNRStats.minimum),
                reinterpret_cast<uint8_t *>(&m_hexMetricPSNR[0]));
      printf(" %16" PRIx64, m_hexMetricPSNR[0]);
    }
  }
}

void DistortionMetricPSNR::reportMaximum  ()
{
  printf("%7.3f %7.3f %7.3f ", m_metricStats[Y_COMP].maximum, m_metricStats[U_COMP].maximum, m_metricStats[V_COMP].maximum);
  if (m_enableShowMSE == TRUE)
    printf("%10.3f %10.3f %10.3f ", m_mseStats[Y_COMP].maximum, m_mseStats[U_COMP].maximum, m_mseStats[V_COMP].maximum);
  if (m_enablexPSNR == TRUE)
    printf("%10.3f ", m_xPSNRStats.maximum);
  if (m_enableHexMetric == TRUE) {
    for (int c = 0; c < T_COMP; c++) {
      std::copy(reinterpret_cast<uint8_t *>(&m_metricStats[c].maximum),
                reinterpret_cast<uint8_t *>(&m_metricStats[c].maximum) + sizeof(m_metricStats[c].maximum),
                reinterpret_cast<uint8_t *>(&m_hexMetricPSNR[c]));
    }
    printf(" %16" PRIx64 " %16" PRIx64 " %16" PRIx64, m_hexMetricPSNR[Y_COMP], m_hexMetricPSNR[U_COMP], m_hexMetricPSNR[V_COMP]);
    if (m_enablexPSNR == TRUE) {
      std::copy(reinterpret_cast<uint8_t *>(&m_xPSNRStats.maximum),
                reinterpret_cast<uint8_t *>(&m_xPSNRStats.maximum) + sizeof(m_xPSNRStats.maximum),
                reinterpret_cast<uint8_t *>(&m_hexMetricPSNR[0]));
      printf(" %16" PRIx64, m_hexMetricPSNR[0]);
    }
  }
}

void DistortionMetricPSNR::printHeader()
{
  if (m_isWindow == FALSE ) {
  switch (m_colorSpace) {
    case CM_YCbCr:
      printf("PSNR-Y  "); // 8
      printf("PSNR-U  "); // 8
      printf("PSNR-V  "); // 8
      if (m_enableShowMSE == TRUE) {
        printf("   MSE-Y   "); // 11
        printf("   MSE-U   "); // 11
        printf("   MSE-V   "); // 11
      }
      if (m_enablexPSNR == TRUE) {
        printf(" xPSNRYUV  "); // 11
      }
      if (m_enableHexMetric == TRUE) {
        printf("    hexPSNR-Y    "); // 17
        printf("    hexPSNR-U    "); // 17
        printf("    hexPSNR-V    "); // 17
        if (m_enablexPSNR == TRUE) {
          printf("   hexxPSNRYUV   "); // 11
        }
      }
      break;
    case CM_RGB:
      printf("PSNR-R  "); // 8
      printf("PSNR-G  "); // 8
      printf("PSNR-B  "); // 8
      if (m_enableShowMSE == TRUE) {
        printf("   MSE-R   "); // 11
        printf("   MSE-G   "); // 11
        printf("   MSE-B   "); // 11
      }
      if (m_enablexPSNR == TRUE) {
        printf(" xPSNRRGB  "); // 11
      }
      if (m_enableHexMetric == TRUE) {
        printf("    hexPSNR-R    "); // 17
        printf("    hexPSNR-G    "); // 17
        printf("    hexPSNR-B    "); // 17
        if (m_enablexPSNR == TRUE) {
          printf("   hexxPSNRRGB   "); // 11
        }
      }
      break;
    case CM_XYZ:
      printf("PSNR-X  "); // 8
      printf("PSNR-Y  "); // 8
      printf("PSNR-Z  "); // 8
      if (m_enableShowMSE == TRUE) {
        printf("   MSE-X   "); // 11
        printf("   MSE-Y   "); // 11
        printf("   MSE-Z   "); // 11
      }
      if (m_enablexPSNR == TRUE) {
        printf(" xPSNRXYZ  "); // 11
      }
      if (m_enableHexMetric == TRUE) {
        printf("    hexPSNR-X    "); // 17
        printf("    hexPSNR-Y    "); // 17
        printf("    hexPSNR-Z    "); // 17
        if (m_enablexPSNR == TRUE) {
          printf("   hexxPSNRXYZ   "); // 11
        }
      }
      break;
    default:
      printf("PSNR-C0 "); // 8
      printf("PSNR-C1 "); // 8
      printf("PSNR-C2 "); // 8
      if (m_enableShowMSE == TRUE) {
        printf("   MSE-C0  "); // 11
        printf("   MSE-C1  "); // 11
        printf("   MSE-C2  "); // 11
      }
      if (m_enablexPSNR == TRUE) {
        printf(" xPSNRCMP  "); // 11
      }
      if (m_enableHexMetric == TRUE) {
        printf("    hexPSNR-C0   "); // 17
        printf("    hexPSNR-C1   "); // 17
        printf("    hexPSNR-C2   "); // 17
        if (m_enablexPSNR == TRUE) {
          printf("   hexxPSNRCMP   "); // 11
        }
      }
      break;
  }
}
  else {
    switch (m_colorSpace) {
      case CM_YCbCr:
        printf("wPSNR-Y "); // 8
        printf("wPSNR-U "); // 8
        printf("wPSNR-V "); // 8
        if (m_enableShowMSE == TRUE) {
          printf("  wMSE-Y   "); // 11
          printf("  wMSE-U   "); // 11
          printf("  wMSE-V   "); // 11
        }
        if (m_enablexPSNR == TRUE) {
          printf(" mxPSNRYUV "); // 11
        }
        if (m_enableHexMetric == TRUE) {
          printf("    hexwPSNR-Y   "); // 17
          printf("    hexwPSNR-U   "); // 17
          printf("    hexwPSNR-V   "); // 17
          if (m_enablexPSNR == TRUE) {
            printf("   hexmxPSNRYUV  "); // 11
          }
        }
        break;
      case CM_RGB:
        printf("wPSNR-R "); // 8
        printf("wPSNR-G "); // 8
        printf("wPSNR-B "); // 8
        if (m_enableShowMSE == TRUE) {
          printf("  wMSE-R   "); // 11
          printf("  wMSE-G   "); // 11
          printf("  wMSE-B   "); // 11
        }
        if (m_enablexPSNR == TRUE) {
          printf(" mxPSNRRGB "); // 11
        }
        if (m_enableHexMetric == TRUE) {
          printf("    hexwPSNR-R   "); // 17
          printf("    hexwPSNR-G   "); // 17
          printf("    hexwPSNR-B   "); // 17
          if (m_enablexPSNR == TRUE) {
            printf("   hexmxPSNRRGB  "); // 11
          }
        }
        break;
      case CM_XYZ:
        printf("wPSNR-X "); // 8
        printf("wPSNR-Y "); // 8
        printf("wPSNR-Z "); // 8
        if (m_enableShowMSE == TRUE) {
          printf("  wMSE-X   "); // 11
          printf("  wMSE-Y   "); // 11
          printf("  wMSE-Z   "); // 11
        }
        if (m_enablexPSNR == TRUE) {
          printf(" mxPSNRXYZ "); // 11
        }
        if (m_enableHexMetric == TRUE) {
          printf("    hexwPSNR-X   "); // 17
          printf("    hexwPSNR-Y   "); // 17
          printf("    hexwPSNR-Z   "); // 17
          if (m_enablexPSNR == TRUE) {
            printf("   hexmxPSNRXYZ  "); // 11
          }
        }
        break;
      default:
        printf("wPSNRC0 "); // 8
        printf("wPSNRC1 "); // 8
        printf("wPSNRC2 "); // 8
        if (m_enableShowMSE == TRUE) {
          printf("  wMSE-C0  "); // 11
          printf("  wMSE-C1  "); // 11
          printf("  wMSE-C2  "); // 11
        }
        if (m_enablexPSNR == TRUE) {
          printf(" mxPSNRCMP "); // 11
        }
        if (m_enableHexMetric == TRUE) {
          printf("    hexwPSNRC0   "); // 17
          printf("    hexwPSNRC1   "); // 17
          printf("    hexwPSNRC2   "); // 17
          if (m_enablexPSNR == TRUE) {
            printf("   hexmxPSNRCMP  "); // 11
          }
        }
        break;
    }
  }
}

void DistortionMetricPSNR::printSeparator(){
  printf("--------");
  printf("--------");
  printf("--------");
  if (m_enableShowMSE == TRUE) {
    printf("-----------");
    printf("-----------");
    printf("-----------");
  }
  if (m_enablexPSNR == TRUE) {
    printf("-----------");
  }
  if (m_enableHexMetric == TRUE) {
    printf("-----------------");
    printf("-----------------");
    printf("-----------------");
    if (m_enablexPSNR == TRUE) {
      printf("-----------------");
    }
  }
}
} // namespace hdrtoolslib
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
