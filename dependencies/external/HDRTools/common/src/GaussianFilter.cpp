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
 * \file GaussianFilter
 *
 * \brief
 *    Gaussian filtering
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
#include "GaussianFilter.H"
#include <math.h>

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------
#define TEST_FILTER 0

namespace hdrtoolslib {

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------
GaussianFilter::GaussianFilter(int sizeX, int sizeY, double radius, bool verbose) {
  double  dSizeX   = ((double) sizeX - 1.0) / 2.0;
  double  dSizeY   = ((double) sizeY - 1.0) / 2.0;
    
  double  std      = 2.0 * radius * radius;
  double  x,y;  
  double  expPower, h , sumh = 0.0;
  int     i , j;

  m_sizeX = sizeX;
  m_sizeY = sizeY;
  m_midX = (int) (dSizeX + 0.5);
  m_midY = (int) (dSizeY + 0.5);

  m_filter.resize(sizeY);
  for (i = 0; i < sizeY; i++) {
    m_filter[i].resize(sizeX);
  }
  
  for (y = -dSizeY, j = 0; y <= dSizeY; y += 1.0, j++){
    for (x = -dSizeX, i = 0; x <= dSizeX; x += 1.0, i++){
      expPower = -(x * x + y * y) / std;
      h     = exp(expPower);
      m_filter[j][i] = h;      
      sumh += h;
    }
  }
  if (sumh > 0.0) {
    h  = h/sumh;
    for ( i = 0; i < sizeX; i ++ ){
      for ( j = 0; j < sizeY; j ++ ){
        m_filter[j][i] /= sumh;
      }
    }
  }

  if (verbose) {
    printf("Gaussian Filter used for filtering.\n");
    printf("===================================\n");
    printf("size (%d x %d) radius%5.6f \n", sizeX, sizeY, radius);
    for ( i = 0; i < sizeX; i ++ ){
      for ( j = 0; j < sizeY; j ++ ){
       printf("%10.8f ", m_filter[j][i]);
      }
      printf("\n");
    }
    printf("===================================\n");
  }  
}

GaussianFilter::~GaussianFilter() {
}


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------
double GaussianFilter::filter(double *inp, int posY, int posX, int width, int height){
  int i, j;
  int clipY, clipX;
  double value = 0.0;

  for (j = 0; j < m_sizeY; j++) {
    clipY = iClip(posY + j - m_midY, 0, height - 1) * width;
    for (i = 0; i < m_sizeX; i++) {
      clipX = iClip(posX + i - m_midX, 0, width - 1);
      value += (double) m_filter[j][i] * (double) inp[clipY + clipX];
    }
  }
  return value;
}

double GaussianFilter::filter(float *inp, int posY, int posX, int width, int height){
  int i, j;
  int clipY, clipX;
  double value = 0.0;

  for (j = 0; j < m_sizeY; j++) {
    clipY = iClip(posY + j - m_midY, 0, height - 1) * width;
    for (i = 0; i < m_sizeX; i++) {
      clipX = iClip(posX + i - m_midX, 0, width - 1);
      value += (double) m_filter[j][i] * (double) inp[clipY + clipX];
    }
  }
  return value;
}

double GaussianFilter::filter(imgpel *inp, int posY, int posX, int width, int height){
  int i, j;
  int clipY, clipX;
  double value = 0.0;

  for (j = 0; j < m_sizeY; j++) {
    clipY = iClip(posY + j - m_midY, 0, height - 1) * width;
    for (i = 0; i < m_sizeX; i++) {
      clipX = iClip(posX + i - m_midX, 0, width - 1);
      value += (double) m_filter[j][i] * (double) inp[clipY + clipX];
    }
  }
  return value;
}


double GaussianFilter::filter(uint16 *inp, int posY, int posX, int width, int height){
  int i, j;
  int clipY, clipX;
  double value = 0.0;

  for (j = 0; j < m_sizeY; j++) {
    clipY = iClip(posY + j - m_midY, 0, height - 1) * width;
    for (i = 0; i < m_sizeX; i++) {
      clipX = iClip(posX + i - m_midX, 0, width - 1);
      value += (double) m_filter[j][i] * (double) inp[clipY + clipX];
    }
  }
  return value;
}

void GaussianFilter::compute(double *inp, double *out, int width, int height) {
  int i, j;

  for (j = 0; j < height; j++) {  
    for (i = 0; i < width; i++) {
      out[j * width + i] = filter(inp, j, i, width, height);
    }
  }  
}

void GaussianFilter::compute(float *inp, double *out, int width, int height) {
  int i, j;

  for (j = 0; j < height; j++) {  
    for (i = 0; i < width; i++) {
      out[j * width + i] = filter(inp, j, i, width, height);
    }
  }  
}

void GaussianFilter::compute(imgpel *inp, double *out, int width, int height) {
  int i, j;

  for (j = 0; j < height; j++) {  
    for (i = 0; i < width; i++) {
      out[j * width + i] = filter(inp, j, i, width, height);
    }
  }  
}

void GaussianFilter::compute(uint16 *inp, double *out, int width, int height) {
  int i, j;

  for (j = 0; j < height; j++) {  
    for (i = 0; i < width; i++) {
      out[j * width + i] = filter(inp, j, i, width, height);
    }
  }  
}

} // namespace hdrtoolslib

//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
