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
 * \file GaussianFilter.H
 *
 * \brief
 *    GaussianFilter class Header
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */


#ifndef __GaussianFilter_H__
#define __GaussianFilter_H__

#include "Global.H"
#include "Frame.H"

namespace hdrtoolslib {

class GaussianFilter {
private:
  std::vector<std::vector<double> >  m_filter;
  int     m_sizeX;
  int     m_sizeY;
  int     m_midX;
  int     m_midY;
  double filter(double *inp, int pos_y, int pos_x, int width, int height);  
  double filter(float  *inp, int pos_y, int pos_x, int width, int height);  
  double filter(imgpel *inp, int pos_y, int pos_x, int width, int height);  
  double filter(uint16 *inp, int pos_y, int pos_x, int width, int height);  

public:

  // Should also add safe/unsafe filter functions to make this
  // process faster. TBD.


  void compute(double *inp, double *out, int width, int height);  
  void compute(float  *inp, double *out, int width, int height);  
  void compute(imgpel *inp, double *out, int width, int height);  
  void compute(uint16 *inp, double *out, int width, int height);  
  
  
  GaussianFilter(int sizeX, int sizeY, double radius, bool verbose = FALSE);
  ~GaussianFilter();
};
} // namespace hdrtoolslib

#endif
