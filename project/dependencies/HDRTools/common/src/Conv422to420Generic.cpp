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
 * \file Conv422to420Generic.cpp
 *
 * \brief
 *    Convert 444 to 420 using a Generic separable filter approach
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
#include "Conv422to420Generic.H"
#include "ScaleFilter.H"
#include <string.h>

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

namespace hdrtoolslib {

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

Conv422to420Generic::Conv422to420Generic(int width, int height, int method, ChromaLocation chromaLocationType[2]) {
  int offset, scale;
  int vPhase;
    
  // Currently we only support progressive formats, and thus ignore the bottom chroma location type
  switch (chromaLocationType[FP_FRAME]) {
    case CL_FIVE:   // This is not yet correct. I need to add an additional phase to support this. TBD
      vPhase = 0;
      break;
    case CL_FOUR:  // This is not yet correct. I need to add an additional phase to support this. TBD
      vPhase = 0;
      break;
    case CL_THREE:
      vPhase = 0;
      break;
    case CL_TWO:
      vPhase = 0;
      break;
    case CL_ONE:
      vPhase = 1;
      break;
    case CL_ZERO:
    default:
      vPhase = 1;
      break;
  }
  
  m_verFilter = new ScaleFilter(method, 0,  2, 0, 0, &offset, &scale, vPhase);
}

Conv422to420Generic::~Conv422to420Generic() {
  if (m_verFilter == NULL)
    delete m_verFilter;
  m_verFilter = NULL;
}

float Conv422to420Generic::filterVertical(const float *inp, const ScaleFilter *filter, int pos_y, int width, int height, float minValue, float maxValue) {
  int i;
  float value = 0.0;
  for (i = 0; i < filter->m_numberOfTaps; i++) {
    value += filter->m_floatFilter[i] * inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
  }
  
  if (filter->m_clip == TRUE)
    return fClip((value + filter->m_floatOffset) * filter->m_floatScale, minValue, maxValue);
  else
    return (value + filter->m_floatOffset) * filter->m_floatScale;
}


int Conv422to420Generic::filterVertical(const int16 *inp, const ScaleFilter *filter, int pos_y, int width, int height, int minValue, int maxValue) {
  int i;
  int value = 0;
  for (i = 0; i < filter->m_numberOfTaps; i++) {
    value += filter->m_i32Filter[i] * inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
  }

  if (filter->m_clip == TRUE)
    return iClip((value + filter->m_i32Offset) >> filter->m_i32Shift, minValue, maxValue);
  else
    return (value + filter->m_i32Offset) >> filter->m_i32Shift;
}

int Conv422to420Generic::filterVertical(const uint16 *inp, const ScaleFilter *filter, int pos_y, int width, int height, int minValue, int maxValue) {
  int i;
  int value = 0;
  for (i = 0; i < filter->m_numberOfTaps; i++) {
    value += filter->m_i32Filter[i] * inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
  }
  
  if (filter->m_clip == TRUE)
    return iClip((value + filter->m_i32Offset) >> filter->m_i32Shift, minValue, maxValue);
  else
    return (value + filter->m_i32Offset) >> filter->m_i32Shift;
}

int Conv422to420Generic::filterVertical(const imgpel *inp, const ScaleFilter *filter, int pos_y, int width, int height, int minValue, int maxValue) {
  int i;
  int value = 0;
  for (i = 0; i < filter->m_numberOfTaps; i++) {
    value += filter->m_i32Filter[i] * inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
  }
  
  if (filter->m_clip == TRUE)
    return iClip((value + filter->m_i32Offset) >> filter->m_i32Shift, minValue, maxValue);
  else
    return (value + filter->m_i32Offset) >> filter->m_i32Shift;
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
void Conv422to420Generic::filter(float *out, const float *inp, int width, int height, float minValue, float maxValue)
{
  int i, j;
  int inputHeight = 2 * height;

  for (j = 0; j < height; j++) {
    for (i = 0; i < width; i++) {
      out[ j * width + i ] = filterVertical(&inp[i], m_verFilter, (2 * j), width, inputHeight - 1, minValue, maxValue);
    }
  }
}

void Conv422to420Generic::filter(uint16 *out, const uint16 *inp, int width, int height, int minValue, int maxValue)
{
  int i, j;
  int inputHeight = 2 * height;
  
  for (j = 0; j < height; j++) {
    for (i = 0; i < width; i++) {
      out[ j * width + i ] = filterVertical(&inp[i], m_verFilter, (2 * j), width, inputHeight - 1, minValue, maxValue);
    }
  }
}

void Conv422to420Generic::filter(imgpel *out, const imgpel *inp, int width, int height, int minValue, int maxValue)
{
  int i, j;
  int inputHeight = 2 * height;
  
  for (j = 0; j < height; j++) {
    for (i = 0; i < width; i++) {
      out[ j * width + i ] = filterVertical(&inp[i], m_verFilter, (2 * j), width, inputHeight - 1, minValue, maxValue);
    }
  }
}

void Conv422to420Generic::filterInterlaced(float *out, float *inp, int width, int height, float minValue, float maxValue)
{
  int i, j;
  
  float *iChromaLineCurrent, *iChromaLineNext, *oChromaLineCurrent;
  
  // process top field
  for (j = ZERO; j < height; j += 2) {
    oChromaLineCurrent = &out[j * width];
    iChromaLineCurrent = &inp[(j << 1) * width];
    iChromaLineNext = &inp[((j + 1) << 1) * width];
    
    for (i = ZERO; i < width; i++) {
      *(oChromaLineCurrent++) = (3.0f * *(iChromaLineCurrent++) + *(iChromaLineNext++) + 2.0f) / 4.0f;
    }
  }
  
  // process bottom field
  for (j = ONE; j < height; j += 2) {
    oChromaLineCurrent = &out[j * width];
    iChromaLineCurrent = &inp[((j << 1) - 1) * width];
    iChromaLineNext = &inp[(((j + 1) << 1) - 1) * width];
    
    for (i = ZERO; i < width; i++) {
      *(oChromaLineCurrent++) = (*(iChromaLineCurrent++) + 3.0f * *(iChromaLineNext++) + 2.0f) / 4.0f;
    }
  }
}

void Conv422to420Generic::filterInterlaced(uint16 *out, uint16 *inp, int width, int height, int minValue, int maxValue)
{
  int i, j;
  
  uint16 *iChromaLineCurrent, *iChromaLineNext, *oChromaLineCurrent;
  
  // process top field
  for (j = ZERO; j < height; j += 2) {
    oChromaLineCurrent = &out[j * width];
    iChromaLineCurrent = &inp[(j << 1) * width];
    iChromaLineNext = &inp[((j + 1) << 1) * width];
    
    for (i = ZERO; i < width; i++) {
      *(oChromaLineCurrent++) = (3 * *(iChromaLineCurrent++) + *(iChromaLineNext++) + 2) >> 2;
    }
  }
  
  // process bottom field
  for (j = ONE; j < height; j += 2) {
    oChromaLineCurrent = &out[j * width];
    iChromaLineCurrent = &inp[((j << 1) - 1) * width];
    iChromaLineNext = &inp[(((j + 1) << 1) - 1) * width];
    
    for (i = ZERO; i < width; i++) {
      *(oChromaLineCurrent++) = (*(iChromaLineCurrent++) + 3 * *(iChromaLineNext++) + 2) >> 2;
    }
  }
}

void Conv422to420Generic::filterInterlaced(imgpel *out, imgpel *inp, int width, int height, int minValue, int maxValue)
{
  int i, j;
  
  imgpel *iChromaLineCurrent, *iChromaLineNext, *oChromaLineCurrent;
  
  // process top field
  for (j = ZERO; j < height; j += 2) {
    oChromaLineCurrent = &out[j * width];
    iChromaLineCurrent = &inp[(j << 1) * width];
    iChromaLineNext = &inp[((j + 1) << 1) * width];
    
    for (i = ZERO; i < width; i++) {
      *(oChromaLineCurrent++) = (3 * *(iChromaLineCurrent++) + *(iChromaLineNext++) + 2) >> 2;
    }
  }
  
  // process bottom field
  for (j = ONE; j < height; j += 2) {
    oChromaLineCurrent = &out[j * width];
    iChromaLineCurrent = &inp[((j << 1) - 1) * width];
    iChromaLineNext = &inp[(((j + 1) << 1) - 1) * width];
    
    for (i = ZERO; i < width; i++) {
      *(oChromaLineCurrent++) = (*(iChromaLineCurrent++) + 3 * *(iChromaLineNext++) + 2) >> 2;
    }
  }
}
//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void Conv422to420Generic::process ( Frame* out, const Frame *inp)
{
  if (( out->m_isFloat != inp->m_isFloat ) || (( inp->m_isFloat == 0 ) && ( out->m_bitDepth != inp->m_bitDepth ))) {
    fprintf(stderr, "Error: trying to copy frames of different data types. \n");
    exit(EXIT_FAILURE);
  }
  
  if (out->m_compSize[Y_COMP] != inp->m_compSize[Y_COMP]) {
    fprintf(stderr, "Error: trying to copy frames of different sizes (%d  vs %d). \n",out->m_compSize[Y_COMP], inp->m_compSize[Y_COMP]);
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
  
  if (inp->m_format.m_isInterlaced == TRUE) {
    if (out->m_isFloat == TRUE) {    // floating point data
      memcpy(out->m_floatComp[Y_COMP], inp->m_floatComp[Y_COMP], (int) out->m_compSize[Y_COMP] * sizeof(float));
      for (c = U_COMP; c <= V_COMP; c++) {
        filterInterlaced(out->m_floatComp[c], inp->m_floatComp[c], out->m_width[c], out->m_height[c], (float) out->m_minPelValue[c], (float) out->m_maxPelValue[c] );
      }
    }
    else if (out->m_bitDepth == 8) {   // 8 bit data
      memcpy(out->m_comp[Y_COMP], inp->m_comp[Y_COMP], (int) out->m_compSize[Y_COMP] * sizeof(imgpel));
      for (c = U_COMP; c <= V_COMP; c++) {
        filterInterlaced(out->m_comp[c], inp->m_comp[c], out->m_width[c], out->m_height[c], out->m_minPelValue[c], out->m_maxPelValue[c]);
      }
    }
    else { // 16 bit data
      memcpy(out->m_ui16Comp[Y_COMP], inp->m_ui16Comp[Y_COMP], (int) out->m_compSize[Y_COMP] * sizeof(uint16));
      for (c = U_COMP; c <= V_COMP; c++) {
        filterInterlaced(out->m_ui16Comp[c], inp->m_ui16Comp[c], out->m_width[c], out->m_height[c], out->m_minPelValue[c], out->m_maxPelValue[c]);
      }
    }
    
  }
  else {
    if (out->m_isFloat == TRUE) {    // floating point data
      memcpy(out->m_floatComp[Y_COMP], inp->m_floatComp[Y_COMP], (int) out->m_compSize[Y_COMP] * sizeof(float));
      for (c = U_COMP; c <= V_COMP; c++) {
        filter(out->m_floatComp[c], inp->m_floatComp[c], out->m_width[c], out->m_height[c], (float) out->m_minPelValue[c], (float) out->m_maxPelValue[c] );
      }
    }
    else if (out->m_bitDepth == 8) {   // 8 bit data     
      memcpy(out->m_comp[Y_COMP], inp->m_comp[Y_COMP], (int) out->m_compSize[Y_COMP] * sizeof(imgpel));
      for (c = U_COMP; c <= V_COMP; c++) {
        filter(out->m_comp[c], inp->m_comp[c], out->m_width[c], out->m_height[c], out->m_minPelValue[c], out->m_maxPelValue[c]);
      }
    }
    else { // 16 bit data
      memcpy(out->m_ui16Comp[Y_COMP], inp->m_ui16Comp[Y_COMP], (int) out->m_compSize[Y_COMP] * sizeof(uint16));
      for (c = U_COMP; c <= V_COMP; c++) {
        filter(out->m_ui16Comp[c], inp->m_ui16Comp[c], out->m_width[c], out->m_height[c], out->m_minPelValue[c], out->m_maxPelValue[c]);
      }
    }
  }
}
} // namespace hdrtoolslib
  

//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
