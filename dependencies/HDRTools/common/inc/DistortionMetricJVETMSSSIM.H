/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = ITU/ISO/IEC
 * <ORGANIZATION> = ITU/ISO/IEC
 * <YEAR> = 2021
 *
 * Copyright (c) (c) 2010-2021, ITU/ISO/IEC
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
 * \file DistortionMetricJVETMSSSIM.H
 *
 * \brief
 *    MS-SSIM distortion computation class
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */


#ifndef __DistortionMetricJVETMSSSIM_H__
#define __DistortionMetricJVETMSSSIM_H__

#include "Global.H"
#include "Frame.H"
#include "DistortionMetric.H"
#include <cinttypes>

#ifndef UNBIASED_VARIANCE
//#define UNBIASED_VARIANCE // unbiased estimation of the variance
#endif

namespace hdrtoolslib {

class DistortionMetricJVETMSSSIM : public DistortionMetric {
private:
  ColorSpace        m_colorSpace;
  double            m_maxValue[T_COMP];
  
  int  m_blockDistance;
  int  m_blockSizeX;
  int  m_blockSizeY;
  bool m_useLogSSIM;
  bool m_enableHexMetric;
  uint64_t m_hexMetricMSSSIM[T_COMP];  
  
  float compute                     (uint8  *inp0Data, uint8  *inp1Data, int height, int width, int comp, int   maxPixelValue);
  float compute                     (uint16 *inp0Data, uint16 *inp1Data, int height, int width, int comp, int   maxPixelValue);
  float compute                     (float  *inp0Data, float  *inp1Data, int height, int width, int comp, float maxPixelValue);
public:
  // Construct/Deconstruct
  DistortionMetricJVETMSSSIM(const FrameFormat *format, SSIMParams *params, double maxSampleValue, bool enableHexMetric = false);
  virtual ~DistortionMetricJVETMSSSIM();
  
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


