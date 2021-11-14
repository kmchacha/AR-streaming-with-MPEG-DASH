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
 * \file OutputPNG.cpp
 *
 * \brief
 *    OutputPNG class C++ file for allowing input from PNG files
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
#include "OutputPNG.H"
#include "Global.H"
#include "IOFunctions.H"

#include <fstream>

//-----------------------------------------------------------------------------
// Macros/Defines
//-----------------------------------------------------------------------------

namespace hdrtoolslib {

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

OutputPNG::OutputPNG(IOVideo *videoFile, FrameFormat *format) {
  // We currently do not support floating point data
  m_isFloat         = FALSE;
  format->m_isFloat = m_isFloat;
  videoFile->m_format.m_isFloat = m_isFloat;
  m_frameRate       = format->m_frameRate;
  auto PQ = getFraction(m_frameRate);
  m_frameRateNum    = PQ.first;
  m_frameRateDenom  = PQ.second;
  
  m_comp[Y_COMP]      = NULL;
  m_comp[U_COMP]      = NULL;
  m_comp[V_COMP]      = NULL;
  
  m_floatComp[Y_COMP] = NULL;
  m_floatComp[U_COMP] = NULL;
  m_floatComp[V_COMP] = NULL;
  m_floatComp[A_COMP] = NULL;
  
  m_ui16Comp[Y_COMP] = NULL;
  m_ui16Comp[U_COMP] = NULL;
  m_ui16Comp[V_COMP] = NULL;
  m_ui16Comp[A_COMP] = NULL;

  init(format);
}

OutputPNG::~OutputPNG() {
  clear();
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------

FILE* OutputPNG::openFrameFile( IOVideo *outputFile, int FrameNumberInFile)
{
  char outFile [FILE_NAME_SIZE], outNumber[16];
  int length = 0;
  outNumber[length]='\0';
  length = (int) strlen(outputFile->m_fHead);
  strncpy(outFile, outputFile->m_fHead, sizeof(outFile));
  outFile[length]='\0';
  
  // Is this a single frame file? If yes, m_fTail would be of size 0.
  if (strlen(outputFile->m_fTail) != 0) {
    if (outputFile->m_zeroPad == TRUE)
      snprintf(outNumber, 16, "%0*d", outputFile->m_numDigits, FrameNumberInFile);
    else
      snprintf(outNumber, 16, "%*d", outputFile->m_numDigits, FrameNumberInFile);
    
    strncat(outFile, outNumber, FILE_NAME_SIZE - strlen(outFile) - 1);
    length += sizeof(outNumber);
    outFile[length]='\0';
    strncat(outFile, outputFile->m_fTail, FILE_NAME_SIZE - strlen(outFile) - 1);
    length += (int) strlen(outputFile->m_fTail);
    outFile[length]='\0';
  }
  
  FILE *fp = fopen(outFile, "wb");
  if(fp == NULL) {
    printf("OutputPNG::openFrameFile: cannot open file %s\n", outFile);
  }
  return fp;
}

void OutputPNG::libpngOnError(png_structp png_ptr, png_const_charp error_msg)
{
  printf("libpng error: %s\n", error_msg);
  exit(EXIT_FAILURE);
}

void OutputPNG::libpngOnWarning(png_structp png_ptr, png_const_charp warning_msg)
{
  printf("libpng warning: %s\n", warning_msg);
}

void OutputPNG::init(FrameFormat *format)
{
  m_chromaFormat    = format->m_chromaFormat = CF_444;
  m_pixelFormat     = format->m_pixelFormat = PF_RGB; // curretnly we only deal with RGB
  m_height[Y_COMP]  = format->m_height[Y_COMP];
  m_width[Y_COMP]   = format->m_width[Y_COMP];

  if(m_height[Y_COMP] == 0 || m_width[Y_COMP] == 0) {
    printf("Check output dimensions\n");
    exit(EXIT_FAILURE);
  }

  switch (m_chromaFormat){
    case CF_400:
      m_width [U_COMP] = m_width [V_COMP] = format->m_height[U_COMP] = format->m_height[V_COMP] = 0;
      m_height[U_COMP] = m_height[V_COMP] = format->m_width[U_COMP] = format->m_width[V_COMP] = 0;
      printf("Chroma subsammpling format 400 is not supported\n");
      exit(EXIT_FAILURE);
      break;
    case CF_420:
      m_width [U_COMP] = m_width [V_COMP] = format->m_height[U_COMP] = format->m_height[V_COMP] = m_width [Y_COMP] >> ONE;
      m_height[U_COMP] = m_height[V_COMP] = format->m_width[U_COMP] = format->m_width[V_COMP] = m_height[Y_COMP] >> ONE;
      printf("Chroma subsammpling format 420 is not supported\n");
      exit(EXIT_FAILURE);
      break;
    case CF_422:
      m_width [U_COMP] = m_width [V_COMP] = format->m_height[U_COMP] = format->m_height[V_COMP] = m_width [Y_COMP] >> ONE;
      m_height[U_COMP] = m_height[V_COMP] = format->m_width[U_COMP] = format->m_width[V_COMP] = m_height[Y_COMP];
      printf("Chroma subsammpling format 422 is not supported\n");
      exit(EXIT_FAILURE);
      break;
    case CF_444:
      m_width [U_COMP] = m_width [V_COMP] = format->m_height[U_COMP] = format->m_height[V_COMP] = m_width [Y_COMP];
      m_height[U_COMP] = m_height[V_COMP] = format->m_width[U_COMP] = format->m_width[V_COMP] = m_height[Y_COMP];
      break;
    default:
      fprintf(stderr, "\n Unsupported Chroma Subsampling Format %d\n", m_chromaFormat);
      exit(EXIT_FAILURE);
  }

  // copy ICC profile path and set profile name based on the file base name
  m_strICCProfilePath = std::string(format->m_strICCProfilePath);
  m_strICCProfileName = m_strICCProfilePath;
  // Strip directory.
  const size_t foundSlash = m_strICCProfileName.find_last_of("/\\");
  if(std::string::npos != foundSlash)
  {
    m_strICCProfileName = m_strICCProfileName.substr(foundSlash+1);
  }
  // Remove extension if present.
  const size_t foundDot = m_strICCProfileName.rfind('.');
  if (std::string::npos != foundDot)
  {
    m_strICCProfileName = m_strICCProfileName.substr(0, foundDot);
  }

  m_height [A_COMP] = 0;
  m_width  [A_COMP] = 0;
  m_compSize[Y_COMP] = format->m_compSize[Y_COMP] = m_height[Y_COMP] * m_width[Y_COMP];
  m_compSize[U_COMP] = format->m_compSize[U_COMP] = m_height[U_COMP] * m_width[U_COMP];
  m_compSize[V_COMP] = format->m_compSize[V_COMP] = m_height[V_COMP] * m_width[V_COMP];
  m_compSize[A_COMP] = format->m_compSize[A_COMP] = m_height[A_COMP] * m_width[A_COMP];

  m_size = format->m_size = m_compSize[Y_COMP] + m_compSize[U_COMP] + m_compSize[V_COMP] + m_compSize[A_COMP];

  
  m_colorPrimaries   = format->m_colorPrimaries;
  m_sampleRange      = format->m_sampleRange;
  m_colorSpace       = format->m_colorSpace = CM_RGB; // force RGB
  m_transferFunction = format->m_transferFunction;
  m_systemGamma      = format->m_systemGamma;

  m_bitDepthComp[Y_COMP] = (m_sampleRange == SR_SDI_SCALED && format->m_bitDepthComp[Y_COMP] > 8) ? 16 : format->m_bitDepthComp[Y_COMP];
  m_bitDepthComp[U_COMP] = (m_sampleRange == SR_SDI_SCALED && format->m_bitDepthComp[U_COMP] > 8) ? 16 : format->m_bitDepthComp[U_COMP];
  m_bitDepthComp[V_COMP] = (m_sampleRange == SR_SDI_SCALED && format->m_bitDepthComp[U_COMP] > 8) ? 16 : format->m_bitDepthComp[V_COMP];
  m_bitDepthComp[A_COMP] = format->m_bitDepthComp[A_COMP];

  m_isInterleaved = FALSE;
  m_isInterlaced  = FALSE;

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
  if (m_bitDepthComp[Y_COMP] == 8) {
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
    exit(EXIT_FAILURE);
  }
}

int OutputPNG::writePNG(FrameFormat *format, FILE *fp)
{
  png_structp pPng;
  png_infop   pPngInfo;

  pPng = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, &OutputPNG::libpngOnError, &OutputPNG::libpngOnWarning);
  if(pPng == NULL)
  {
    printf("Could not create PNG struct.\n");
    return 1;
  }
  pPngInfo = png_create_info_struct(pPng);
  if(pPngInfo == NULL)
  {
    printf("Could not allocate memory for PNG info struct.\n");
    png_destroy_write_struct(&pPng,  NULL);
    return 1;
  }
  png_init_io(pPng, fp);

  // init was here

  png_set_IHDR(pPng, pPngInfo, m_width[Y_COMP], m_height[Y_COMP], m_bitDepthComp[Y_COMP],
    PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  png_color_8 sig_bit;
  sig_bit.red = m_bitDepthComp[Y_COMP];
  sig_bit.green = m_bitDepthComp[U_COMP];
  sig_bit.blue = m_bitDepthComp[V_COMP];
  png_set_sBIT(pPng, pPngInfo, &sig_bit);

  // png_set_gAMA(pPng, pPngInfo, static_cast<double>(m_systemGamma));

  // write iCCP chunk if possible
  std::ifstream iccInput(m_strICCProfilePath, std::ios::binary);
  if(iccInput)
  {
    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(iccInput), {});
    png_set_iCCP(pPng, pPngInfo, m_strICCProfileName.c_str(), 0, buffer.data(), buffer.size());
  }

  // Write the file header information.
  png_write_info(pPng, pPngInfo);

  auto bytesPerPixel = (m_bitDepthComp[Y_COMP] * 3) / 8;
  if(m_height[Y_COMP] > PNG_SIZE_MAX / (m_width[Y_COMP] * bytesPerPixel))
    png_error(pPng, "Image data buffer would be too large");
  
  if(m_height[Y_COMP] > PNG_UINT_32_MAX / (sizeof (png_bytep)))
    png_error(pPng, "Image is too tall to process in memory");
  
  std::vector<png_byte> image(m_height[Y_COMP] * m_width[Y_COMP] * bytesPerPixel);
  std::vector<png_bytep> row_pointers(m_height[Y_COMP]);

  // Set up pointers into your "image" byte array.
  for(int row = 0; row < m_height[Y_COMP]; row++)
    row_pointers[row] = image.data() + row * m_width[Y_COMP] * bytesPerPixel;

  if(m_bitDepthComp[Y_COMP] == 8) {
    for(int i=0; i<m_height[Y_COMP]; i++)
    {
      for(int j=0; j<m_width[Y_COMP]; j++)
      {
        row_pointers[i][j*bytesPerPixel + 0] = m_comp[Y_COMP][i * m_width[Y_COMP] + j];
        row_pointers[i][j*bytesPerPixel + 1] = m_comp[U_COMP][i * m_width[U_COMP] + j];
        row_pointers[i][j*bytesPerPixel + 2] = m_comp[V_COMP][i * m_width[V_COMP] + j];
      }
    }
  }
  else if(m_bitDepthComp[Y_COMP] == 16) {
    for(int i=0; i<m_height[Y_COMP]; i++) {
      for(int j=0; j<m_width[Y_COMP]; j++) {
        row_pointers[i][j*bytesPerPixel + 0] = m_ui16Comp[Y_COMP][i * m_width[Y_COMP] + j] >> 8;
        row_pointers[i][j*bytesPerPixel + 1] = m_ui16Comp[Y_COMP][i * m_width[Y_COMP] + j];
        row_pointers[i][j*bytesPerPixel + 2] = m_ui16Comp[U_COMP][i * m_width[U_COMP] + j] >> 8;
        row_pointers[i][j*bytesPerPixel + 3] = m_ui16Comp[U_COMP][i * m_width[U_COMP] + j];
        row_pointers[i][j*bytesPerPixel + 4] = m_ui16Comp[V_COMP][i * m_width[V_COMP] + j] >> 8;
        row_pointers[i][j*bytesPerPixel + 5] = m_ui16Comp[V_COMP][i * m_width[V_COMP] + j];
      }
    }
  }
  else {
    printf("Bitdepth %i is currently not supported.\n", m_bitDepthComp[Y_COMP]);
    png_destroy_write_struct(&pPng, &pPngInfo);
    return -1;
  }

  // Write out the entire image data in one call
  png_write_image(pPng, row_pointers.data());

  // finish writing the rest of the file
  png_write_end(pPng, pPngInfo);

  png_destroy_write_struct(&pPng, &pPngInfo);
  return 0;
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

/*!
 ************************************************************************
 * \brief
 *    Writes one new frame into a single Tiff file
 *
 * \param inputFile
 *    Output file to write into
 * \param frameNumber
 *    Frame number in the source file
 * \param fileHeader
 *    Number of bytes in the source file to be skipped
 * \param frameSkip
 *    Start position in file
 ************************************************************************
 */
int OutputPNG::writeOneFrame (IOVideo *outputFile, int frameNumber, int fileHeader, int frameSkip) {

  FILE* fp = openFrameFile(outputFile, frameNumber + frameSkip);
  if(fp==NULL) {
    return 0;
  }

  int iRetVal = writePNG(&outputFile->m_format, fp);

  fclose(fp);
  return iRetVal == 0;
}
} // namespace hdrtoolslib

#endif
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------

