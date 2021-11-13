/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Apple Inc.
 * <ORGANIZATION> = Apple Inc.
 * <YEAR> = 2018
 *
 * Copyright (c) 2018, Apple Inc.
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
 * \file OutputPPM.cpp
 *
 * \brief
 *    OutputPPM class C++ file for allowing output of PPM files
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include <string.h>
#include "OutputPPM.H"
#include "Global.H"
#include "IOFunctions.H"

//-----------------------------------------------------------------------------
// Macros/Defines
//-----------------------------------------------------------------------------

namespace hdrtoolslib {

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------


OutputPPM::OutputPPM(IOVideo *videoFile, FrameFormat *format) {
  // We currently do not support floating point data
  m_isFloat         = FALSE;
  format->m_isFloat = m_isFloat;
  videoFile->m_format.m_isFloat = m_isFloat;
  videoFile->m_format.m_chromaFormat = format->m_chromaFormat;
  m_format          = *format;
  m_frameRate       = format->m_frameRate;
  auto PQ = getFraction(m_frameRate);
  m_frameRateNum    = PQ.first;
  m_frameRateDenom  = PQ.second;

  m_memoryAllocated = FALSE;
  
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
  
  allocateMemory(format);
  
}

OutputPPM::~OutputPPM() {
  
  freeMemory();
  
  clear();
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------

/*!
 ************************************************************************
 * \brief
 *   Set an unsigned short without swapping.
 *
 ************************************************************************
 */
static uint32 setU16 (PGraphics * t, uint16 in)
{
  char *pIn = (char *) &in;
  *t->mp++ = *pIn++;
  *t->mp++ = *pIn++;
  
  return 2;
}


/*!
 ************************************************************************
 * \brief
 *   Set an unsigned int32 without swapping.
 *
 ************************************************************************
 */
static uint32 setU32 (PGraphics * t, uint32 in)
{
  char *pIn = (char *) &in;
  *t->mp++ = *pIn++;
  *t->mp++ = *pIn++;
  *t->mp++ = *pIn++;
  *t->mp++ = *pIn++;
  
  return 4;
}


// Swap versions

/*!
 ************************************************************************
 * \brief
 *   Set an unsigned short and swap.
 *
 ************************************************************************
 */
static uint32 setSwappedU16 (PGraphics * t, uint16 in)
{
  char *pIn = (char *) &in;
  *t->mp++ = pIn[1];
  *t->mp++ = pIn[0];
  
  return 2;
}


/*!
 ************************************************************************
 * \brief
 *   Set an unsigned int32 and swap.
 *
 ************************************************************************
 */
static uint32 setSwappedU32 (PGraphics * t, uint32 in)
{
  char *pIn = (char *) &in;
  *t->mp++ = pIn[3];
  *t->mp++ = pIn[2];
  *t->mp++ = pIn[1];
  *t->mp++ = pIn[0];
  
  return 2;
}

/*!
 ************************************************************************
 * \brief
 *   Write writeFileFromMemory into file named 'path' into memory buffer 't->fileInMemory'.
 *
 * \return
 *   0 if successful
 ************************************************************************
 */
int OutputPPM::writeFileFromMemory (PGraphics * t, FILE *file, uint32 counter)
{
  long result = (long) fwrite((char *) &t->fileInMemory[0], sizeof(char), counter, file);
  if (result != counter) {
    if (file != NULL) {
      fclose( file);
      file = NULL;
    }
    return 1;
  }
  
  return 0;
}

/*!
 ************************************************************************
 * \brief
 *    Read image data into 't->img'.
 *
 ************************************************************************
 */
uint32 OutputPPM::writeImageData (PGraphics * t)
{
  uint32  i;
  uint8  *mp, *s;
  uint16 *p;
  uint32 byteCounter = 0;
  
  switch (t->BitsPerSample[0]) {
    case 8:
      p = (uint16 *) &t->img[0];
      s = (uint8 *) &t->fileInMemory[0];
      for (i=0; i < m_components * m_ppm.m_height * m_ppm.m_width; ++i) {
        byteCounter ++;
        *s++ = (uint8) *p++;
      }
      break;
    case 16:
      mp = t->mp;                       // save memory pointer
      p = (uint16 *) &t->img[0];
      for (i = 0; i < m_components * m_ppm.m_height * m_ppm.m_width; ++i) {
          byteCounter += t->setU16( t, *p++);
      }
     
      t->mp = mp;                       // restore memory pointer
      break;
  }
  return byteCounter;
}

/*!
 *****************************************************************************
 * \brief
 *    Read the Image File Header.
 *
 *****************************************************************************
 */
uint32 OutputPPM::writeImageFileHeader (FrameFormat *format, FILE* file)
{
  uint32 byteCounter = 0;
  if (format->m_chromaFormat == CF_400) {
    fprintf(file, "P5\n");
  }
  else {
    fprintf(file, "P6\n");
  }
  fprintf(file, "%d %d\n", m_width[Y_COMP], m_height[Y_COMP]);
  fprintf(file, "%d\n", (1 << m_bitDepthComp[Y_COMP]) - 1);
  byteCounter = ftell(file);
  return byteCounter;
}

/*!
 *****************************************************************************
 * \brief
 *    Write the PPM file.
 *
 *****************************************************************************
 */
int OutputPPM::writeFile (FrameFormat *format, FILE* file) {
  
  if (m_memoryAllocated == FALSE) {
    allocateMemory(format);
  }
  
  writeImageFileHeader( format, file);
  
  if (writeFileFromMemory( &m_ppm, file, (uint32) (maxFramePosition)))
    goto Error;
  
  return 1;
  
Error:
  return 0;
}


/*!
 ************************************************************************
 * \brief
 *    Open file containing a single frame
 ************************************************************************
 */
int OutputPPM::openFrameFile( IOVideo *outputFile, char *outFile, int FrameNumberInFile, size_t outSize)
{
  char outNumber[16];
  int length = 0;
  outNumber[length]='\0';
  length = (int) strlen(outputFile->m_fHead);
  strncpy(outFile, outputFile->m_fHead, outSize);
  outFile[length]='\0';
  
  // Is this a single frame file? If yes, m_fTail would be of size 0.
  if (strlen(outputFile->m_fTail) != 0) {
    if (outputFile->m_zeroPad == TRUE)
      snprintf(outNumber, 16, "%0*d", outputFile->m_numDigits, FrameNumberInFile);
    else
      snprintf(outNumber, 16, "%*d", outputFile->m_numDigits, FrameNumberInFile);
    
    strncat(outFile, outNumber, strlen(outFile) - 1); 
    length += sizeof(outNumber);
    outFile[length]='\0';
    strncat(outFile, outputFile->m_fTail, strlen(outFile) - 1);
    length += (int) strlen(outputFile->m_fTail);
    outFile[length]='\0';
  }
  
  
  //*file = IOFunctions::openFile(outFile, "w+t");
  
  return outputFile->m_fileNum;
}

void OutputPPM::allocateMemory(FrameFormat *format)
{
  m_memoryAllocated = TRUE;
  m_chromaFormat     = format->m_chromaFormat;
    
  m_ppm.m_height = m_height[Y_COMP] = format->m_height[Y_COMP];
  m_ppm.m_width  = m_width[Y_COMP]  = format->m_width [Y_COMP];
  
  switch (m_chromaFormat){
    case CF_400:
      m_width [U_COMP] = m_width [V_COMP] = format->m_height[U_COMP] = format->m_height[V_COMP] = 0;
      m_height[U_COMP] = m_height[V_COMP] = format->m_width[U_COMP] = format->m_width[V_COMP] = 0;
      m_components = 1;
      break;
    case CF_420:
      m_width [U_COMP] = m_width [V_COMP] = format->m_height[U_COMP] = format->m_height[V_COMP] = m_width [Y_COMP] >> ONE;
      m_height[U_COMP] = m_height[V_COMP] = format->m_width[U_COMP] = format->m_width[V_COMP] = m_height[Y_COMP] >> ONE;
      m_components = 3;
      break;
    case CF_422:
      m_width [U_COMP] = m_width [V_COMP] = format->m_height[U_COMP] = format->m_height[V_COMP] = m_width [Y_COMP] >> ONE;
      m_height[U_COMP] = m_height[V_COMP] = format->m_width[U_COMP] = format->m_width[V_COMP] = m_height[Y_COMP];
      m_components = 3;
      break;
    case CF_444:
      m_width [U_COMP] = m_width [V_COMP] = format->m_height[U_COMP] = format->m_height[V_COMP] = m_width [Y_COMP];
      m_height[U_COMP] = m_height[V_COMP] = format->m_width[U_COMP] = format->m_width[V_COMP] = m_height[Y_COMP];
      m_components = 3;
      break;
    default:
      fprintf(stderr, "\n Unsupported Chroma Subsampling Format %d\n", m_chromaFormat);
      exit(EXIT_FAILURE);
  }
  m_height [A_COMP] = 0;
  m_width  [A_COMP] = 0;
  
  m_compSize[Y_COMP] = format->m_compSize[Y_COMP] = m_height[Y_COMP] * m_width[Y_COMP];
  m_compSize[U_COMP] = format->m_compSize[U_COMP] = m_height[U_COMP] * m_width[U_COMP];
  m_compSize[V_COMP] = format->m_compSize[V_COMP] = m_height[V_COMP] * m_width[V_COMP];
  m_compSize[A_COMP] = format->m_compSize[A_COMP] = m_height[A_COMP] * m_width[A_COMP];
  
  m_size = format->m_size = m_compSize[Y_COMP] + m_compSize[U_COMP] + m_compSize[V_COMP] + m_compSize[A_COMP];
  
  // Color space is explicitly specified here. We could do a test inside the code though to at least check if
  // RGB is RGB and YUV is YUV, but not important currently
  m_colorSpace       = format->m_colorSpace;
  m_colorPrimaries   = format->m_colorPrimaries;
  m_sampleRange      = format->m_sampleRange;
  
  //format->m_colorSpace   = m_colorSpace   = CM_RGB;
  m_transferFunction = format->m_transferFunction;
  m_systemGamma      = format->m_systemGamma;
  
  m_bitDepthComp[Y_COMP] = (m_sampleRange == SR_SDI_SCALED && format->m_bitDepthComp[Y_COMP] > 8) ? 16 : format->m_bitDepthComp[Y_COMP];
  m_bitDepthComp[U_COMP] = (m_sampleRange == SR_SDI_SCALED && format->m_bitDepthComp[U_COMP] > 8) ? 16 : format->m_bitDepthComp[U_COMP];
  m_bitDepthComp[V_COMP] = (m_sampleRange == SR_SDI_SCALED && format->m_bitDepthComp[U_COMP] > 8) ? 16 : format->m_bitDepthComp[V_COMP];
  
  m_ppm.BitsPerSample[Y_COMP] = (m_bitDepthComp[Y_COMP] > 8 ? 16 : 8);
  m_ppm.BitsPerSample[U_COMP] = (m_bitDepthComp[U_COMP] > 8 ? 16 : 8);
  m_ppm.BitsPerSample[V_COMP] = (m_bitDepthComp[V_COMP] > 8 ? 16 : 8);
  
  
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
  
  
  // init size of file based on image data to write (without headers
  m_ppmSize = m_size * (m_ppm.BitsPerSample[Y_COMP] > 8 ? 2 : 1);
    
  // memory buffer for the image data
  m_ppm.img.resize((unsigned int) m_ppmSize);
  // Assign a rather large buffer
  m_ppm.fileInMemory.resize((long) m_size * 4);
  
  if (format->m_bitDepthComp[Y_COMP] == 8) {
    m_data.resize((unsigned int) m_size);
    m_comp[Y_COMP]      = &m_data[0];
    m_comp[U_COMP]      = m_comp[Y_COMP] + m_compSize[Y_COMP];
    m_comp[V_COMP]      = m_comp[U_COMP] + m_compSize[U_COMP];
    m_comp[A_COMP]      = NULL;
    m_ui16Comp[Y_COMP]  = NULL;
    m_ui16Comp[U_COMP]  = NULL;
    m_ui16Comp[V_COMP]  = NULL;
    m_ui16Comp[A_COMP]  = NULL;
  }
  else {
    m_comp[Y_COMP]      = NULL;
    m_comp[U_COMP]      = NULL;
    m_comp[V_COMP]      = NULL;
    m_ui16Data.resize((unsigned int) m_size);
    m_ui16Comp[Y_COMP]  = &m_ui16Data[0];
    m_ui16Comp[U_COMP]  = m_ui16Comp[Y_COMP] + m_compSize[Y_COMP];
    m_ui16Comp[V_COMP]  = m_ui16Comp[U_COMP] + m_compSize[U_COMP];
    m_ui16Comp[A_COMP]  = NULL;
  }
  
  m_floatComp[Y_COMP] = NULL;
  m_floatComp[U_COMP] = NULL;
  m_floatComp[V_COMP] = NULL;
  m_floatComp[A_COMP] = NULL;
  
  int endian = 1;
  int machineLittleEndian = (*( (char *)(&endian) ) == 1) ? 1 : 0;
  
  m_ppm.le = 1;
  if (m_ppm.le == machineLittleEndian)  { // endianness of machine matches file
    m_ppm.setU16 = setSwappedU16;
    m_ppm.setU32 = setSwappedU32;
  }
  else {                               // endianness of machine does not match file
    m_ppm.setU16 = setU16;
    m_ppm.setU32 = setU32;
  }
  m_ppm.mp = (uint8 *) &m_ppm.fileInMemory[0];
  
}

void OutputPPM::freeMemory()
{
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
}

/*!
 ************************************************************************
 * \brief
 *    Read Header data
 ************************************************************************
 */

int OutputPPM::writeData (int vfile,  FrameFormat *source, uint8 *buf) {
  return 1;
}

int OutputPPM::reformatData () {
  int i, k;
  
  if (m_ppm.BitsPerSample[0] == 8) {
    imgpel *comp0 = NULL;
    imgpel *comp1 = NULL;
    imgpel *comp2 = NULL;
    
    uint16 *curBuf = (uint16 *) &m_ppm.img[0];
    
    // Unpack the data appropriately (interleaving is done at the row level).
    if (m_components == 1) {
      for (k = 0; k < m_height[Y_COMP]; k++) {
        comp0   = &m_comp[0][k * m_width[0]];
        for (i = 0; i < m_width[Y_COMP]; i++) {
          *curBuf++ = *comp0++;
        }
      }
    }
    else {
      for (k = 0; k < m_height[Y_COMP]; k++) {
        comp0   = &m_comp[0][k * m_width[0]];
        comp1   = &m_comp[1][k * m_width[1]];
        comp2   = &m_comp[2][k * m_width[2]];
        for (i = 0; i < m_width[Y_COMP]; i++) {
          *curBuf++ = *comp0++;
          *curBuf++ = *comp1++;
          *curBuf++ = *comp2++;
        }
      }
    }
    maxFramePosition =m_height[Y_COMP] * m_width[Y_COMP] * m_components;
  }
  else {
    uint16 *comp0 = NULL;
    uint16 *comp1 = NULL;
    uint16 *comp2 = NULL;
    
    uint16 *curBuf = (uint16 *) &m_ppm.img[0];
    
    // Unpack the data appropriately (interleaving is done at the row level).
    if (m_components == 1) {
      for (k = 0; k < m_height[Y_COMP]; k++) {
        comp0   = &m_ui16Comp[0][k * m_width[0]];
        for (i = 0; i < m_width[Y_COMP]; i++) {
          *curBuf++ = *comp0++;
        }
      }
    }
    else {
      for (k = 0; k < m_height[Y_COMP]; k++) {
        comp0   = &m_ui16Comp[0][k * m_width[0]];
        comp1   = &m_ui16Comp[1][k * m_width[1]];
        comp2   = &m_ui16Comp[2][k * m_width[2]];
        for (i = 0; i < m_width[Y_COMP]; i++) {
          *curBuf++ = *comp0++;
          *curBuf++ = *comp1++;
          *curBuf++ = *comp2++;
        }
      }
    }
    maxFramePosition =m_height[Y_COMP] * m_width[Y_COMP] * m_components * 2;
  }
  return 1;
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
int OutputPPM::writeOneFrame (IOVideo *outputFile, int frameNumber, int fileHeader, int frameSkip) {
  char outFile [FILE_NAME_SIZE];
  FILE*         frameFile = NULL;

  int fileWrite = 1;
  FrameFormat *format = &outputFile->m_format;
  openFrameFile( outputFile, outFile, frameNumber + frameSkip, sizeof(outFile));
  
  frameFile = IOFunctions::openFile(outFile, "w+t");

  if (frameFile != NULL) {
    
    reformatData ();
    writeImageData( &m_ppm);

    fileWrite = writeFile( format, frameFile);
    
    IOFunctions::closeFile(frameFile);
  }
  
  return fileWrite;
}
} // namespace hdrtoolslib
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------

