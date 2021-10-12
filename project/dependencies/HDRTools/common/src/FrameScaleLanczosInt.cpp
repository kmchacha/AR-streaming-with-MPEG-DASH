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
 * \file FrameScaleLanczosInt.cpp
 *
 * \brief
 *    Scale using Lanczos interpolation (integer precision)
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
#include "FrameScaleLanczosInt.H"
#include <string.h>

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

namespace hdrtoolslib {

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

FrameScaleLanczosInt::FrameScaleLanczosInt(int iWidth, int iHeight, int oWidth, int oHeight, int lanczosLobes, ChromaLocation chromaLocationType, int offsetMethod, int precision) {

  m_lobes = lanczosLobes;
  m_lobesX = lanczosLobes;
  m_lobesY = lanczosLobes;
  // Should this ideally be half the lobes of luma depending of course on chroma type? 
  // What is sometimes done in practice is to convert 4:2:0 data to 4:4:4 and then apply the same
  // filters for luma and chroma (no issue with phases then). Maybe add a control parameter for chroma.
  m_chromaLobesX = lanczosLobes;
  m_chromaLobesY = lanczosLobes;
  
  m_pi = (double) M_PI  / (double) m_lobes;
  m_piSq = (double) M_PI * m_pi;

  /* if resampling is actually downsampling we have to EXTEND the length of the
  original filter; the ratio is calculated below */
  m_factorY = (double)(iHeight) / (double) (oHeight);
  m_factorX = (double)(iWidth ) / (double) (oWidth );
  //offsetMethod = 1;
  if (offsetMethod == 1) {
    m_offsetX = m_factorX > 1.0 ? 1 / 2.0 : 2.0;
    m_offsetY = m_factorY > 1.0 ? 1 / 2.0 : 2.0;
    // Note that filtering for a particular dimension is overwritten depending on the chroma format.
    // The values here are specified basically for 4:2:0 formats
    
    /*
     CL_ZERO  = 0,   // middle aligned vertically, left aligned (co-sited) horizontally
     CL_ONE   = 1,   // center aligned vertically & horizontally
     CL_TWO   = 2,   // top aligned (co-sited) vertically, left aligned (co-sited) horizontally
     CL_THREE = 3,   // top aligned (co-sited) vertically, middle aligned horizontally 
     CL_FOUR  = 4,   // bottom aligned (co-sited) vertically, left aligned (co-sited) horizontally
     CL_FIVE  = 5,   // bottom aligned (co-sited) vertically, middle aligned horizontally
     */
    if (chromaLocationType == CL_ZERO) {
      m_chromaOffsetX = m_factorX > 1.0 ? 1 / m_factorX : m_factorX;
      m_chromaOffsetY = m_factorY > 1.0 ? 1 / m_factorY : m_factorX;
    }
    else if (chromaLocationType == CL_ONE) {
      m_chromaOffsetX = m_factorX > 1.0 ? 1 / m_factorX : m_factorX;
      m_chromaOffsetY = m_factorY > 1.0 ? 1 / m_factorY : m_factorX;
    }
    else if (chromaLocationType == CL_TWO) {
      m_chromaOffsetX = m_factorX > 1.0 ? 1 / m_factorX : m_factorX;
      m_chromaOffsetY = m_factorY > 1.0 ? 1 / m_factorY : m_factorX;
    }
    else if (chromaLocationType == CL_THREE) {
      m_chromaOffsetX = m_factorX > 1.0 ? 1 / m_factorX : m_factorX;
      m_chromaOffsetY = m_factorY > 1.0 ? 1 / m_factorY : m_factorX;
    }
    else if (chromaLocationType == CL_FOUR) {
      m_chromaOffsetX = m_factorX > 1.0 ? 1 / m_factorX : m_factorX;
      m_chromaOffsetY = m_factorY > 1.0 ? 1 / m_factorY : m_factorX;
    }
    else if (chromaLocationType == CL_FIVE) {
      m_chromaOffsetX = m_factorX > 1.0 ? 1 / m_factorX : m_factorX;
      m_chromaOffsetY = m_factorY > 1.0 ? 1 / m_factorY : m_factorX;
    }
  }
  else {
    m_offsetX = 0.0;
    m_offsetY = 0.0;
    if (chromaLocationType == CL_ZERO) {
      m_chromaOffsetX = 0.0;
      m_chromaOffsetY = m_factorY > 1.0 ? 1 / m_factorY : m_factorX; // TO be derived
    }
    else if (chromaLocationType == CL_ONE) {
      m_chromaOffsetX = m_factorX > 1.0 ? 1 / m_factorX : m_factorX; // TO be derived
      m_chromaOffsetY = m_factorY > 1.0 ? 1 / m_factorY : m_factorX; // TO be derived
    }
    else if (chromaLocationType == CL_TWO) {
      m_chromaOffsetX = 0.0;
      m_chromaOffsetY = 0.0;
    }
    else if (chromaLocationType == CL_THREE) {
      m_chromaOffsetX = m_factorX > 1.0 ? 1 / m_factorX : m_factorX; // TO be derived
      m_chromaOffsetY = 0.0;
    }
    else if (chromaLocationType == CL_FOUR) {
      m_chromaOffsetX = 0.0;
      m_chromaOffsetY = 0.0;
    }
    else if (chromaLocationType == CL_FIVE) {
      m_chromaOffsetX = m_factorX > 1.0 ? 1 / m_factorX : m_factorX; // TO be derived
      m_chromaOffsetY = 0.0;
    }
  }

    // We could create a function here that allows us to generate the filters for one dimension 
  // and that can be reused easily for luma/chroma X/Y. To be done later.
  
  // Allocate filter memory
  m_filterTapsX = (m_factorX == 1.0 ) ? 1 : (m_factorX > 1.0 ) ? (int) ceil (m_factorX * 2 * m_lobesX) :  2 * m_lobesX;
  m_filterOffsetsX.resize(m_filterTapsX);
  // We basically allocate filter coefficients for all target positions.
  // This is done once and saves us time from deriving the proper filter for each position.
  m_filterCoeffsX.resize(oWidth * m_filterTapsX);
  m_filterIntCoeffsX.resize(oWidth * m_filterTapsX);
  
  // Initialize the filter boundaries
  SetFilterLimits((int *) &m_filterOffsetsX[0], m_filterTapsX, iWidth,  oWidth,  m_factorX, &m_iMinX, &m_iMaxX, &m_oMinX, &m_oMaxX);
  // Finally prepare the filter coefficients for horizontal and vertical filtering
  // Horizontal
  PrepareFilterCoefficients((double *) &m_filterCoeffsX[0], (int *) &m_filterIntCoeffsX[0], (int *) &m_filterOffsetsX[0], m_factorX, m_filterTapsX, m_offsetX, oWidth, m_lobesX, precision);
  

  // Allocate filter memory
  m_filterTapsY = (m_factorY == 1.0 ) ? 1 : (m_factorY > 1.0 ) ? (int) ceil (m_factorY * 2 * m_lobesY) :  2 * m_lobesY;
  m_filterOffsetsY.resize(m_filterTapsY);
  // We basically allocate filter coefficients for all target positions.
  // This is done once and saves us time from deriving the proper filter for each position.
  m_filterCoeffsY.resize(oHeight * m_filterTapsY);
  m_filterIntCoeffsY.resize(oHeight * m_filterTapsY);
  
  // Initialize the filter boundaries
  SetFilterLimits((int *) &m_filterOffsetsY[0], m_filterTapsY, iHeight, oHeight, m_factorY, &m_iMinY, &m_iMaxY, &m_oMinY, &m_oMaxY);
  // Finally prepare the filter coefficients for horizontal and vertical filtering
  // Vertical
  PrepareFilterCoefficients((double *) &m_filterCoeffsY[0], (int *) &m_filterIntCoeffsY[0], (int *) &m_filterOffsetsY[0], m_factorY, m_filterTapsY, m_offsetY, oHeight, m_lobesY, precision);

  // Allocate filter memory  
  m_chromaFilterTapsX = (m_factorX == 1.0 ) ? 1 : (m_factorX > 1.0 ) ? (int) ceil (m_factorX * 2 * m_chromaLobesX) :  2 * m_chromaLobesX;
  m_chromaFilterOffsetsX.resize(m_chromaFilterTapsX);
  // We basically allocate filter coefficients for all target positions.
  // This is done once and saves us time from deriving the proper filter for each position.
  m_chromaFilterCoeffsX.resize(oWidth  / 2 * m_chromaFilterTapsX);
  m_chromaFilterIntCoeffsX.resize(oWidth  / 2 * m_chromaFilterTapsX);

  // Initialize the filter boundaries
  SetFilterLimits((int *) &m_chromaFilterOffsetsX[0], m_chromaFilterTapsX, iWidth / 2,  oWidth / 2,  m_factorX, &m_iChromaMinX, &m_iChromaMaxX, &m_oChromaMinX, &m_oChromaMaxX);
  // Horizontal
  PrepareFilterCoefficients((double *) &m_chromaFilterCoeffsX[0], (int *) &m_chromaFilterIntCoeffsX[0], (int *) &m_chromaFilterOffsetsX[0], m_factorX, m_chromaFilterTapsX, m_chromaOffsetX, oWidth / 2, m_chromaLobesX, precision);
  
  // Allocate filter memory
  m_chromaFilterTapsY = (m_factorY == 1.0 ) ? 1 : (m_factorY > 1.0 ) ? (int) ceil (m_factorY * 2 * m_chromaLobesY) :  2 * m_chromaLobesY;
  m_chromaFilterOffsetsY.resize(m_chromaFilterTapsY);
  // We basically allocate filter coefficients for all target positions.
  // This is done once and saves us time from deriving the proper filter for each position.
  m_chromaFilterCoeffsY.resize(oHeight / 2 * m_chromaFilterTapsY);
  m_chromaFilterIntCoeffsY.resize(oHeight / 2 * m_chromaFilterTapsY);

  // Initialize the filter boundaries
  SetFilterLimits((int *) &m_filterOffsetsY[0], m_filterTapsY, iHeight / 2, oHeight / 2, m_factorY, &m_iChromaMinY, &m_iChromaMaxY, &m_oChromaMinY, &m_oChromaMaxY);
  // Vertical
  PrepareFilterCoefficients((double *) &m_chromaFilterCoeffsY[0], (int *) &m_chromaFilterIntCoeffsY[0], (int *) &m_chromaFilterOffsetsY[0], m_factorY, m_chromaFilterTapsY, m_chromaOffsetY, oHeight / 2, m_chromaLobesY, precision);
}

//FrameScaleLanczosInt::~FrameScaleLanczosInt() {
//}

//-----------------------------------------------------------------------------
// Private Methods
//-----------------------------------------------------------------------------


double FrameScaleLanczosInt::FilterTap( double dist, double piScaled, double factor, int lobes ) {
  dist = dAbs(dist);

  double L = lobes * factor;
  double tap;
  
  if ( dist > L )
    return 0.0;
  else {
    if (dist > 0.0) {
      double xL = M_PI * dist / L;
      double wTerm = sin ( xL ) / xL;
      double sTerm = sin( M_PI * dist / factor)/(M_PI * dist / factor);
      double gTerm = wTerm * sTerm;
      tap = gTerm;
    }
    else {
      tap = 1.0;
    }
    return tap;
  }
}


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void FrameScaleLanczosInt::process ( Frame* out, const Frame *inp)
{
  if (( out->m_isFloat != inp->m_isFloat ) || (( inp->m_isFloat == 0 ) && ( out->m_bitDepth != inp->m_bitDepth ))) {
    fprintf(stderr, "Error: trying to copy frames of different data types. \n");
    exit(EXIT_FAILURE);
  }
    
  int c;
  
  out->m_frameNo = inp->m_frameNo;
  out->m_isAvailable = TRUE;
  
  for (c = Y_COMP; c <= V_COMP; c++) {
    out->m_minPelValue[c]  = inp->m_minPelValue[c];
    out->m_midPelValue[c]  = inp->m_midPelValue[c];
    out->m_maxPelValue[c]  = inp->m_maxPelValue[c];
  }
  
  if (out->m_isFloat == TRUE) {    // floating point data... ignore this for now
    filter( inp->m_floatComp[Y_COMP], out->m_floatComp[Y_COMP], inp->m_width[Y_COMP], inp->m_height[Y_COMP], out->m_width[Y_COMP], out->m_height[Y_COMP], (double) out->m_minPelValue[Y_COMP], (double) out->m_maxPelValue[Y_COMP] );
    // Here we should pass the chroma format... TBD
    for (c = U_COMP; c <= V_COMP; c++) {
      filter( inp->m_floatComp[c], out->m_floatComp[c], inp->m_width[c], inp->m_height[c], out->m_width[c], out->m_height[c], (double) out->m_minPelValue[c], (double) out->m_maxPelValue[c] );
    }
  }
  else if (out->m_bitDepth == 8) {   // 8 bit data
    filter( inp->m_comp[Y_COMP], out->m_comp[Y_COMP], inp->m_width[Y_COMP], inp->m_height[Y_COMP], out->m_width[Y_COMP], out->m_height[Y_COMP], out->m_minPelValue[Y_COMP], out->m_maxPelValue[Y_COMP] );
    // Here we should pass the chroma format... TBD
    for (c = U_COMP; c <= V_COMP; c++) {
      filter( inp->m_comp[c], out->m_comp[c], inp->m_width[c], inp->m_height[c], out->m_width[c], out->m_height[c], out->m_minPelValue[c], out->m_maxPelValue[c] );
    }
  }
  else { // 16 bit data
    filter( inp->m_ui16Comp[Y_COMP], out->m_ui16Comp[Y_COMP], inp->m_width[Y_COMP], inp->m_height[Y_COMP], out->m_width[c], out->m_height[Y_COMP], out->m_minPelValue[Y_COMP], out->m_maxPelValue[Y_COMP] );
    // Here we should pass the chroma format... TBD
    for (c = U_COMP; c <= V_COMP; c++) {
      filter( inp->m_ui16Comp[c], out->m_ui16Comp[c], inp->m_width[c], inp->m_height[c], out->m_width[c], out->m_height[c], out->m_minPelValue[c], out->m_maxPelValue[c] );
    }
  }
}

} // namespace hdrtoolslib


//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
