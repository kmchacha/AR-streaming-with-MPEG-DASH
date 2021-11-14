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
 * \file OutputY4M.cpp
 *
 * \brief
 *    OutputY4M class C++ file for allowing output of YUV4MPEG2 files
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *     - Dimitri Podborski
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include <string.h>
#include "OutputY4M.H"
#include "Global.H"
#include "IOFunctions.H"

#include <iostream>
#include <string>

//-----------------------------------------------------------------------------
// Macros/Defines
//-----------------------------------------------------------------------------

namespace hdrtoolslib {

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

OutputY4M::OutputY4M(IOVideo *outputFile, FrameFormat *format) {
  m_isFloat              = FALSE;
  format->m_isFloat      = m_isFloat;

  m_pos = 0;
  writeStreamHeader (outputFile,  format);

  m_colorSpace           = format->m_colorSpace;
  m_colorPrimaries       = format->m_colorPrimaries;
  m_transferFunction     = format->m_transferFunction;
  m_systemGamma          = format->m_systemGamma;
  m_sampleRange          = format->m_sampleRange;

  m_chromaFormat         = format->m_chromaFormat;
  m_isInterleaved        = outputFile->m_isInterleaved;
  m_isInterlaced         = format->m_isInterlaced;
  
  m_chromaLocation[FP_TOP]    = format->m_chromaLocation[FP_TOP];
  m_chromaLocation[FP_BOTTOM] = format->m_chromaLocation[FP_BOTTOM];
  
  if (m_isInterlaced == FALSE && m_chromaLocation[FP_TOP] != m_chromaLocation[FP_BOTTOM]) {
    printf("Progressive Content. Chroma Type Location needs to be the same for both fields.\n");
    printf("Resetting Bottom field chroma location from type %d to type %d\n", m_chromaLocation[FP_BOTTOM], m_chromaLocation[FP_TOP]);
    m_chromaLocation[FP_BOTTOM] = format->m_chromaLocation[FP_BOTTOM] = m_chromaLocation[FP_TOP];    
  }

  m_pixelFormat          = format->m_pixelFormat;
  m_frameRate            = format->m_frameRate;
  auto PQ = getFraction(m_frameRate);
  m_frameRateNum    = PQ.first;
  m_frameRateDenom  = PQ.second;

  m_picUnitSizeOnDisk    = format->m_picUnitSizeOnDisk;
  m_picUnitSizeShift3    = format->m_picUnitSizeShift3;


  m_bitDepthComp[Y_COMP] = format->m_bitDepthComp[Y_COMP];
  m_bitDepthComp[U_COMP] = format->m_bitDepthComp[U_COMP];
  m_bitDepthComp[V_COMP] = format->m_bitDepthComp[V_COMP];

  m_height[Y_COMP]       = format->m_height[Y_COMP];
  m_width [Y_COMP]       = format->m_width [Y_COMP];
  
  // Sim2 params
  m_cositedSampling       = format->m_cositedSampling;
  m_improvedFilter        = format->m_improvedFilter;

  if (format->m_chromaFormat == CF_420) {
    format->m_height[V_COMP] = format->m_height[U_COMP] = m_height [V_COMP] = m_height [U_COMP] = m_height [Y_COMP] >> 1;
    format->m_width [V_COMP] = format->m_width [U_COMP] = m_width  [V_COMP] = m_width  [U_COMP] = m_width  [Y_COMP] >> 1;
  }
  else if (format->m_chromaFormat == CF_422) {
    format->m_height[V_COMP] = format->m_height[U_COMP] = m_height [V_COMP] = m_height [U_COMP] = m_height [Y_COMP];
    format->m_width [V_COMP] = format->m_width [U_COMP] = m_width  [V_COMP] = m_width  [U_COMP] = m_width  [Y_COMP] >> 1;
  }
  else {
    format->m_height[V_COMP] = format->m_height[U_COMP] = m_height [V_COMP] = m_height [U_COMP] = m_height [Y_COMP];
    format->m_width [V_COMP] = format->m_width [U_COMP] = m_width  [V_COMP] = m_width  [U_COMP] = m_width  [Y_COMP];
  }

  format->m_compSize[Y_COMP] = m_compSize[Y_COMP] = m_height[Y_COMP] * m_width[Y_COMP];
  format->m_compSize[U_COMP] = m_compSize[U_COMP] = m_height[U_COMP] * m_width[U_COMP];
  format->m_compSize[V_COMP] = m_compSize[V_COMP] = m_height[V_COMP] * m_width[V_COMP];

  m_size = m_compSize[Y_COMP] + m_compSize[U_COMP] + m_compSize[V_COMP];
  m_bufSize = m_size;
  
  if (m_pixelFormat == PF_B64A)
    m_bufSize += m_compSize[A_COMP];

  if (m_isInterleaved) {
    m_iBuffer.resize((unsigned int) m_bufSize * m_picUnitSizeShift3);
    m_iBuf = &m_iBuffer[0];
  }

  m_buffer.resize((unsigned int) m_bufSize * m_picUnitSizeShift3);
  m_buf = &m_buffer[0];

  if (m_picUnitSizeShift3 > 1) {
    m_ui16Data.resize((unsigned int) m_size);
    m_ui16Comp[Y_COMP] = &m_ui16Data[0];
    m_ui16Comp[U_COMP] = m_ui16Comp[Y_COMP] + m_compSize[Y_COMP];
    m_ui16Comp[V_COMP] = m_ui16Comp[U_COMP] + m_compSize[U_COMP];
    m_comp[Y_COMP] = NULL;
    m_comp[U_COMP] = NULL;
    m_comp[V_COMP] = NULL;
    m_floatComp[Y_COMP] = NULL;
    m_floatComp[U_COMP] = NULL;
    m_floatComp[V_COMP] = NULL;
  }
  else {
    m_ui16Comp[Y_COMP] = NULL;
    m_ui16Comp[U_COMP] = NULL;
    m_ui16Comp[V_COMP] = NULL;

    m_data.resize((unsigned int) m_size);
    m_comp[Y_COMP] = &m_data[0];
    m_comp[U_COMP] = m_comp[Y_COMP] + m_compSize[Y_COMP];
    m_comp[V_COMP] = m_comp[U_COMP] + m_compSize[U_COMP];

    m_floatComp[Y_COMP] = NULL;
    m_floatComp[U_COMP] = NULL;
    m_floatComp[V_COMP] = NULL;
  }
  outputFile->m_format = *format;
}

OutputY4M::~OutputY4M() {
  m_iBuf = NULL;
  m_buf = NULL;
  m_comp[Y_COMP] = NULL;
  m_comp[U_COMP] = NULL;
  m_comp[V_COMP] = NULL;
  
  m_ui16Comp[Y_COMP] = NULL;
  m_ui16Comp[U_COMP] = NULL;
  m_ui16Comp[V_COMP] = NULL;
  
  m_floatComp[Y_COMP] = NULL;
  m_floatComp[U_COMP] = NULL;
  m_floatComp[V_COMP] = NULL;
    
  clear();
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------

int OutputY4M::writeData (int vfile, uint8 *buf) {
  uint8 *curBuf = buf;
  int writeSize = m_picUnitSizeShift3 * m_width[Y_COMP];
  
  int i, j;
    
  for (i = 0; i < m_height[Y_COMP]; i++) {
    if (mm_write(vfile, curBuf, writeSize) != writeSize) {
      printf ("writeSize: cannot write %d bytes to output file, unexpected error!\n", m_width[Y_COMP]);
      return 0;
    }
    curBuf += writeSize;
  }
  
  if (m_chromaFormat != CF_400) {
    for (j = U_COMP; j <= V_COMP; j++) {
      writeSize = m_picUnitSizeShift3 * m_width[ (int) j];
      for (i = 0; i < m_height[(int) j]; i++) {
        if (mm_write(vfile, curBuf, writeSize) != writeSize) {
          printf ("writeSize: cannot write %d bytes to output file, unexpected error!\n", m_width[j]);
          return 0;
        }
        curBuf += writeSize;
      }
    }
  }
  return 1;
}

int OutputY4M::writeData (int vfile, int framesizeInBytes, uint8 *buf) {
  if (mm_write(vfile, buf, framesizeInBytes) != framesizeInBytes) {
    printf ("writeSize: cannot write %d bytes to output file, unexpected error!\n", framesizeInBytes);
    return 0;
  }
  return 1;
}



int64 OutputY4M::getFrameSizeInBytes(FrameFormat *source, bool isInterleaved)
{
  uint32 symbolSizeInBytes = m_picUnitSizeShift3;
  int64 framesizeInBytes;
  
  const int bytesY  = m_compSize[Y_COMP];
  const int bytesUV = m_compSize[U_COMP];
  
  if (isInterleaved == FALSE) {
    framesizeInBytes = (bytesY + 2 * bytesUV) * symbolSizeInBytes;
  }
  else {
    if (source->m_pixelFormat == PF_PCK) {
      if (source->m_bitDepthComp[0] == 10) {
        framesizeInBytes = (bytesY + 2 * bytesUV) * 5 / 4;
      }
      else {
        framesizeInBytes = (bytesY + 2 * bytesUV) * 3 / 2;
      }
    }
    else {
#ifdef __SIM2_SUPPORT_ENABLED__
      if (source->m_pixelFormat == PF_SIM2) {
        framesizeInBytes = (bytesY * 3);
      }
      else 
#endif
      {
        switch (source->m_chromaFormat) {
          case CF_420:
            framesizeInBytes = (bytesY + 2 * bytesUV) * symbolSizeInBytes;
            break;
          case CF_422:
            switch (source->m_pixelFormat) {
              case PF_YUYV:
              case PF_YUY2:
              case PF_YVYU:
              case PF_UYVY:
                framesizeInBytes = (bytesY + 2 * bytesUV) * symbolSizeInBytes;
                break;
              case PF_V210:
              case PF_UYVY10:
                // Pack 12 10-bit samples into each 16 bytes
                //framesizeInBytes = ((bytesY + 2 * bytesUV) / 3) << 2;
                framesizeInBytes = (bytesY / 3) << 3;
                break;
              default:
                fprintf(stderr, "Unsupported pixel format.\n");
                exit(EXIT_FAILURE);
                break;
            }
            break;
          case CF_444:
            if (source->m_pixelFormat == PF_V410 || source->m_pixelFormat == PF_R210 || source->m_pixelFormat == PF_R10K) {
              // Pack 3 10-bit samples into a 32 bit little-endian word
              framesizeInBytes = bytesY * 4;
            }
            else if(source->m_pixelFormat== PF_R12B || source->m_pixelFormat== PF_R12L) {
              // Pack 8 12-bit samples into a 9 x 32 bit words
              framesizeInBytes = ((bytesY * 3 * 3 ) >> 1);
            }
            else if (source->m_pixelFormat == PF_B64A) {
              framesizeInBytes = bytesY * 4;
            }
            else {
              framesizeInBytes = (bytesY + 2 * bytesUV) * symbolSizeInBytes;
            }
            break;
          default:
            fprintf(stderr, "Unknown Chroma Format type %d\n", source->m_chromaFormat);
            exit(EXIT_FAILURE);
            break;
        }
 
       }
    }
  }
  
  return framesizeInBytes;
}

int OutputY4M::writeStreamHeader (IOVideo *outputFile,  FrameFormat *source) {
  int  vfile = outputFile->m_fileNum;

  // header
  int len = mm_write(vfile, "YUV4MPEG2", 9);
  m_pos += 9;
  lseek (vfile, m_pos, SEEK_SET);
  
  // width
  std::string w = " W" + std::to_string(source->m_width[Y_COMP]);
  len = mm_write(vfile, w.c_str(), w.size());
  m_pos += w.size();
  lseek (vfile, m_pos, SEEK_SET);
  
  // height
  std::string h = " H" + std::to_string(source->m_height[Y_COMP]);
  len = mm_write(vfile, h.c_str(), h.size());
  m_pos += h.size();
  lseek (vfile, m_pos, SEEK_SET);
  
  // frame rate
  auto PQ = getFraction(source->m_frameRate);
  std::string f = " F" + std::to_string(PQ.first) + ":" + std::to_string(PQ.second);
  len = mm_write(vfile, f.c_str(), f.size());
  m_pos += f.size();
  lseek (vfile, m_pos, SEEK_SET);
  
  // interlacing
  char interlaceMode = getInterlaceMode(source);
  if(interlaceMode) {
    std::string i = std::string(" I") + interlaceMode;
    len = mm_write(vfile, i.c_str(), i.size());
    m_pos += i.size();
    lseek (vfile, m_pos, SEEK_SET);
  }
  
  // color space
  std::string colourSpace = getChromaSubsampling(source);
  if(colourSpace.size()>0) {
    std::string c = std::string(" C") + colourSpace;
    len = mm_write(vfile, c.c_str(), c.size());
    m_pos += c.size();
    lseek (vfile, m_pos, SEEK_SET);
  }

  // end of stream header
  char endByte = 0x0A;
  mm_write(vfile, &endByte, 1);
  m_pos += 1;
  lseek (vfile, m_pos, SEEK_SET);
  return 0;
}

int OutputY4M::writeFrameHeader (int vfile,  FrameFormat *format) {
  int len = mm_write(vfile, "FRAME", 5);
  m_pos += 5;
  lseek (vfile, m_pos, SEEK_SET);

  // TODO: more parameters could go here (each preceded by space)

  // write end of frame header
  char endByte = 0x0A;
  len = mm_write(vfile, &endByte, 1);
  m_pos += 1;
  lseek (vfile, m_pos, SEEK_SET);
  return 0;
}

char OutputY4M::getInterlaceMode(FrameFormat *format) const {
  char mode = 0;
 
  if(!format)
    return mode;
 
  if(!format->m_isInterlaced) {
    mode = 'p';
  } else {
    // TODO: add support for interlacing signalling
    mode = 't';
    printf("Note: curretnly only top field interlacing is signalled.\n");
  }
  
  return mode;
}

std::string OutputY4M::getChromaSubsampling(FrameFormat *format) const {
  
  if(format->m_chromaLocation[FP_TOP]    == CL_ONE && 
     format->m_chromaLocation[FP_BOTTOM] == CL_ONE &&
     format->m_chromaFormat              == CF_420 &&
     format->m_bitDepthComp  [Y_COMP]    == 8 &&
     format->m_bitDepthComp  [U_COMP]    == 8 &&
     format->m_bitDepthComp  [V_COMP]    == 8 ) {
    return "420jpeg";
  }
 
  if(format->m_chromaLocation[FP_TOP]    == CL_ZERO &&
     format->m_chromaLocation[FP_BOTTOM] == CL_ZERO &&
     format->m_chromaFormat              == CF_420 &&
     format->m_bitDepthComp  [Y_COMP]    == 8 &&
     format->m_bitDepthComp  [U_COMP]    == 8 &&
     format->m_bitDepthComp  [V_COMP]    == 8 ) {
    return "420mpeg2";
  }

  if (format->m_chromaLocation[FP_TOP]    == CL_ZERO &&
      format->m_chromaLocation[FP_BOTTOM] == CL_ZERO &&
      format->m_chromaFormat              == CF_420 &&
      format->m_bitDepthComp  [Y_COMP]    == 10 &&
      format->m_bitDepthComp  [U_COMP]    == 10 &&
      format->m_bitDepthComp  [V_COMP]    == 10 ) {
    return "420p10";
  }

  if (format->m_chromaLocation[FP_TOP]    == CL_ZERO &&
      format->m_chromaLocation[FP_BOTTOM] == CL_ZERO &&
      format->m_chromaFormat              == CF_420 &&
      format->m_bitDepthComp  [Y_COMP]    == 12 &&
      format->m_bitDepthComp  [U_COMP]    == 12 &&
      format->m_bitDepthComp  [V_COMP]    == 12 ) {
    return "420p12";
  }

  if (format->m_chromaLocation[FP_TOP]    == CL_ZERO &&
      format->m_chromaLocation[FP_BOTTOM] == CL_ZERO &&
      format->m_chromaFormat              == CF_420 &&
      format->m_bitDepthComp  [Y_COMP]    == 14 &&
      format->m_bitDepthComp  [U_COMP]    == 14 &&
      format->m_bitDepthComp  [V_COMP]    == 14 ) {
    return "420p14";
  }

  if (format->m_chromaLocation[FP_TOP]    == CL_ZERO &&
      format->m_chromaLocation[FP_BOTTOM] == CL_ZERO &&
      format->m_chromaFormat              == CF_422 &&
      format->m_bitDepthComp  [Y_COMP]    == 10 &&
      format->m_bitDepthComp  [U_COMP]    == 10 &&
      format->m_bitDepthComp  [V_COMP]    == 10 ) {
    return "422p10";
  }

  if (format->m_chromaLocation[FP_TOP]    == CL_ZERO &&
      format->m_chromaLocation[FP_BOTTOM] == CL_ZERO &&
      format->m_chromaFormat              == CF_422 &&
      format->m_bitDepthComp  [Y_COMP]    == 12 &&
      format->m_bitDepthComp  [U_COMP]    == 12 &&
      format->m_bitDepthComp  [V_COMP]    == 12 ) {
    return "422p12";
  }

  if (format->m_chromaLocation[FP_TOP]    == CL_ZERO &&
      format->m_chromaLocation[FP_BOTTOM] == CL_ZERO &&
      format->m_chromaFormat              == CF_422 &&
      format->m_bitDepthComp  [Y_COMP]    == 14 &&
      format->m_bitDepthComp  [U_COMP]    == 14 &&
      format->m_bitDepthComp  [V_COMP]    == 14 ) {
    return "422p14";
  }

  if (format->m_chromaFormat              == CF_444 &&
      format->m_bitDepthComp  [Y_COMP]    == 10 &&
      format->m_bitDepthComp  [U_COMP]    == 10 &&
      format->m_bitDepthComp  [V_COMP]    == 10 ) {
    return "444p10";
  }

  if (format->m_chromaFormat              == CF_444 &&
      format->m_bitDepthComp  [Y_COMP]    == 12 &&
      format->m_bitDepthComp  [U_COMP]    == 12 &&
      format->m_bitDepthComp  [V_COMP]    == 12 ) {
    return "444p12";
  }

  if (format->m_chromaFormat              == CF_444 &&
      format->m_bitDepthComp  [Y_COMP]    == 14 &&
      format->m_bitDepthComp  [U_COMP]    == 14 &&
      format->m_bitDepthComp  [V_COMP]    == 14 ) {
    return "444p14";
  }

  if (format->m_chromaFormat              == CF_444 &&
      format->m_bitDepthComp  [Y_COMP]    == 16 &&
      format->m_bitDepthComp  [U_COMP]    == 16 &&
      format->m_bitDepthComp  [V_COMP]    == 16 ) {
    return "444p16";
  }

  if (format->m_chromaLocation[FP_TOP]    == CL_ZERO &&
      format->m_chromaLocation[FP_BOTTOM] == CL_ZERO &&
      format->m_chromaFormat              == CF_420 &&
      format->m_bitDepthComp  [Y_COMP]    == 8 &&
      format->m_bitDepthComp  [U_COMP]    == 8 &&
      format->m_bitDepthComp  [V_COMP]    == 8 ) {
    printf("format 444 with alpha (not supported)\n");
    return "420paldv";
  }

  if (format->m_chromaLocation[FP_TOP]    == CL_ZERO &&
      format->m_chromaLocation[FP_BOTTOM] == CL_ZERO &&
      format->m_chromaFormat              == CF_420 &&
      format->m_bitDepthComp  [Y_COMP]    == 8 &&
      format->m_bitDepthComp  [U_COMP]    == 8 &&
      format->m_bitDepthComp  [V_COMP]    == 8 ) {
    printf("format 411 cosited (not supported)\n");
    return "411";
  }

  if (format->m_chromaLocation[FP_TOP]    == CL_ZERO &&
      format->m_chromaLocation[FP_BOTTOM] == CL_ZERO &&
      format->m_chromaFormat              == CF_422 &&
      format->m_bitDepthComp  [Y_COMP]    == 8 &&
      format->m_bitDepthComp  [U_COMP]    == 8 &&
      format->m_bitDepthComp  [V_COMP]    == 8 ) {
    return "422";
  }

  if (format->m_chromaFormat              == CF_444 &&
      format->m_bitDepthComp  [Y_COMP]    == 8 &&
      format->m_bitDepthComp  [U_COMP]    == 8 &&
      format->m_bitDepthComp  [V_COMP]    == 8 ) {
    return "444";
  }

  if(format->m_chromaFormat              == CF_400 &&
     format->m_bitDepthComp  [Y_COMP]    == 8 &&
     format->m_bitDepthComp  [U_COMP]    == 8 &&
     format->m_bitDepthComp  [V_COMP]    == 8 ) {
    return "mono";
  }

  return "";
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------


/*!
 ************************************************************************
 * \brief
 *    Writes one new frame into a concatenated raw file
 *
 * \param outputFile
 *    Output file to write to
 * \param frameNumber
 *    Frame number in the output file
 * \param fileHeader
 *    Number of bytes in the output file to be skipped when writing
 * \param frameSkip
 *    Start position in file
 ************************************************************************
 */
int OutputY4M::writeOneFrame (IOVideo *outputFile, int frameNumber, int fileHeader, int frameSkip) {
  int fileWrite = 0;
  int vfile = outputFile->m_fileNum;
  FrameFormat *format = &outputFile->m_format;
  
  const int64 framesizeInBytes = getFrameSizeInBytes(format, outputFile->m_isInterleaved);
#ifdef __SIM2_SUPPORT_ENABLED__
  bool isBytePacked = (bool) (outputFile->m_isInterleaved && (format->m_pixelFormat == PF_V210 || format->m_pixelFormat == PF_UYVY10 || format->m_pixelFormat == PF_PCK || format->m_pixelFormat == PF_SIM2)) ? TRUE : FALSE;
#else
  bool isBytePacked = (bool) (outputFile->m_isInterleaved && (format->m_pixelFormat == PF_V210 || format->m_pixelFormat == PF_UYVY10 || format->m_pixelFormat == PF_PCK)) ? TRUE : FALSE;
#endif
  
  // Here we are at the correct position for the source frame in the file.
  // Now write it.
  if ((format->m_picUnitSizeOnDisk & 0x07) == 0)  {
    if (this->m_bitDepthComp[Y_COMP] == 8)
      imageReformat ( m_buf, &m_data[0], format, m_picUnitSizeShift3 );
    else
      imageReformatUInt16 ( m_buf, format, m_picUnitSizeShift3 );

    // If format is interleaved, then perform reinterleaving
    if (m_isInterleaved)
      reInterleave ( &m_buf, &m_iBuf, format, m_picUnitSizeShift3);

    writeFrameHeader(vfile, format);
    if (isBytePacked)
      fileWrite = writeData (vfile, (int) framesizeInBytes, m_buf);
    else
      fileWrite = writeData (vfile, m_buf);
  }
  else {
    fprintf (stderr, "writeOneFrame (NOT IMPLEMENTED): pic unit size on disk must be divided by 8");
    exit(EXIT_FAILURE);
  }

  // seek to the next frame
  m_pos += fileHeader + framesizeInBytes * (1 + frameSkip);
  if(lseek (vfile, m_pos, SEEK_SET) == -1)  {
    fprintf(stderr, "writeOneFrame: cannot lseek to (Header size) in output file\n");
    exit(EXIT_FAILURE);
  }

  return fileWrite;
}
} // namespace hdrtoolslib
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------

