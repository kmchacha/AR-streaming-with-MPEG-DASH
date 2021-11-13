/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Apple Inc., Qualcomm Inc.
 * <ORGANIZATION> = Apple Inc., Qualcomm Inc.
 * <YEAR> = 2017
 *
 * Copyright (c) 2017, Apple Inc., Qualcomm Inc.
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
 * \file DistortionMetricWTPSNR.H
 *
 * \brief
 *    Weighted PSNR distortion computation class
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *     - Adarsh Krishnan Ramasubramonian <aramasub@qti.qualcomm.com>
 *
 *************************************************************************************
 */


#ifndef __DistortionMetricWTPSNR_H__
#define __DistortionMetricWTPSNR_H__

#include "Global.H"
#include "Frame.H"
#include "DistortionMetric.H"
#include <cinttypes>

namespace hdrtoolslib {

class DistortionMetricWTPSNR : public DistortionMetric {
private:
  ColorSpace        m_colorSpace;
  bool              m_enableShowMSE;
  bool              m_enableJVETSNR;
  bool              m_enableHexMetric;
  double            m_maxValue[T_COMP];
  double            m_sse[T_COMP];
  double            m_mse[T_COMP];
  double            m_weights[WT_DISTORTION_COUNT];
  uint64_t          m_hexMetricWTPSNR[T_COMP];
  
  MetricStatistics  m_sseStats[T_COMP];
  MetricStatistics  m_mseStats[T_COMP];

  double compute(const float *iComp0, const float *iComp1, const float *YComp0, int height, int width, int lHeight, int lWidth, double maxValue);
  double compute(const uint16 *iComp0, const uint16 *iComp1, const uint16 *YComp0, int height, int width, int lHeight, int lWidth, int shift);
  double compute(const uint8 *iComp0, const uint8 *iComp1, const uint8 *YComp0, int height, int width, int lHeight, int lWidth);
  
  
public:
  // Construct/Deconstruct
  DistortionMetricWTPSNR(const FrameFormat *format, bool enableShowMSE, bool enableJVETSNR, double maxSampleValue, float *weights, bool enableHexMetric = false);
  virtual ~DistortionMetricWTPSNR();
  
  virtual void   computeMetric (Frame* inp0, Frame* inp1);                // Compute metric for all components
  virtual void   computeMetric (Frame* inp0, Frame* inp1, int component); // Compute metric for only one component
  virtual void   reportMetric  ();                                        // report frame level results
  virtual void   reportSummary ();                                        // report summary results
  virtual void   reportMinimum ();
  virtual void   reportMaximum ();
  virtual void   printHeader   ();
  virtual void   printSeparator();
};
} // namespace hdrtoolslib

#endif


