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
 * \file DistortionMetricGSSIM.H
 *
 * \brief
 *    SSIM distortion computation class using a Gaussian weighting function
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


#ifndef __DistortionMetricGSSIM_H__
#define __DistortionMetricGSSIM_H__

#include "Global.H"
#include "Frame.H"
#include "DistortionMetric.H"
#include "GaussianFilter.H"

#include <cinttypes>

#ifndef UNBIASED_VARIANCE
//#define UNBIASED_VARIANCE // unbiased estimation of the variance
#endif

namespace hdrtoolslib {

class DistortionMetricGSSIM : public DistortionMetric {
private:
  ColorSpace        m_colorSpace;
  double            m_maxValue[T_COMP];

  double            m_K1;
  double            m_K2;
  double            m_C1;
  double            m_C2;
  int               m_width;
  int               m_height;  
  int               m_size;

  uint64_t          m_hexMetricSSIM[T_COMP];
  GaussianFilter    *m_filter;
  std::vector<double>   m_muX2;
  std::vector<double>   m_muY2;
  std::vector<double>   m_muXY;
  std::vector<double>   m_squareX;
  std::vector<double>   m_squareY;
  std::vector<double>   m_mulXY;
  std::vector<double>   m_sigmaX2;
  std::vector<double>   m_sigmaY2;
  std::vector<double>   m_sigmaXY;
  //std::vector<float>   m_num;
  //std::vector<float>   m_den;
  //std::vector<float>   m_ssimMap;
  
  int  m_useLogSSIM;
  bool m_enableHexMetric;
  
  
  float compute                     (uint8  *inp0Data, uint8  *inp1Data, int height, int width, int comp, int   maxPixelValue);
  float compute                     (uint16 *inp0Data, uint16 *inp1Data, int height, int width, int comp, int   maxPixelValue);
  float compute                     (float  *inp0Data, float  *inp1Data, int height, int width, int comp, float maxPixelValue);
  
  void horizontalSymmetricExtension(int *buffer, int width, int height);
  void horizontalSymmetricExtension(float *buffer, int width, int height );

  void verticalSymmetricExtension  (int *buffer, int width, int height);
  void verticalSymmetricExtension  (float *buffer, int width, int height);
  
  void downsample                    (const uint8*  src, uint8*  out, int iWidth, int iHeight, int oWidth, int oHeight);
  void downsample                    (const uint16* src, uint16* out, int iWidth, int iHeight, int oWidth, int oHeight);
  void downsample                    (const float*  src, float*  out, int iWidth, int iHeight, int oWidth, int oHeight);

  void padImage                      (const uint8*  src, int   *buffer, int width, int height);
  void padImage                      (const uint16* src, int   *buffer, int width, int height);
  void padImage                      (const float*  src, float *buffer, int width, int height);
public:
  // Construct/Deconstruct
  DistortionMetricGSSIM(const FrameFormat *format, SSIMParams *params, double maxSampleValue, bool enableHexMetric);
  virtual ~DistortionMetricGSSIM();
  
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


