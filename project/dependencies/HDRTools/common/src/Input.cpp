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
 * \file Input.cpp
 *
 * \brief
 *    Input Class relating to I/O operations
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */


//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "Input.H"
#include "InputDPX.H"
#include "InputEXR.H"
#include "InputTIFF.H"
#include "InputY4M.H"
#include "InputYUV.H"
#include "InputPFM.H"
#include "InputPPM.H"
#include "InputPGM.H"
#include "InputPNG.H"
#include "Global.H"

#include <stdlib.h>
#include <string.h>
//#include <math.h>
#include <cmath>

namespace hdrtoolslib {
  
//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
/*!
 ************************************************************************
 * \brief
 *   Get an unsigned short without swapping.
 *
 ************************************************************************
 */
static uint16 getU16 (const char *& buf)
{
  uint16 in;
  
  *(((char *) &in) + 0) = *buf++;
  *(((char *) &in) + 1) = *buf++;
  
  return in;
}

/*!
 ************************************************************************
 * \brief
 *   Get an unsigned short and swap.
 *
 ************************************************************************
 */
static uint16 getSwappedU16 (const char *& buf)
{
  uint16 in;
  
  *(((char *) &in) + 1) = *buf++;
  *(((char *) &in) + 0) = *buf++;
  
  return in;
}


void Input::deInterleaveYUV420( 
  uint8** input,         //!< input buffer
  uint8** output,        //!< output buffer
  FrameFormat *source,   //!< format of source buffer
  int symbolSizeInBytes  //!< number of bytes per symbol
  ) 
{
  int i;  
  // original buffer
  uint8 *icmp  = *input;
  // final buffer
  uint8 *ocmp0 = *output;

  uint8 *ocmp1 = ocmp0 + symbolSizeInBytes * source->m_compSize[Y_COMP];
  uint8 *ocmp2 = ocmp1 + symbolSizeInBytes * source->m_compSize[U_COMP];

  for (i = 0; i < source->m_compSize[U_COMP]; i++) {
    memcpy(ocmp1, icmp, symbolSizeInBytes);
    ocmp1 += symbolSizeInBytes;
    icmp  += symbolSizeInBytes;
    memcpy(ocmp0, icmp, 2 * symbolSizeInBytes);
    ocmp0 += 2 * symbolSizeInBytes;
    icmp  += 2 * symbolSizeInBytes;
    memcpy(ocmp2, icmp, symbolSizeInBytes);
    ocmp2 += symbolSizeInBytes;
    icmp  += symbolSizeInBytes;
    memcpy(ocmp0, icmp, 2 * symbolSizeInBytes);
    ocmp0 += 2 * symbolSizeInBytes;
    icmp  += 2 * symbolSizeInBytes;
  }

  // flip buffers
  icmp    = *input;
  *input  = *output;
  *output = icmp;  
}

// Deinterleave the NV12/YCM format
void Input::deInterleaveNV12(
  uint8** input,         //!< input buffer
  uint8** output,        //!< output buffer
  FrameFormat *source,   //!< format of source buffer
  int symbolSizeInBytes  //!< number of bytes per symbol
  )
{
  int i;
  // original buffer
  uint8 *icmp  = *input;
  // final buffer
  uint8 *ocmp0 = *output;

  uint8 *ocmp1 = ocmp0 + symbolSizeInBytes * source->m_compSize[Y_COMP];
  uint8 *ocmp2 = ocmp1 + symbolSizeInBytes * source->m_compSize[U_COMP];
  for (i = 0; i < source->m_compSize[Y_COMP]; i++) {
    memcpy(ocmp0, icmp, symbolSizeInBytes);
    ocmp0 += symbolSizeInBytes;
    icmp  += symbolSizeInBytes;
  }

  for (i = 0; i < source->m_compSize[U_COMP]; i++) {
    memcpy(ocmp1, icmp, symbolSizeInBytes);
    ocmp1 += symbolSizeInBytes;
    icmp  += symbolSizeInBytes;
    memcpy(ocmp2, icmp, symbolSizeInBytes);
    ocmp2 += symbolSizeInBytes;
    icmp  += symbolSizeInBytes;
  }

  // flip buffers
  icmp    = *input;
  *input  = *output;
  *output = icmp;
}

// Deinterleave the HHI Packed format
void Input::deInterleavePCK(
                            uint8** input,         //!< input buffer
                            uint8** output,        //!< output buffer
                            FrameFormat *source,   //!< format of source buffer
                            int symbolSizeInBytes  //!< number of bytes per symbol
)
{
  int i, cmp;
  // original buffer
  uint8 *icmp  = *input;
  
  uint8  *ui8cmp  = (uint8 *) *input;
  uint16 *ui16cmp = (uint16 *) *output;
  
  if (source->m_bitDepthComp[0] == 10) {
    for (cmp = 0; cmp < (source->m_chromaFormat == CF_400 ? 1 : 3); cmp++) {
      for (i = 0; i < source->m_compSize[cmp]; i+= 4) {
        // Byte 0             Byte 1              Byte 2             Byte 3             Byte 5
        // 7 6 5 4 3 2 1 0    5 4 3 2 1 0 9 8     3 2 1 0 9 8 7 6    1 0 9 8 7 6 5 4    9 8 7 6 5 4 3 2

        *ui16cmp++ = (  *ui8cmp           ) + (((*(ui8cmp + 1)) & 0x03) << 8);            // Y 0
        *ui16cmp++ = ((*(ui8cmp + 1)) >> 2) + (((*(ui8cmp + 2)) & 0x0F) << 6);            // Y 1
        *ui16cmp++ = ((*(ui8cmp + 2)) >> 4) + (((*(ui8cmp + 3)) & 0x3F) << 4);            // Y 2
        *ui16cmp++ = ((*(ui8cmp + 3)) >> 6) + (((*(ui8cmp + 4))       ) << 2);            // Y 3
        
        ui8cmp += 5;
      }
    }
  }
  else if (source->m_bitDepthComp[0] == 12) {
    for (cmp = 0; cmp < (source->m_chromaFormat == CF_400 ? 1 : 3); cmp++) {
      for (i = 0; i < source->m_compSize[cmp]; i+= 2) {
        // Byte 0             Byte 1              Byte 2
        // 7 6 5 4 3 2 1 0    3 2 1 0 B A 9 8     B A 9 8 7 6 5 4
        
        *ui16cmp++ = ( *(ui8cmp    )      ) + (((*(ui8cmp + 1)) & 0x0F) << 8);            // Y 0
        *ui16cmp++ = ((*(ui8cmp + 1)) >> 4) + (((*(ui8cmp + 2))       ) << 4);            // Y 1
        
        ui8cmp += 3;
      }
    }
  }
  // flip buffers
  icmp    = *input;
  *input  = *output;
  *output = icmp;
}


void Input::deInterleaveYUV444(
  uint8** input,       //!< input buffer
  uint8** output,      //!< output buffer
  FrameFormat *source,         //!< format of source buffer
  int symbolSizeInBytes     //!< number of bytes per symbol
  ) 
{
  int i;  
  // original buffer
  uint8 *icmp  = *input;
  // final buffer
  uint8 *ocmp0 = *output;

  uint8 *ocmp1 = ocmp0 + symbolSizeInBytes * source->m_compSize[Y_COMP];
  uint8 *ocmp2 = ocmp1 + symbolSizeInBytes * source->m_compSize[U_COMP];

  for (i = 0; i < source->m_compSize[Y_COMP]; i++) {
    memcpy(ocmp0, icmp, symbolSizeInBytes);
    ocmp0 += symbolSizeInBytes;
    icmp  += symbolSizeInBytes;
    memcpy(ocmp1, icmp, symbolSizeInBytes);
    ocmp1 += symbolSizeInBytes;
    icmp  += symbolSizeInBytes;
    memcpy(ocmp2, icmp, symbolSizeInBytes);
    ocmp2 += symbolSizeInBytes;
    icmp  += symbolSizeInBytes;
  }
  // flip buffers
  icmp    = *input;
  *input  = *output;
  *output = icmp;
}

void Input::deInterleaveYUYV (
  uint8** input,               //!< input buffer
  uint8** output,              //!< output buffer
  FrameFormat *source,         //!< format of source buffer
  int symbolSizeInBytes        //!< number of bytes per symbol
  ) 
{
  int i;  
  // original buffer
  uint8 *icmp = *input;
  // final buffer
  uint8 *ocmp0 = *output;

  uint8 *ocmp1 = ocmp0 + symbolSizeInBytes * source->m_compSize[Y_COMP];
  uint8 *ocmp2 = ocmp1 + symbolSizeInBytes * source->m_compSize[U_COMP];

  for (i = 0; i < source->m_compSize[U_COMP]; i++) {
    // Y
    memcpy(ocmp0, icmp, symbolSizeInBytes);
    ocmp0 += symbolSizeInBytes;
    icmp  += symbolSizeInBytes;
    // U
    memcpy(ocmp1, icmp, symbolSizeInBytes);
    ocmp1 += symbolSizeInBytes;
    icmp  += symbolSizeInBytes;
    // Y
    memcpy(ocmp0, icmp, symbolSizeInBytes);
    ocmp0 += symbolSizeInBytes;
    icmp  += symbolSizeInBytes;
    // V
    memcpy(ocmp2, icmp, symbolSizeInBytes);
    ocmp2 += symbolSizeInBytes;
    icmp  += symbolSizeInBytes;
  }
  // flip buffers
  icmp    = *input;
  *input  = *output;
  *output = icmp;
}

void Input::deInterleaveYVYU (
  uint8** input,         //!< input buffer
  uint8** output,        //!< output buffer
  FrameFormat *source,   //!< format of source buffer
  int symbolSizeInBytes  //!< number of bytes per symbol
  ) 
{
  int i;  
  // original buffer
  uint8 *icmp  = *input;
  // final buffer
  uint8 *ocmp0 = *output;

  uint8 *ocmp1 = ocmp0 + symbolSizeInBytes * source->m_compSize[Y_COMP];
  uint8 *ocmp2 = ocmp1 + symbolSizeInBytes * source->m_compSize[U_COMP];

  for (i = 0; i < source->m_compSize[U_COMP]; i++) {
    // Y
    memcpy(ocmp0, icmp, symbolSizeInBytes);
    ocmp0 += symbolSizeInBytes;
    icmp  += symbolSizeInBytes;
    // V
    memcpy(ocmp2, icmp, symbolSizeInBytes);
    ocmp2 += symbolSizeInBytes;
    icmp  += symbolSizeInBytes;
    // Y
    memcpy(ocmp0, icmp, symbolSizeInBytes);
    ocmp0 += symbolSizeInBytes;
    icmp  += symbolSizeInBytes;
    // U
    memcpy(ocmp1, icmp, symbolSizeInBytes);
    ocmp1 += symbolSizeInBytes;
    icmp  += symbolSizeInBytes;
  }
  // flip buffers
  icmp    = *input;
  *input  = *output;
  *output = icmp;
}

void Input::deInterleaveUYVY (
  uint8** input,         //!< input buffer
  uint8** output,        //!< output buffer
  FrameFormat *source,   //!< format of source buffer
  int symbolSizeInBytes  //!< number of bytes per symbol
  ) 
{
  int i;  
  // original buffer
  uint8 *icmp  = *input;
  // final buffer
  uint8 *ocmp0 = *output;

  uint8 *ocmp1 = ocmp0 + symbolSizeInBytes * source->m_compSize[Y_COMP];
  uint8 *ocmp2 = ocmp1 + symbolSizeInBytes * source->m_compSize[U_COMP];

  for (i = 0; i < source->m_compSize[U_COMP]; i++) {
    // U
    memcpy(ocmp1, icmp, symbolSizeInBytes);
    ocmp1 += symbolSizeInBytes;
    icmp  += symbolSizeInBytes;
    // Y
    memcpy(ocmp0, icmp, symbolSizeInBytes);
    ocmp0 += symbolSizeInBytes;
    icmp  += symbolSizeInBytes;
    // V
    memcpy(ocmp2, icmp, symbolSizeInBytes);
    ocmp2 += symbolSizeInBytes;
    icmp  += symbolSizeInBytes;
    // Y
    memcpy(ocmp0, icmp, symbolSizeInBytes);
    ocmp0 += symbolSizeInBytes;
    icmp  += symbolSizeInBytes;
  }
  // flip buffers
  icmp    = *input;
  *input  = *output;
  *output = icmp;
}

void Input::deInterleaveV210 (
  uint8** input,            //!< input buffer
  uint8** output,           //!< output buffer
  FrameFormat *source,      //!< format of source buffer
  int symbol_size_in_bytes  //!< number of bytes per symbol
)
{
  int i;
  // original buffer
  uint8 *icmp  = NULL;
  
  uint32  *ui32cmp = (uint32 *) *input;
  uint16 *ui16cmp0 = (uint16 *) *output;
  uint16 *ui16cmp1 = ui16cmp0 + source->m_compSize[Y_COMP];
  uint16 *ui16cmp2 = ui16cmp1 + source->m_compSize[U_COMP];
  
  for (i = 0; i < source->m_compSize[Y_COMP]; i+= 6) {
    // Byte 3          Byte 2          Byte 1          Byte 0
    // Cr 0                Y 0                 Cb 0
    // X X 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
    *ui16cmp2   = (*ui32cmp & 0x3FF00000)>>20;         // Cr 0
    *ui16cmp0   = (*ui32cmp & 0xffc00)>>10;            // Y 0
    *ui16cmp1   = (*(ui32cmp++) & 0x3FF);              // Cb 0
    
    // Byte 7          Byte 6          Byte 5          Byte 4
    // Y 2                 Cb 1                Y 1
    // X X 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
    *(ui16cmp0 + 2) = (*ui32cmp & 0x3FF00000)>>20;     // Y 2
    *(ui16cmp1 + 1) = (*ui32cmp & 0xffc00)>>10;        // Cb 1
    *(ui16cmp0 + 1) = (*(ui32cmp++) & 0x3FF);          // Y 1
    
    // Byte 11         Byte 10         Byte 9          Byte 8
    // Cb 2                Y 3                 Cr 1
    // X X 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
    *(ui16cmp1 + 2) = (*ui32cmp & 0x3FF00000)>>20;     // Cb 2
    *(ui16cmp0 + 3) = (*ui32cmp & 0xffc00)>>10;        // Y 3
    *(ui16cmp2 + 1) = (*(ui32cmp++) & 0x3FF);          // Cr 1
    
    // Byte 15         Byte 14         Byte 13         Byte 12
    // Y 5                 Cr 2                Y 4
    // X X 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
    *(ui16cmp0 + 5) = (*ui32cmp & 0x3FF00000)>>20;     // Y 2
    *(ui16cmp2 + 2) = (*ui32cmp & 0xffc00)>>10;        // Cb 1
    *(ui16cmp0 + 4) = (*(ui32cmp++) & 0x3FF);          // Y 1
    
    ui16cmp0 += 6;
    ui16cmp1 += 3;
    ui16cmp2 += 3;
  }
  // flip buffers
  icmp    = *input;
  *input  = *output;
  *output = icmp;
}

void Input::deInterleaveV410 (
                              uint8** input,            //!< input buffer
                              uint8** output,           //!< output buffer
                              FrameFormat *source,      //!< format of source buffer
                              int symbol_size_in_bytes  //!< number of bytes per symbol
)
{
  int i;
  // original buffer
  uint8 *icmp  = NULL;
  
  uint32  *ui32cmp = (uint32 *) *input;
  uint16 *ui16cmp0 = (uint16 *) *output;
  uint16 *ui16cmp1 = ui16cmp0 + source->m_compSize[Y_COMP];
  uint16 *ui16cmp2 = ui16cmp1 + source->m_compSize[U_COMP];
  
  for (i = 0; i < source->m_compSize[Y_COMP]; i++) {
    // Byte 3          Byte 2          Byte 1          Byte 0
    // Cr                  Y                  Cb 
    // 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 X X
    *ui16cmp2++   = (*ui32cmp & 0xFFC00000) >> 22;         // Cr 
    *ui16cmp0++   = (*ui32cmp & 0x003FF000) >> 12;         // Y 
    *ui16cmp1++   = (*ui32cmp & 0x00000FFC) >>  2;         // Cb 
    ui32cmp++;
  }
  
  // flip buffers
  icmp    = *input;
  *input  = *output;
  *output = icmp;
}

void Input::deInterleaveR10K (
                              uint8** input,            //!< input buffer
                              uint8** output,           //!< output buffer
                              FrameFormat *source,      //!< format of source buffer
                              int symbol_size_in_bytes  //!< number of bytes per symbol
)
{
  int i;
  // original buffer
  uint8 *icmp  = NULL;
  
  uint32  *ui32cmp = (uint32 *) *input;
  uint16 *ui16cmp0 = (uint16 *) *output;
  uint16 *ui16cmp1 = ui16cmp0 + source->m_compSize[Y_COMP];
  uint16 *ui16cmp2 = ui16cmp1 + source->m_compSize[U_COMP];
  
  for (i = 0; i < source->m_compSize[Y_COMP]; i++) {
    // Byte 3          Byte 2          Byte 1          Byte 0
    // R                   G                   B 
    // 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 X X
    *ui16cmp1++   = (*ui32cmp & 0x3FF) ;            // R 
    *ui16cmp2++   = ((*ui32cmp >> 10) & 0x3FF);    // G 
    *ui16cmp0++   = ((*ui32cmp >> 20) & 0x3FF);    // B 
    ui32cmp++;
  }
  
  // flip buffers
  icmp    = *input;
  *input  = *output;
  *output = icmp;
}

void Input::deInterleaveB64A (
                              uint8** input,            //!< input buffer
                              uint8** output,           //!< output buffer
                              FrameFormat *source,      //!< format of source buffer
                              int symbol_size_in_bytes  //!< number of bytes per symbol
)
{
  int i;
  // original buffer
  uint8 *icmp  = NULL;

  const char  *ui8cmp = (const char *) *input;
  //uint16  *ui16cmp = (uint16 *) *input;
  uint16 *ui16cmp0 = (uint16 *) *output;
  uint16 *ui16cmp1 = ui16cmp0 + source->m_compSize[Y_COMP];
  uint16 *ui16cmp2 = ui16cmp1 + source->m_compSize[U_COMP];
  
  for (i = 0; i < source->m_compSize[Y_COMP]; i++) {
    ui8cmp+=2;
    *ui16cmp1++ = mGetU16(ui8cmp);
    *ui16cmp0++ = mGetU16(ui8cmp);
    *ui16cmp2++ = mGetU16(ui8cmp);
  }
  
  // flip buffers
  icmp    = *input;
  *input  = *output;
  *output = icmp;
}

void Input::deInterleaveR210 (
                              uint8** input,            //!< input buffer
                              uint8** output,           //!< output buffer
                              FrameFormat *source,      //!< format of source buffer
                              int symbol_size_in_bytes  //!< number of bytes per symbol
)
{
  int i;
  // original buffer
  uint8 *icmp  = NULL;
  
  uint32  *ui32cmp = (uint32 *) *input;
  uint16 *ui16cmp0 = (uint16 *) *output;
  uint16 *ui16cmp1 = ui16cmp0 + source->m_compSize[Y_COMP];
  uint16 *ui16cmp2 = ui16cmp1 + source->m_compSize[U_COMP];

  for (i = 0; i < source->m_compSize[Y_COMP]; i++) {
    // Byte 3          Byte 2          Byte 1          Byte 0
    // Blo             Glo         Bhi Rlo     Ghi     X X Rhi
    // 7 6 5 4 3 2 1 0 5 4 3 2 1 0 9 8 3 2 1 0 9 8 7 6 x x 9 8 7 6 5 4
    *ui16cmp0++ = ((*ui32cmp & 0xFF000000) >> 24) & ((*ui32cmp & 0x00030000)<< 8);
    *ui16cmp1++ = ((*ui32cmp & 0x00FC0000) >> 18) & ((*ui32cmp & 0x00000F00) << 4);
    *ui16cmp2++ = ((*ui32cmp & 0x0000F000) >> 12) & ((*ui32cmp & 0x0000003F) << 4);

      ui32cmp++;
  }
  
  // flip buffers
  icmp    = *input;
  *input  = *output;
  *output = icmp;
}

void Input::deInterleaveUYVY10 (
  uint8** input,         //!< input buffer
  uint8** output,        //!< output buffer
  FrameFormat *source,   //!< format of source buffer
  int symbolSizeInBytes  //!< number of bytes per symbol
  )
{
    int i;
    // original buffer
    uint8 *icmp  = *input;
    // final buffer
    uint8 *ocmp0 = *output;
  
    uint8 *ocmp1 = ocmp0 + symbolSizeInBytes * source->m_compSize[Y_COMP];
    uint8 *ocmp2 = ocmp1 + symbolSizeInBytes * source->m_compSize[U_COMP];
  
    for (i = 0; i < source->m_compSize[Y_COMP]; i+=6) {
        // Read four 32-bit words containing 12 10-bit packed samples, fix endianness
        uint32 word0 = icmp[3]  | (icmp[2]  << 8) | (icmp[1]  << 16) | (icmp[0]  << 24);
        uint32 word1 = icmp[7]  | (icmp[6]  << 8) | (icmp[5]  << 16) | (icmp[4]  << 24);
        uint32 word2 = icmp[11] | (icmp[10] << 8) | (icmp[9]  << 16) | (icmp[8] << 24);
        uint32 word3 = icmp[15] | (icmp[14] << 8) | (icmp[13] << 16) | (icmp[12] << 24);
        uint16 y[6];
        uint16 u[3];
        uint16 v[3];
    
        // Unpack the samples
        u[0] =  word0 >> 22;
        y[0] = (word0 >> 12) & 0x3ff;
        v[0] = (word0 >>  2) & 0x3ff;
        y[1] =  word1 >> 22;
        u[1] = (word1 >> 12) & 0x3ff;
        y[2] = (word1 >>  2) & 0x3ff;
        v[1] =  word2 >> 22;
        y[3] = (word2 >> 12) & 0x3ff;
        u[2] = (word2 >>  2) & 0x3ff;
        y[4] =  word3 >> 22;
        v[2] = (word3 >> 12) & 0x3ff;
        y[5] = (word3 >>  2) & 0x3ff;
    
        // Y
        memcpy(ocmp0, y, 6 * symbolSizeInBytes);
        ocmp0 += (6 * symbolSizeInBytes);
        // U
        memcpy(ocmp1, u, 3 * symbolSizeInBytes);
        ocmp1 += (3 * symbolSizeInBytes);
        // V
        memcpy(ocmp2, v, 3 * symbolSizeInBytes);
        ocmp2 += (3 * symbolSizeInBytes);
    
       icmp += 16;
      }
    // flip buffers
    icmp    = *input;
    *input  = *output;
    *output = icmp;
}


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------
Input *Input::create(IOVideo *inputFile, FrameFormat *format, Parameters *inputParams) {
  Input *result = NULL;
  FrameFormat *source = &inputFile->m_format;

  switch (inputFile->m_videoType) {
    case VIDEO_YUV:
    case VIDEO_RGB:
      //if (inputFile->m_isConcatenated == TRUE)
        result = new InputYUV(inputFile, format);
      //else {
        //fprintf(stderr, "Only concatenated file types currently supported\n");
        //exit(EXIT_FAILURE);
      //}
      break;
    case VIDEO_DPX:
      result = new InputDPX(inputFile, format);
      break;
    case VIDEO_EXR:
      result = new InputEXR(inputFile, format);
      break;
    case VIDEO_TIFF:
      result = new InputTIFF(inputFile, format);
      break;
    case VIDEO_Y4M:
      result = new InputY4M(inputFile, format);
      break;
    case VIDEO_PFM:
      result = new InputPFM(inputFile, format);
      break;
    case VIDEO_PPM:
      result = new InputPPM(inputFile, format);
      break;
    case VIDEO_PGM:
      result = new InputPGM(inputFile, format);
      break;
#ifdef LIBPNG
    case VIDEO_PNG:
      result = new InputPNG(inputFile, format);
      break;
#endif
    default:
      fprintf(stderr, "Not supported format type %d\n", inputFile->m_videoType);
      exit(EXIT_FAILURE);
      break;
  }
  
  // I/O buffer to image mapping functions
  result->m_bufToImg = BufToImg::create(source);

  return result;
}

Input::Input () {
  m_isFloat           = FALSE;
  m_isInterleaved     = FALSE;
  m_isInterlaced      = FALSE;
  m_size              = 0;
  m_compSize[0]       = m_compSize[1] = m_compSize[2] = m_compSize[3] = 0;
  m_height[0]         = m_height[1] = m_height[2] = m_height[3] = 0;
  m_width[0]          = m_width[1] = m_width[2] = m_width[3] = 0;
  m_colorSpace        = CM_YCbCr;
  m_colorPrimaries    = CP_709;
  m_sampleRange       = SR_STANDARD;
  m_chromaFormat      = CF_420;
  m_chromaLocation[FP_TOP] = m_chromaLocation[FP_BOTTOM] = CL_ZERO;
  m_transferFunction  = TF_NULL;
  m_pixelFormat       = PF_UNKNOWN;

  m_bitDepthComp[0]   = m_bitDepthComp[1] = m_bitDepthComp[2] = m_bitDepthComp[3] = 8;
  m_pixelType[0]      = m_pixelType[1]    = m_pixelType[2]    = m_pixelType[3] = UINT;
  m_frameRate         = 24.0f;
  m_frameRateDenom    = 1;
  m_frameRateNum      = 24;  
  m_picUnitSizeOnDisk = 8;
  m_picUnitSizeShift3 = m_picUnitSizeOnDisk >> 3;
  m_bufToImg          = NULL;
  m_comp[0]           = m_comp[1]      = m_comp[2]      = m_comp[3]      = NULL;
  m_ui16Comp[0]       = m_ui16Comp[1]  = m_ui16Comp[2]  = m_ui16Comp[3]  = NULL;
  m_floatComp[0]      = m_floatComp[1] = m_floatComp[2] = m_floatComp[3] = NULL;
  
  // Check endianess
  int endian = 1;
  int machineLittleEndian = (*( (char *)(&endian) ) == 1) ? 1 : 0;
  if (machineLittleEndian == 0)  { // endianness of machine matches b64a files
    mGetU16 = getU16;
  }
  else {                           // endianness of machine does not match b64a file
    mGetU16 = getSwappedU16;
  }
}


Input::~Input() {
  if(m_bufToImg != NULL) {
    delete m_bufToImg;
    m_bufToImg = NULL;
  }
}

void Input::clear() {
}

// copy data from input to frm target
void Input::copyFrame(Frame *frm) {
  // we currently only copy the first three components and discard alpha
  //int size = m_compSize[ZERO] + m_compSize[ONE] + m_compSize[TWO];
  if (m_isFloat) {
    // Copying floating point data from the input buffer to the frame buffer
    frm->m_floatData = m_floatData;
  }
  else if (frm->m_bitDepth == m_bitDepthComp[Y_COMP]) {
    if (m_chromaFormat == frm->m_chromaFormat) {
      // if same chroma type, simply copy the frame
      if (frm->m_bitDepth == 8)
        frm->m_data = m_data;
      else
        frm->m_ui16Data = m_ui16Data;
    }
    else {
      // This should never happen. To revisit.
      // otherwise, most likely a 422 to 420 conversion (note that currently not checking for 420 to 422; Should fix)
      if (frm->m_bitDepth == 8) {
        memcpy(frm->m_comp[Y_COMP], m_comp[Y_COMP], (int) m_compSize[Y_COMP] * sizeof(imgpel));
      }
      else {
        memcpy(frm->m_ui16Comp[Y_COMP], m_ui16Comp[Y_COMP], (int) m_compSize[Y_COMP] * sizeof(imgpel));
      }
    }
  }
  else {
    // Different bitdepth
    fprintf(stderr, "Different bitdepth (%d <> %d).\n", frm->m_bitDepth, m_bitDepthComp[Y_COMP]);
    // Currently not supported
    // This should never happen. To revisit.
  }
}


/*!
************************************************************************
* \brief
*    Deinterleave file read buffer to source picture structure
************************************************************************
*/
void Input::deInterleave ( uint8** input,       //!< input buffer
                           uint8** output,      //!< output buffer
                           FrameFormat *format,         //!< format of source buffer
                           int symbolSizeInBytes     //!< number of bytes per symbol
                         ) 
{
  if (format->m_pixelFormat == PF_PCK) {
    deInterleavePCK(input, output, format, symbolSizeInBytes);
  }
  else {
    switch (format->m_chromaFormat) {
      case CF_420:
        if (format->m_pixelFormat == PF_YCM)
          deInterleaveNV12  (input, output, format, symbolSizeInBytes);
        else
          deInterleaveYUV420(input, output, format, symbolSizeInBytes);
        break;
      case CF_422:
        switch (format->m_pixelFormat) {
          case PF_YUYV:
          case PF_YUY2:
            deInterleaveYUYV(input, output, format, symbolSizeInBytes);
            break;
          case PF_YVYU:
            deInterleaveYVYU(input, output, format, symbolSizeInBytes);
            break;
          case PF_UYVY:
            deInterleaveUYVY(input, output, format, symbolSizeInBytes);
            break;
          case PF_V210:
            deInterleaveV210(input, output, format, symbolSizeInBytes);
            break;
          case PF_UYVY10:
            deInterleaveUYVY10(input, output, format, symbolSizeInBytes);
            break;
          default:
            fprintf(stderr, "Unsupported pixel format (%d).\n", format->m_pixelFormat);
            exit(EXIT_FAILURE);
            break;
        }
        break;
      case CF_444:
        switch (format->m_pixelFormat) {
          case PF_V410:
            deInterleaveV410(input, output, format, symbolSizeInBytes);
            break;
          case PF_R10K:
            deInterleaveR10K(input, output, format, symbolSizeInBytes);
            break;
          case PF_R210:
            deInterleaveR210(input, output, format, symbolSizeInBytes);
            break;
          case PF_R12B:
            deInterleaveR210(input, output, format, symbolSizeInBytes);
            break;
          case PF_R12L:
            deInterleaveR210(input, output, format, symbolSizeInBytes);
            break;
          case PF_B64A:
            deInterleaveB64A(input, output, format, symbolSizeInBytes);
            break;
          default:
            deInterleaveYUV444(input, output, format, symbolSizeInBytes);
            break;
        }
        break;
      default:
        fprintf(stderr, "Unknown Chroma Format type (%d)\n", format->m_chromaFormat);
        exit(EXIT_FAILURE);
        break;
    }
  }
}

/*!
************************************************************************
* \brief
*    Deinterleave file read buffer to source picture structure
************************************************************************
*/
void Input::imageReformat (
                           uint8* buf,           //!< input buffer
                           uint8* out,           //!< output buffer
                           FrameFormat *format,  //!< format of source buffer
                           int symbolSizeInBytes //!< number of bytes per symbol
)
{
  if (format->m_chromaFormat != m_chromaFormat) {
    printf("Warning. ChromaFormat of input and buffer differ (%d != %d)\n", format->m_chromaFormat, m_chromaFormat);
  }
  else {
    bool  rgb_input = (bool) (format->m_colorSpace == CM_RGB && m_chromaFormat == CF_444);
    const int bytesY  = format->m_compSize[Y_COMP] * symbolSizeInBytes;
    const int bytesUV = format->m_compSize[U_COMP] * symbolSizeInBytes;
    int   cmp1 = Y_COMP;
    int   cmp2 = U_COMP;
    int   cmp3 = V_COMP;

    if(rgb_input) {
      switch (format->m_pixelFormat) {
        case PF_BRG:
          cmp1 = B_COMP; cmp2 = R_COMP; cmp3 = G_COMP;
          break;
        case PF_BGR:
          cmp1 = B_COMP; cmp2 = G_COMP; cmp3 = R_COMP;
          break;
        case PF_RBG:
          cmp1 = R_COMP; cmp2 = B_COMP; cmp3 = G_COMP;
          break;
        case PF_GRB:
          cmp1 = G_COMP; cmp2 = R_COMP; cmp3 = B_COMP;
          break;
        case PF_GBR:
          cmp1 = G_COMP; cmp2 = B_COMP; cmp3 = R_COMP;
          break;
        default:
          cmp1 = R_COMP; cmp2 = G_COMP; cmp3 = B_COMP;
          break;
      }
    }

    int bit_scale = format->m_bitDepthComp[cmp1] - m_bitDepthComp[Y_COMP];
    m_bufToImg->process(m_comp[cmp1], buf, format->m_width[cmp1], format->m_height[cmp1], m_width[Y_COMP], m_height[Y_COMP], symbolSizeInBytes, bit_scale);
    
    if (m_chromaFormat != CF_400) {
      bit_scale = format->m_bitDepthComp[cmp2] - m_bitDepthComp[U_COMP];
      m_bufToImg->process(m_comp[cmp2], buf + bytesY, format->m_width[cmp2], format->m_height[cmp2], m_width[U_COMP], m_height[U_COMP], symbolSizeInBytes, bit_scale);
      bit_scale = format->m_bitDepthComp[cmp3] - m_bitDepthComp[V_COMP];
      m_bufToImg->process(m_comp[cmp3], buf + bytesY + bytesUV, format->m_width[cmp3], format->m_height[cmp3], m_width[V_COMP], m_height[V_COMP], symbolSizeInBytes, bit_scale);
    }
  }
}

/*!
************************************************************************
* \brief
*    Deinterleave file read buffer to source picture structure
************************************************************************
*/
void Input::imageReformatUInt16 (
                                 uint8* buf,                //!< input buffer
                                 FrameFormat *format,       //!< format of source buffer
                                 int symbolSizeInBytes   //!< number of bytes per symbol
)
{
  if (format->m_chromaFormat != m_chromaFormat) { // different chroma format buffers? This should not happen.
    printf("Warning. ChromaFormat of input and buffer differ (%d != %d)\n", format->m_chromaFormat, m_chromaFormat);
  }
  else {
    bool  rgb_input = (bool) (format->m_colorSpace == CM_RGB && m_chromaFormat == CF_444);
    const int bytesY  = format->m_compSize[Y_COMP] * symbolSizeInBytes;
    const int bytesUV = format->m_compSize[U_COMP] * symbolSizeInBytes;
    int   cmp1 = Y_COMP;
    int   cmp2 = U_COMP;
    int   cmp3 = V_COMP;

    
    if(rgb_input) {
      switch (format->m_pixelFormat) {
        case PF_BRG:
          cmp1 = B_COMP; cmp2 = R_COMP; cmp3 = G_COMP;
          break;
        case PF_BGR:
          cmp1 = B_COMP; cmp2 = G_COMP; cmp3 = R_COMP;
          break;
        case PF_RBG:
          cmp1 = R_COMP; cmp2 = B_COMP; cmp3 = G_COMP;
          break;
        case PF_GRB:
          cmp1 = G_COMP; cmp2 = R_COMP; cmp3 = B_COMP;
          break;
        case PF_GBR:
          cmp1 = G_COMP; cmp2 = B_COMP; cmp3 = R_COMP;
          break;
        case PF_RGB:
          cmp1 = R_COMP; cmp2 = G_COMP; cmp3 = B_COMP;
          break;
        case PF_R10K:
        case PF_R210:
        case PF_R12B:
        case PF_R12L:
        default:
          cmp1 = B_COMP; cmp2 = R_COMP; cmp3 = G_COMP;
          break;
      }
    }

    int bit_scale = format->m_bitDepthComp[cmp1] - m_bitDepthComp[Y_COMP];
    
    m_bufToImg->process(m_ui16Comp[cmp1], buf, format->m_width[cmp1], format->m_height[cmp1], m_width[Y_COMP], m_height[Y_COMP], symbolSizeInBytes, bit_scale);

    if (m_chromaFormat != CF_400) {
      bit_scale = format->m_bitDepthComp[cmp2] - m_bitDepthComp[U_COMP];
      m_bufToImg->process(m_ui16Comp[cmp2], buf + bytesY, format->m_width[cmp2], format->m_height[cmp2], m_width[U_COMP], m_height[U_COMP], symbolSizeInBytes, bit_scale);
      bit_scale = format->m_bitDepthComp[cmp3] - m_bitDepthComp[V_COMP];
      m_bufToImg->process(m_ui16Comp[cmp3], buf + bytesY + bytesUV, format->m_width[cmp3], format->m_height[cmp3], m_width[V_COMP], m_height[V_COMP], symbolSizeInBytes, bit_scale);
    }
  }
  // For the YCM format, the data are in fact shifted to fit the full 16 bits
  if (format->m_pixelFormat == PF_YCM) {
    for (int i = 0; i < format->m_size; i++) {
      m_ui16Data[i] = m_ui16Data[i] >> 6;
    }
  }
}

std::pair<uint32, uint32> Input::getFraction(float num) const {
  if (num == 0.0f) {
    return std::make_pair<uint32, uint32>(0 ,0); // "unknown"
  }
  else {
    float eps = 0.001f;
    
    float whole;
    float fractional = modff(num, &whole);
    if(fractional < eps)
      return std::make_pair<uint32, uint32>(static_cast<uint32>(whole) ,1);
    else if(fabs(num - 59.94) < eps)
      return std::make_pair<uint32, uint32>(60000 ,1001); // NTSC
    else if(fabs(num - 29.97) < eps)
      return std::make_pair<uint32, uint32>(30000 ,1001); // NTSC
    else if(fabs(num - 23.976) < eps)
      return std::make_pair<uint32, uint32>(24000 ,1001); // Trensferred to NTSC
    
    return std::make_pair<uint32, uint32>(static_cast<uint32>(num*1000.0f) ,1000);
  }
}

void Input::printFormat() {
  //printf("W x H:  (%dx%d) \n", m_width[Y_COMP], m_height[Y_COMP]);
  printf("ColorSpace: %s (%s/%s)\n",  COLOR_SPACE[m_colorSpace + 1], COLOR_PRIMARIES[m_colorPrimaries + 1], TRANSFER_CHAR[m_transferFunction]);
  printf("Format: %s%s, ", COLOR_FORMAT[m_chromaFormat + 1], INTERLACED_TYPE[m_isInterlaced]);
  if (m_isInterleaved || m_colorSpace == CM_RGB) {
    printf("PxFmt: %s (I)", FOUR_CC_CODE[m_pixelFormat + 1]);
  }
  else if (m_colorSpace == CM_YCbCr){
    printf("PxFmt: YUV");
  }
  else {
    printf("PxFmt: N/A");
  }
  printf("\n");
  if (m_isFloat == TRUE) {
    printf("DataType: Float, Type: %s\n",  PIXEL_TYPE[m_pixelType[Y_COMP]]);
    }
  else
  {
    printf("DataType: Fixed, BitDepth: %d, ", m_bitDepthComp[Y_COMP] );
    printf("Range: %s\n", SOURCE_RANGE_TYPE[m_sampleRange + 1]);
  }
  if (m_frameRate == 0.0f)
    printf("Fps: \"unknown\" (%d/%d)\n", m_frameRateNum, m_frameRateDenom);
  else
    printf("Fps: %7.3f (%d/%d)\n",  m_frameRate, m_frameRateNum, m_frameRateDenom);
}
} // namespace hdrtoolslib
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
