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
 * \file InputPNG.cpp
 *
 * \brief
 *    InputPNG class C++ file for allowing input from PNG files
 * 
 *    libpng manual: http://www.libpng.org/pub/png/libpng-manual.txt
 *
 * \author
 *     - Dimitri Podborski
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------
#ifdef LIBPNG
#include <vector>
#include <string.h>
#include <assert.h>
#include "InputPNG.H"
#include "Global.H"
#include "IOFunctions.H"

//-----------------------------------------------------------------------------
// Macros/Defines
//-----------------------------------------------------------------------------
//#define __PRINT_INPUT_PNG__

namespace hdrtoolslib {

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

InputPNG::InputPNG(IOVideo *videoFile, FrameFormat *format) {
  m_isFloat         = FALSE;
  format->m_isFloat = m_isFloat;
  videoFile->m_format.m_isFloat = m_isFloat;
  m_frameRate = format->m_frameRate;
  auto PQ           = getFraction(m_frameRate);
  m_frameRateNum    = PQ.first;
  m_frameRateDenom  = PQ.second;

  m_size      = 0;  
  m_buf       = NULL;

  m_floatComp[Y_COMP] = NULL;
  m_floatComp[U_COMP] = NULL;
  m_floatComp[V_COMP] = NULL;
  m_floatComp[A_COMP] = NULL;
  
  m_ui16Comp[Y_COMP] = NULL;
  m_ui16Comp[U_COMP] = NULL;
  m_ui16Comp[V_COMP] = NULL;
  m_ui16Comp[A_COMP] = NULL;
  
  m_colorPrimaries   = format->m_colorPrimaries;
  m_sampleRange      = format->m_sampleRange;
  m_transferFunction = format->m_transferFunction;
  m_systemGamma      = format->m_systemGamma;
}

InputPNG::~InputPNG() {
  m_comp[Y_COMP]     = NULL;
  m_comp[U_COMP]     = NULL;
  m_comp[V_COMP]     = NULL;
  m_comp[A_COMP]     = NULL;
  
  m_ui16Comp[Y_COMP] = NULL;
  m_ui16Comp[U_COMP] = NULL;
  m_ui16Comp[V_COMP] = NULL;
  m_ui16Comp[A_COMP] = NULL;
  
  m_floatComp[Y_COMP] = NULL;
  m_floatComp[U_COMP] = NULL;
  m_floatComp[V_COMP] = NULL;
  m_floatComp[A_COMP] = NULL;
  clear();
}

/*!
 ************************************************************************
 * \brief
 *    Open file containing a single frame
 ************************************************************************
 */
FILE* InputPNG::openFrameFile( IOVideo *inputFile, int FrameNumberInFile)
{
  char inFile [FILE_NAME_SIZE], in_number[16];
  int length = 0;
  in_number[length]='\0';
  length = (int) strlen(inputFile->m_fHead);
  strncpy(inFile, inputFile->m_fHead, sizeof(inFile));
  inFile[length]='\0';
  
  // Is this a single frame file? If yes, m_fTail would be of size 0.
  if (strlen(inputFile->m_fTail) != 0) {
    if (inputFile->m_zeroPad == TRUE)
      snprintf(in_number, 16, "%0*d", inputFile->m_numDigits, FrameNumberInFile);
    else
      snprintf(in_number, 16, "%*d", inputFile->m_numDigits, FrameNumberInFile);
    
    //strncat(inFile, in_number, sizeof(in_number));
    strncat(inFile, in_number, FILE_NAME_SIZE - strlen(inFile) - 1);
    length += sizeof(in_number);
    inFile[length]='\0';
    strncat(inFile, inputFile->m_fTail, FILE_NAME_SIZE - strlen(inFile) - 1);
    length += (int) strlen(inputFile->m_fTail);
    inFile[length]='\0';
  }

  FILE *fp = fopen(inFile, "rb");
  if(fp == NULL) {
    printf("InputPNG::openFrameFile: cannot open file %s\n", inFile);
  }
  return fp;
}

int InputPNG::readPNG(FrameFormat *format, FILE *fp)
{
  png_structp pPng;
  png_infop   pPngInfo;

  pPng = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(pPng == NULL) {
    printf("Could not create PNG struct.\n");
    return 1;
  }
  pPngInfo = png_create_info_struct(pPng);
  if(pPngInfo == NULL) {
    printf("Could not allocate memory for PNG info struct.\n");
    png_destroy_read_struct(&pPng, NULL, NULL);
    return 1;
  }
  if(setjmp(png_jmpbuf(pPng))) {
    png_destroy_read_struct(&pPng, &pPngInfo, NULL);
    return 1;
  }
  png_init_io(pPng, fp);

  uint32 width, height;
  int bitDepth, colorType, interlaceType, compressionMethod, filterMethod;
  png_read_info(pPng, pPngInfo);
  png_get_IHDR(pPng, pPngInfo, &width, &height, &bitDepth, &colorType,
    &interlaceType, &compressionMethod, &filterMethod);

  m_isInterleaved = FALSE;
  if(interlaceType == PNG_INTERLACE_NONE) {
    format->m_isInterlaced = m_isInterlaced = FALSE;
  }
  else {
    printf("Interlaced PNGs are not supported yet\n");
    png_destroy_read_struct(&pPng, &pPngInfo, NULL);
    return 1;  
  }

  switch(colorType)
  {
  case PNG_COLOR_TYPE_RGB_ALPHA:  // (bit_depths 8, 16)
  case PNG_COLOR_TYPE_PALETTE:    // (bit depths 1, 2, 4, 8)
  case PNG_COLOR_TYPE_RGB:        // (bit_depths 8, 16)
    format->m_colorSpace = m_colorSpace = CM_RGB;
    format->m_chromaFormat = m_chromaFormat = CF_444;
    format->m_pixelFormat = m_pixelFormat = PF_RGB;
    if(colorType == PNG_COLOR_TYPE_PALETTE) {
      printf("NOTE: paletted PNGa are transformed to RGB\n");
      png_set_palette_to_rgb(pPng);
    }
    if(colorType == PNG_COLOR_TYPE_RGB_ALPHA) {
      // TODO: use m_hasAlpha flag in Frame to signal
      printf("NOTE: alpha bytes are striped from the input data.\n");
      png_set_strip_alpha(pPng);
    }
    break;
  case PNG_COLOR_TYPE_GRAY:       // (bit depths 1, 2, 4, 8, 16)
  case PNG_COLOR_TYPE_GRAY_ALPHA: // (bit depths 8, 16)
    format->m_colorSpace = m_colorSpace = CM_UNKNOWN;
    format->m_chromaFormat = m_chromaFormat = CF_400;
    format->m_pixelFormat = m_pixelFormat = PF_UNKNOWN;
    if(bitDepth < 8) png_set_expand_gray_1_2_4_to_8(pPng); // Not sure if png_set_packing is enough
    if(colorType == PNG_COLOR_TYPE_GRAY_ALPHA) {
      // we can use m_hasAlpha flag in Frame to signal
      printf("NOTE: alpha bytes are striped from the input data.\n");
      png_set_strip_alpha(pPng);
    }
    break;
  default:
    printf("Unknown colorType %i\n", colorType);
    png_destroy_read_struct(&pPng, &pPngInfo, NULL);
    return 1;
  }
  
  if(bitDepth < 8) {
    printf("NOTE: bit depths 1, 2 or 4 detected. We will extract each pixel as a single byte and treat it as 8 bit depth.\n");
    png_set_packing(pPng);
    bitDepth = 8;
  }

  m_width[Y_COMP]  = format->m_width[Y_COMP] = width;
  m_height[Y_COMP] = format->m_height[Y_COMP] = height;
  if(format->m_chromaFormat == CF_400) {
    m_width[U_COMP]  = m_width[V_COMP]  = format->m_width[U_COMP]  = format->m_width[V_COMP]  = 0;
    m_height[U_COMP] = m_height[V_COMP] = format->m_height[U_COMP] = format->m_height[V_COMP] = 0;
  }
  else if(format->m_chromaFormat == CF_444) {
    m_width[U_COMP]  = m_width[V_COMP]  = format->m_width[U_COMP]  = format->m_width[V_COMP]  = width;
    m_height[U_COMP] = m_height[V_COMP] = format->m_height[U_COMP] = format->m_height[V_COMP] = height;
  }
  else {
    printf("Chroma format not supported!\n");
    png_destroy_read_struct(&pPng, &pPngInfo, NULL);
    return 1;
  }

  format->m_compSize[Y_COMP] = m_compSize[Y_COMP] = m_height[Y_COMP] * m_width[Y_COMP];
  format->m_compSize[U_COMP] = m_compSize[U_COMP] = m_height[U_COMP] * m_width[U_COMP];
  format->m_compSize[V_COMP] = m_compSize[V_COMP] = m_height[V_COMP] * m_width[V_COMP];

  m_width[A_COMP]  = 0;
  m_height[A_COMP] = 0;
  format->m_width [A_COMP] = m_width[A_COMP];
  format->m_height[A_COMP] = m_height[A_COMP];
  format->m_compSize[A_COMP] = m_compSize[A_COMP] = m_height[A_COMP] * m_width[A_COMP];

  // Note that we do not read alpha but discard it
  format->m_size = m_size = m_compSize[Y_COMP] + m_compSize[U_COMP] + m_compSize[V_COMP] + m_compSize[A_COMP];
  
  format->m_bitDepthComp[Y_COMP] = m_bitDepthComp[Y_COMP] = bitDepth;
  format->m_bitDepthComp[U_COMP] = m_bitDepthComp[U_COMP] = bitDepth;
  format->m_bitDepthComp[V_COMP] = m_bitDepthComp[V_COMP] = bitDepth;
  format->m_bitDepthComp[A_COMP] = m_bitDepthComp[A_COMP] = bitDepth;

  m_chromaLocation[FP_TOP]    = format->m_chromaLocation[FP_TOP];
  m_chromaLocation[FP_BOTTOM] = format->m_chromaLocation[FP_BOTTOM];
  
  if (m_isInterlaced == FALSE && m_chromaLocation[FP_TOP] != m_chromaLocation[FP_BOTTOM]) {
    printf("Progressive Content. Chroma Type Location needs to be the same for both fields.\n");
    printf("Resetting Bottom field chroma location from type %d to type %d\n", m_chromaLocation[FP_BOTTOM], m_chromaLocation[FP_TOP]);
    m_chromaLocation[FP_BOTTOM] = format->m_chromaLocation[FP_BOTTOM] = m_chromaLocation[FP_TOP];    
  }

  // set the component pointers
  m_ui16Comp[Y_COMP] =m_ui16Comp[U_COMP] = m_ui16Comp[V_COMP] = m_ui16Comp[A_COMP] = NULL;
  m_comp[Y_COMP] = m_comp[U_COMP] = m_comp[V_COMP] = m_comp[A_COMP] = NULL;
  m_floatComp[Y_COMP] = m_floatComp[U_COMP] = m_floatComp[V_COMP] = m_floatComp[A_COMP] = NULL;
  if(m_bitDepthComp[Y_COMP] == 8) {
    m_data.resize((unsigned int) m_size);
    m_comp[Y_COMP]      = &m_data[0];
    if(format->m_chromaFormat == CF_444) {
      m_comp[U_COMP]      = m_comp[Y_COMP] + m_compSize[Y_COMP];
      m_comp[V_COMP]      = m_comp[U_COMP] + m_compSize[U_COMP];
    }
  } 
  else if (m_bitDepthComp[Y_COMP] == 16) {
    m_ui16Data.resize((unsigned int) m_size);
    m_ui16Comp[Y_COMP]  = &m_ui16Data[0];
    if(format->m_chromaFormat == CF_444) {
      m_ui16Comp[U_COMP]  = m_ui16Comp[Y_COMP] + m_compSize[Y_COMP];
      m_ui16Comp[V_COMP]  = m_ui16Comp[U_COMP] + m_compSize[U_COMP];
    }
  }
  else {
    printf("Bitdepth %i is not supported\n", m_bitDepthComp[Y_COMP]);
    png_destroy_read_struct(&pPng, &pPngInfo, NULL);
    return 1;
  }

  png_read_update_info(pPng, pPngInfo);

  // read data
  auto row_bytes = png_get_rowbytes(pPng, pPngInfo);
  std::vector<png_bytep> row_pointers(height);
  std::vector<png_byte> buffer(height*row_bytes);
  for(uint32 row = 0; row < height; row++)
    row_pointers[row] = buffer.data() + row * row_bytes;
  png_read_image(pPng, row_pointers.data());
  png_read_end(pPng, pPngInfo);

  uint32 N = row_bytes / width;
  if(bitDepth == 8) {
    for(int i=0; i<m_height[Y_COMP]; i++) {
      for(int j=0; j<m_width[Y_COMP]; j++) {
        if(format->m_chromaFormat == CF_444) {
          m_comp[Y_COMP][i * m_width[Y_COMP] + j] = row_pointers[i][j*N + 0];
          m_comp[U_COMP][i * m_width[U_COMP] + j] = row_pointers[i][j*N + 1];
          m_comp[V_COMP][i * m_width[V_COMP] + j] = row_pointers[i][j*N + 2];
        }
        else {
          m_comp[Y_COMP][i * m_width[Y_COMP] + j] = row_pointers[i][j];
        }
      }
    }
  }
  else if(bitDepth == 16) {
    for(int i=0; i<m_height[Y_COMP]; i++) {
      for(int j=0; j<m_width[Y_COMP]; j++) {
        if(format->m_chromaFormat == CF_444) {
          m_ui16Comp[Y_COMP][i * m_width[Y_COMP] + j] = (row_pointers[i][j*N + 0] << 8) + row_pointers[i][j*N + 1];
          m_ui16Comp[U_COMP][i * m_width[U_COMP] + j] = (row_pointers[i][j*N + 2] << 8) + row_pointers[i][j*N + 3];
          m_ui16Comp[V_COMP][i * m_width[V_COMP] + j] = (row_pointers[i][j*N + 4] << 8) + row_pointers[i][j*N + 5];
        }
        else {
          auto byte1 = row_pointers[i][j*2*2 + 0];
          auto byte2 = row_pointers[i][j*2*2 + 1];
          m_ui16Comp[Y_COMP][i * m_width[Y_COMP] + j] = (row_pointers[i][j*N + 0] << 8) + row_pointers[i][j*N + 1];
        }
      }
    }
  }
  else {
    printf("Bitdepth %i is currently not supported.\n", bitDepth);
    png_destroy_read_struct(&pPng, &pPngInfo, NULL);
    return -1;
  }

  png_destroy_read_struct(&pPng, &pPngInfo, NULL);
  return 0;
}

void InputPNG::printImage() const
{
  if(m_bitDepthComp[Y_COMP] == 8) {
    printf("m_comp[Y_COMP]\n");
    for(int i=0; i<m_height[Y_COMP]; i++) {
      for(int j=0; j<m_width[Y_COMP]; j++) {
        printf("%4i ", m_comp[Y_COMP][j+i*m_height[Y_COMP]]);
      }
      printf("\n");
    }
    printf("\n");
  }
  else if (m_bitDepthComp[Y_COMP] == 16) {
    printf("m_ui16Comp[Y_COMP]\n");
    for(int i=0; i<m_height[Y_COMP]; i++) {
      for(int j=0; j<m_width[Y_COMP]; j++) {
        printf("%4i ", m_ui16Comp[Y_COMP][j+i*m_height[Y_COMP]]);
      }
      printf("\n");
    }
    printf("\n");
  }
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

/*!
 ************************************************************************
 * \brief
 *    Reads one new frame from a single PNG file
 *
 * \param inputFile
 *    Input file to read from
 * \param frameNumber
 *    Frame number in the source file
 * \param fileHeader
 *    Number of bytes in the source file to be skipped
 * \param frameSkip
 *    Start position in file
 ************************************************************************
 */
int InputPNG::readOneFrame (IOVideo *inputFile, int frameNumber, int fileHeader, int frameSkip) {
  FILE* fp = openFrameFile(inputFile, frameNumber + frameSkip);
  if(fp==NULL) {
    return 0;
  }

  int iRetVal = readPNG(&inputFile->m_format, fp);

  fclose(fp);
  return iRetVal == 0;
}
} // namespace hdrtoolslib

#endif
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------

