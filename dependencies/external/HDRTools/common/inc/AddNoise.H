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
 * \file AddNoise.H
 *
 * \brief
 *    AddNoise class Header
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */


#ifndef __AddNoise_H__
#define __AddNoise_H__

#include "Global.H"
#include "Parameters.H"
#include "Frame.H"

namespace hdrtoolslib {

enum {
  NOISE_NULL   = 0,
  NOISE_NORMAL = 1,
  NOISE_AV1FG  = 2,
  NOISE_TOTAL
};

class AV1FGParams {
public:
  bool m_applyGrain;
  bool m_updateParameters;

  // 8 bit values
  int m_scalingPointsY[14][2];
  int m_numYPoints;  // value: 0..14

  // 8 bit values
  int m_scalingPointsCb[10][2];
  int m_numCbPoints;  // value: 0..10

  // 8 bit values
  int m_scalingPointsCr[10][2];
  int m_numCrPoints;  // value: 0..10

  int m_scalingShift;  // values : 8..11
  int m_arCoeffLag;  // values:  0..3

  // 8 bit values
  int m_arCoeffsY[24];
  int m_arCoeffsCb[25];
  int m_arCoeffsCr[25];
  
  int numPosLuma;

  // Shift value: AR coeffs range
  // 6: [-2, 2)
  // 7: [-1, 1)
  // 8: [-0.5, 0.5)
  // 9: [-0.25, 0.25)
  int m_arCoeffShift;  // values : 6..9
  int m_cbMult;        // 8 bits
  int m_cbLumaMult;    // 8 bits
  int m_cbOffset;      // 9 bits
  int m_crMult;        // 8 bits
  int m_crLumaMult;    // 8 bits
  int m_crOffset;      // 9 bits
  int m_overlapFlag;
  bool m_clipToRestrictedRange;
  int m_bitDepth;  // video bit depth
  int m_chromaScalingFromLuma; // bool?
  int m_grainScaleShift;
  uint16 m_randomSeed;
  
  AV1FGParams() {
    int i, j;
    m_applyGrain = FALSE;
    m_updateParameters = FALSE;

    // 8 bit values
    for (j = 0; j < 14; j++ )
      for (i = 0; i < 2; i++ )
        m_scalingPointsY[j][i] = 0;
    m_numYPoints = 0;  
    
    // 8 bit values
    for (j = 0; j < 10; j++ )
      for (i = 0; i < 2; i++ )
        m_scalingPointsCb[j][i] = 0;
    m_numCbPoints = 0;

    // 8 bit values
    for (j = 0; j < 10; j++ )
      for (i = 0; i < 2; i++ )
        m_scalingPointsCr[j][i] = 0;
    m_numCrPoints = 0;

    m_scalingShift = 8; 
    m_arCoeffLag = 0; 
    // 8 bit values
    for (j = 0; j < 24; j++ )
      m_arCoeffsY[j] = 0;
    for (j = 0; j < 25; j++ ) {
      m_arCoeffsCb[j] = 0;
      m_arCoeffsCr[j] = 0;
    }
    
    m_arCoeffShift = 6;  
    m_cbMult = 1;        
    m_cbLumaMult = 1;    
    m_cbOffset = 0;
    m_crMult = 1;        
    m_crLumaMult = 1;
    m_crOffset = 0;      
    m_overlapFlag = 0;
    m_clipToRestrictedRange = FALSE;
    m_bitDepth = 8;  
    m_chromaScalingFromLuma = 0;
    m_grainScaleShift = 0;
    m_randomSeed = 0;
  }
};

class NoiseParameters {
public:
  double  m_noiseVariance;
  double  m_noiseMean;
  AV1FGParams m_AV1FG; 
    
  NoiseParameters() {
    m_noiseVariance = 1.0;
    m_noiseMean = 0.0;
  }
};

class AddNoise {
public:
  // Construct/Deconstruct
  virtual ~AddNoise() {};
  static AddNoise *create(int method, double noiseVariance = 1.0, double noiseMean = 0.0, NoiseParameters *params = NULL);
  
  virtual void process(Frame *out,  Frame *inp) = 0;
  virtual void process(Frame *inp) = 0;
};
} // namespace hdrtoolslib

#endif
