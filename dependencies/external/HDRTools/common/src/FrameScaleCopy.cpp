/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Apple Inc.
 * <ORGANIZATION> = Apple Inc.
 * <YEAR> = 2017
 *
 * Copyright (c) 2017, Apple Inc.
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
 * \file FrameScaleCopy.cpp
 *
 * \brief
 *    Copy data in available area
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "Global.H"
#include "FrameScaleCopy.H"
#include <string.h>

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

namespace hdrtoolslib {

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

FrameScaleCopy::FrameScaleCopy(int iWidth, int iHeight, int oWidth, int oHeight) {
  /* if resampling is actually downsampling we have to EXTEND the length of the
  original filter; the ratio is calculated below */
  m_factorX = (double)(iWidth ) / (double) (oWidth );
  m_factorY = (double)(iHeight) / (double) (oHeight);

  // nearest neighbor uses 2 taps so we can fit the implementation within the generic
  // linear separable filter implementation that we have. The tap values are determined
  // to be equal to either 1 or 0 depending on distance.
  m_filterTapsX = 1;
  m_filterTapsY = 1;

  //  m_offsetX = m_factorX > 1.0 ? 1 / m_factorX : 0.0;
  //  m_offsetY = m_factorY > 1.0 ? 1 / m_factorY : 0.0;
  m_offsetX = 0.0;
  m_offsetY = 0.0;
}

//FrameScaleCopy::~FrameScaleCopy() {
//}

//-----------------------------------------------------------------------------
// Private Methods
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void FrameScaleCopy::process ( Frame* out, const Frame *inp)
{
  if (( out->m_isFloat != inp->m_isFloat ) || (( inp->m_isFloat == 0 ) && ( out->m_bitDepth != inp->m_bitDepth ))) {
    fprintf(stderr, "Error: trying to copy frames of different data types. \n");
    exit(EXIT_FAILURE);
  }
    
  int c, i, j;
  
  out->m_frameNo = inp->m_frameNo;
  out->m_isAvailable = TRUE;
  
  for (c = Y_COMP; c <= V_COMP; c++) {
    out->m_minPelValue[c]  = inp->m_minPelValue[c];
    out->m_midPelValue[c]  = inp->m_midPelValue[c];
    out->m_maxPelValue[c]  = inp->m_maxPelValue[c];
  }
  
  if (out->m_isFloat == TRUE) {    // floating point data
    for (c = Y_COMP; c <= V_COMP; c++) {
      for (i = 0; i < iMin(inp->m_height[c], out->m_height[c]); i++) {
        for (j = 0; j < iMin(inp->m_width[c], out->m_width[c]); j++) {
          out->m_floatComp[c][i * out->m_width[c] + j] = inp->m_floatComp[c][i * inp->m_width[c] + j];
        }
      }
    }
  }
  else if (out->m_bitDepth == 8) {   // 8 bit data
    for (c = Y_COMP; c <= V_COMP; c++) ;
    for (i = 0; i < iMin(inp->m_height[c], out->m_height[c]); i++) {
      for (j = 0; j < iMin(inp->m_width[c], out->m_width[c]); j++) {
        out->m_comp[c][i * out->m_width[c] + j] = inp->m_comp[c][i * inp->m_width[c] + j];
      }
    }
  }
  else { // 16 bit data
    for (c = Y_COMP; c <= V_COMP; c++) {
      for (i = 0; i < iMin(inp->m_height[c], out->m_height[c]); i++) {
        for (j = 0; j < iMin(inp->m_width[c], out->m_width[c]); j++) {
          out->m_ui16Comp[c][i * out->m_width[c] + j] = inp->m_ui16Comp[c][i * inp->m_width[c] + j];
        }
      }
    }
  }
}
} // namespace hdrtoolslib


//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
