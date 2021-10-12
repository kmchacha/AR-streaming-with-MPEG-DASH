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
 * \file ToneMappingRollCIE1931.cpp
 *
 * \brief
 *    ToneMappingRollCIE1931 Class
 *    This process performs the roll operation in CIE 1931
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
#include "ColorTransformGeneric.H"
#include "ToneMappingRollCIE1931.H"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

namespace hdrtoolslib {

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

ToneMappingRollCIE1931::ToneMappingRollCIE1931(double minValue, double maxValue, double targetValue, double gamma, bool scaleGammut) {
  m_scaleGammut = scaleGammut;
  m_minValue    = minValue;
  m_maxValue    = maxValue;
  
  m_transferFunction = TransferFunction::create(TF_PQ, TRUE, 1.0, 1.0, 0.0, 1.0, TRUE);
  m_maxIntensity = m_transferFunction->inverse(targetValue / 10000.0);
  m_KS = 1.5 * m_maxIntensity - 0.5;
  m_KSIntensity = m_transferFunction->forward(m_KS);
 
  m_targetValue = targetValue;
  m_gamma       = 1.0 / gamma;
  m_range       = maxValue - minValue;
  m_range       = m_transferFunction->inverse(maxValue / 10000.0);
  m_mappedRange = m_targetValue - minValue;
  m_scale       = m_maxIntensity / m_transferFunction->inverse(maxValue / 10000.0);
}

ToneMappingRollCIE1931::~ToneMappingRollCIE1931() {
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
void ToneMappingRollCIE1931::convertToXYZ(double *rgb, double *xyz, const double transform[3][3]) {
  // Should we be clipping here? TBD
  xyz[0] = dClip( transform[0][0] * rgb[R_COMP] + transform[0][1] * rgb[G_COMP] + transform[0][2] * rgb[B_COMP], 0, 1);
  xyz[1] = dClip( transform[1][0] * rgb[R_COMP] + transform[1][1] * rgb[G_COMP] + transform[1][2] * rgb[B_COMP], 0, 1);
  xyz[2] = dClip( transform[2][0] * rgb[R_COMP] + transform[2][1] * rgb[G_COMP] + transform[2][2] * rgb[B_COMP], 0, 1);
}

void ToneMappingRollCIE1931::convertXYZToxyY(double *xyz, double *xyY) {
  double sum = xyz[0] + xyz[1] + xyz[2];
  if ( sum == 0.0) {
    xyY[0] = 0.3127;
    xyY[1] = 0.3290;
    xyY[2] = 0.0000;
  }
  else {
    xyY[0] = xyz[0] / sum;
    xyY[1] = xyz[1] / sum;
    xyY[2] = xyz[1];
  }
}

void ToneMappingRollCIE1931::convertxyYToXYZ(double *xyY, double *xyz) {
  if ( xyY[2] == 0.0) {
    xyz[0] = 0.0000;
    xyz[1] = 0.0000;
    xyz[2] = 0.0000;
  }
  else {
    xyz[0] = xyY[2] * xyY[0] / xyY[1];
    xyz[1] = xyY[2];
    xyz[2] = xyY[2] * (1 - xyY[0] - xyY[1]) / xyY[1];
  }
}

void ToneMappingRollCIE1931::convertToRGB(double *xyz, double *rgb, const double transform[3][3]) {
  rgb[0] = dClip( transform[0][0] * xyz[0] + transform[0][1] * xyz[1] + transform[0][2] * xyz[2], 0, 1);
  rgb[1] = dClip( transform[1][0] * xyz[0] + transform[1][1] * xyz[1] + transform[1][2] * xyz[2], 0, 1);
  rgb[2] = dClip( transform[2][0] * xyz[0] + transform[2][1] * xyz[1] + transform[2][2] * xyz[2], 0, 1);
}


void ToneMappingRollCIE1931::setColorConversion(int colorPrimaries, double transformFW[3][3], double transformBW[3][3]) {
  int mode = CTF_IDENTITY;
  if (colorPrimaries == CP_709) {
    mode = CTF_RGB709_2_XYZ;
  }
  else if (colorPrimaries == CP_601) {
    mode = CTF_RGB601_2_XYZ;
  }
  else if (colorPrimaries == CP_2020) {
    mode = CTF_RGB2020_2_XYZ;
  }
  else if (colorPrimaries == CP_P3D65 || colorPrimaries == CP_P3D65N) {
    mode = CTF_RGBP3D65_2_XYZ;
  }
  
  for (int i = Y_COMP; i <= V_COMP; i++){
  for (int j = 0; j < 3; j++) {
    transformFW[i][j] = FWD_TRANSFORM[mode][i][j];
    transformBW[i][j] = INV_TRANSFORM[mode][i][j];
    }
  }
}


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void ToneMappingRollCIE1931::process ( Frame* frame) {

 // It is assumed here that the content are linear RGB data
 // We thus need to convert the data back to XYZ, the xyY, process the luminance,
 // and then convert back the data to their original space.
  if (frame->m_isFloat == TRUE ) {
    double transformFW[3][3];
    double transformBW[3][3];
    double rgbNormal[3], xyzNormal[3];
    double rgbOutNormal[3], xyzOutNormal[3];
    double xyYin[3], xyYout[3];
    double E, p;
    double colourScale;
    setColorConversion(frame->m_colorPrimaries, transformFW, transformBW);
    
    for (int i = 0; i < frame->m_compSize[Y_COMP]; i++) {
      rgbNormal[R_COMP] = frame->m_floatComp[R_COMP][i] / m_maxValue;
      rgbNormal[G_COMP] = frame->m_floatComp[G_COMP][i] / m_maxValue;
      rgbNormal[B_COMP] = frame->m_floatComp[B_COMP][i] / m_maxValue;
      
      convertToXYZ(rgbNormal, xyzNormal, transformFW);
      
      convertXYZToxyY(xyzNormal, xyYin);
      
      E = xyYin[2];
      
      p = pow(E / m_range, m_gamma) * m_maxIntensity;

      colourScale = m_scaleGammut == FALSE ? 1.0 : dMin( p / E, E / p);
      
      xyYout[0] = xyYin[0] * colourScale;
      xyYout[1] = xyYin[1] * colourScale;
      xyYout[2] = p;
      
      convertxyYToXYZ(xyYout, xyzOutNormal);
      convertToRGB(xyzOutNormal, rgbOutNormal, transformBW);
      
      frame->m_floatComp[R_COMP][i] = (float) ( rgbOutNormal[R_COMP] * m_maxValue );
      frame->m_floatComp[G_COMP][i] = (float) ( rgbOutNormal[G_COMP] * m_maxValue );
      frame->m_floatComp[B_COMP][i] = (float) ( rgbOutNormal[B_COMP] * m_maxValue );
    }
  }
}

void ToneMappingRollCIE1931::process ( Frame* out, const Frame *inp) {
  if (out == NULL) {
    // should only be done for pointers. This should help speeding up the code and reducing memory.
    out = (Frame *)inp;
  }
  else {
    if (inp->m_isFloat == TRUE && out->m_isFloat == TRUE && inp->m_size == out->m_size) {
      double transformFW[3][3];
      double transformBW[3][3];
      double rgbNormal[3], xyzNormal[3];
      double rgbOutNormal[3], xyzOutNormal[3];
      double xyYin[3], xyYout[3];
      double E, p;
      double colourScale;
      
      setColorConversion(inp->m_colorPrimaries, transformFW, transformBW);
      for (int i = 0; i < inp->m_compSize[Y_COMP]; i++) {
        rgbNormal[R_COMP] = inp->m_floatComp[R_COMP][i] / m_maxValue;
        rgbNormal[G_COMP] = inp->m_floatComp[G_COMP][i] / m_maxValue;
        rgbNormal[B_COMP] = inp->m_floatComp[B_COMP][i] / m_maxValue;
        
        convertToXYZ(rgbNormal, xyzNormal, transformFW);
        if (xyzNormal[2] >= m_KSIntensity) {
          
          convertXYZToxyY(xyzNormal, xyYin);

          E = xyYin[2];
          
          p = pow(E / m_range, m_gamma) * m_maxIntensity;

          colourScale = m_scaleGammut == FALSE ? 1.0 : dMin( p / E, E / p);
          
          xyYout[0] = xyYin[0] * colourScale;
          xyYout[1] = xyYin[1] * colourScale;
          xyYout[2] = p;
          
          convertxyYToXYZ(xyYout, xyzOutNormal);
          convertToRGB(xyzOutNormal, rgbOutNormal, transformBW);
          
          out->m_floatComp[R_COMP][i] = (float) ( rgbOutNormal[R_COMP] * m_maxValue );
          out->m_floatComp[G_COMP][i] = (float) ( rgbOutNormal[G_COMP] * m_maxValue );
          out->m_floatComp[B_COMP][i] = (float) ( rgbOutNormal[B_COMP] * m_maxValue );
        }
        else {
          out->m_floatComp[R_COMP][i] = inp->m_floatComp[R_COMP][i];
          out->m_floatComp[G_COMP][i] = inp->m_floatComp[G_COMP][i];
          out->m_floatComp[B_COMP][i] = inp->m_floatComp[B_COMP][i];
        }
      }
    }
    else if (inp->m_isFloat == FALSE && out->m_isFloat == FALSE && inp->m_size == out->m_size && inp->m_bitDepth == out->m_bitDepth) {
      out->copy((Frame *) inp);
    }
  }
}
} // namespace hdrtoolslib


//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
