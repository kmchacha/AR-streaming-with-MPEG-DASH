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
 * \file InputPGM.cpp
 *
 * \brief
 *    InputPGM class C++ file for allowing input of PGM graphic image files
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
#include <stdio.h>
#include "InputPGM.H"
#include "Global.H"
#include "IOFunctions.H"

//-----------------------------------------------------------------------------
// Macros/Defines
//-----------------------------------------------------------------------------

namespace hdrtoolslib {
  
//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

InputPGM::InputPGM(IOVideo *videoFile, FrameFormat *format) {
  // PGM files are integer files only
  format->m_isFloat = m_isFloat = FALSE;
  videoFile->m_format.m_isFloat = m_isFloat;
  //m_format          = *format;
  m_frameRate       = format->m_frameRate;
  auto PQ = getFraction(m_frameRate);
  m_frameRateNum    = PQ.first;
  m_frameRateDenom  = PQ.second;

  m_memoryAllocated = FALSE;
  m_size = 0;
  m_prevSize = -1;
  m_buf      = NULL;
  
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
  
  // Color space is explicitly specified here. We could do a test inside the code though to at least check if
  // RGB is RGB and YUV is YUV, but not important currently
  
  m_colorSpace       = format->m_colorSpace;
  m_colorPrimaries   = format->m_colorPrimaries;
  m_sampleRange      = format->m_sampleRange;
  m_transferFunction = format->m_transferFunction;
  m_systemGamma      = format->m_systemGamma;
}

InputPGM::~InputPGM() {
  
  freeMemory();
  
  clear();
}

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
static uint16 getU16 (PGraphics * t)
{
  uint16 in;
  
  *(((char *) &in) + 0) = *t->mp++;
  *(((char *) &in) + 1) = *t->mp++;
  
  return in;
}


/*!
 ************************************************************************
 * \brief
 *   Get an unsigned int32 without swapping.
 *
 ************************************************************************
 */
static uint32 getU32 (PGraphics * t)
{
  uint32 in;
  
  *(((char *) &in) + 0) = *t->mp++;
  *(((char *) &in) + 1) = *t->mp++;
  *(((char *) &in) + 2) = *t->mp++;
  *(((char *) &in) + 3) = *t->mp++;
  
  return in;
  
}


// Swap versions

/*!
 ************************************************************************
 * \brief
 *   Get an unsigned short and swap.
 *
 ************************************************************************
 */
static uint16 getSwappedU16 (PGraphics * t)
{
  uint16 in;
  
  *(((char *) &in) + 1) = *t->mp++;
  *(((char *) &in) + 0) = *t->mp++;
  
  return in;
}


/*!
 ************************************************************************
 * \brief
 *   Get an unsigned int32 and swap.
 *
 ************************************************************************
 */
static uint32 getSwappedU32 (PGraphics * t)
{
  uint32 in;
  
  *(((char *) &in) + 3) = *t->mp++;
  *(((char *) &in) + 2) = *t->mp++;
  *(((char *) &in) + 1) = *t->mp++;
  *(((char *) &in) + 0) = *t->mp++;
  
  return in;
}

/*!
 ************************************************************************
 * \brief
 *   Read readFileFromMemory into file named 'path' into memory buffer 't->fileInMemory'.
 *
 * \return
 *   0 if successful
 ************************************************************************
 */
int InputPGM::readFileIntoMemory (PGraphics * t, FILE *file, uint32 counter)
{
  int endian = 1;
  int machineLittleEndian = (*( (char *)(&endian) ) == 1) ? 1 : 0;
  
  // resize memory to fit the entire image
  t->fileInMemory.resize(counter);
  
  long result = (long) fread((char *) &t->fileInMemory[0], sizeof(char), counter, file);
  if (result != counter) {
    if (file != NULL) {
      fclose( file);
      file = NULL;
    }
    return 1;
  }
  
  m_ppm.le = 1;
  if (m_ppm.le == machineLittleEndian)  { // endianness of machine matches file
    t->getU16 = getSwappedU16;
    t->getU32 = getSwappedU32;
  }
  else {                               // endianness of machine does not match file
    t->getU16 = getU16;
    t->getU32 = getU32;
  }
  
  t->mp = (uint8 *) &t->fileInMemory[0];
  
  return 0;
}

/*!
 ************************************************************************
 * \brief
 *    Read image data into 't->img'.
 *
 ************************************************************************
 */
uint32 InputPGM::readImageData (PGraphics * t)
{
  uint32  i;
  uint8  *mp, *s;
  uint16 *p;
  
  uint32 size = t->m_height * t->m_width;
  
  t->img.resize(size * sizeof(uint16));
  switch (t->BitsPerSample[0]) {
    case 8:
      p = (uint16 *) &t->img[0];
      s = (uint8 *) &t->fileInMemory[0];
      for (i=0; i < size; ++i) {
        *p++ = *s++;
      }
      break;
    case 16:
      mp = t->mp;                       // save memory pointer
      t->mp = (uint8 *) &t->fileInMemory[0];
      p = (uint16 *) &t->img[0];
      for (i = 0; i < size; ++i) {
        *p++ = (uint16) t->getU16( t);       
      }
      
      t->mp = mp;                       // restore memory pointer
      break;
  }
  return 0;
}

#define  MAX_LEN  100

/*!
 *****************************************************************************
 * \brief
 *    Read the Image File Header.
 *
 *****************************************************************************
 */
uint32 InputPGM::readImageFileHeader (FrameFormat *format, FILE* file)
{
  char line[MAX_LEN + 1];
  int   countLine = 0;
  int32 maxValue = 255;
  uint32 byteCounter = 0;

  while(TRUE) {
    //fscanf(file, "%[^\n]\n", line);
    if (fgets(line, 100, file) == NULL)
      exit(EXIT_FAILURE);
    // First line
    if (countLine == 0)  {
      //if (strcmp(&line[0],  "P5") != 0) {
      if (strncmp(&line[0],  "P5", strlen(line) - 1 ) != 0) {
        printf("Only binary files supported. Error!\n");
        exit(EXIT_FAILURE);
      }
      countLine++;
    }
    else    {
      if (line[0] == '#' ){
        // Comment line. Do nothing
      }
      else  {
        if (countLine == 1) {
          sscanf(line, "%d %d", &m_width[Y_COMP], &m_height[Y_COMP]);
          if (m_width[Y_COMP] <= 0 || m_height[Y_COMP] <= 0) {
            printf("Incorrect resolution. Error!\n");
            exit(EXIT_FAILURE);
          }
        }
        else if (countLine == 2) {
          sscanf(line, "%d", &maxValue);
          if (maxValue > 0) {
            m_bitDepthComp[R_COMP] = RoundLog2(maxValue + 1);
          }
          else {
            printf("Incorrect max value. Error!\n");
            exit(EXIT_FAILURE);
          }
          break;
        }
        countLine++;
      }
    }
  }
  
  m_dataSize = m_width[Y_COMP] * m_height[Y_COMP] * (m_bitDepthComp[R_COMP] > 8 ? 2 : 1);
  
  return byteCounter;
}

/*!
 *****************************************************************************
 * \brief
 *    Read the PGM file.
 *
 *****************************************************************************
 */
int InputPGM::readFile (FrameFormat *format, FILE* file) {
  
  if (readImageFileHeader( format, file))
  goto Error;
  
  allocateMemory(format);
  if (readFileIntoMemory( &m_ppm, file, (uint32) (m_dataSize)))
  goto Error;
  
  if (readImageData( &m_ppm))
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
int InputPGM::openFrameFile( IOVideo *inputFile, char *inFile, int FrameNumberInFile, size_t inSize)
{
  char outNumber[16];
  int length = 0;
  outNumber[length]='\0';
  length = (int) strlen(inputFile->m_fHead);
  strncpy(inFile, inputFile->m_fHead, inSize);
  inFile[length]='\0';
  
  // Is this a single frame file? If yes, m_fTail would be of size 0.
  if (strlen(inputFile->m_fTail) != 0) {
    if (inputFile->m_zeroPad == TRUE)
    snprintf(outNumber, 16, "%0*d", inputFile->m_numDigits, FrameNumberInFile);
    else
    snprintf(outNumber, 16, "%*d", inputFile->m_numDigits, FrameNumberInFile);
    
    strncat(inFile, outNumber, strlen(inFile) - 1);
    length += sizeof(outNumber);
    inFile[length]='\0';
    strncat(inFile, inputFile->m_fTail, strlen(inFile) - 1);
    length += (int) strlen(inputFile->m_fTail);
    inFile[length]='\0';
  }
  
  
  return inputFile->m_fileNum;
}

void InputPGM::allocateMemory(FrameFormat *format)
{
  format->m_height[R_COMP] = m_height[R_COMP];
  format->m_width [R_COMP] = m_width [R_COMP];
  
  m_height [V_COMP] = m_height [U_COMP] = format->m_height[U_COMP] = format->m_height[V_COMP] = 0;
  m_width  [V_COMP] = m_width  [U_COMP] = format->m_width[U_COMP]  = format->m_width[V_COMP]  = 0;

  // We are only dealing with 4:4:4 data here
  m_ppm.m_height = m_height [Y_COMP];
  m_ppm.m_width  = m_width  [Y_COMP];
  
  format->m_compSize[R_COMP] = m_compSize[R_COMP] = m_height[R_COMP] * m_width[R_COMP];
  format->m_compSize[G_COMP] = m_compSize[G_COMP] = m_height[G_COMP] * m_width[G_COMP];
  format->m_compSize[B_COMP] = m_compSize[B_COMP] = m_height[B_COMP] * m_width[B_COMP];
  
  m_height [A_COMP] = 0;
  m_width  [A_COMP] = 0;
  
  format->m_height[A_COMP] = m_height[A_COMP];
  format->m_width [A_COMP] = m_width [A_COMP];
  format->m_compSize[A_COMP] = m_compSize[A_COMP] = m_height[A_COMP] * m_width[A_COMP];
  
  
  // Note that we do not read alpha but discard it
  format->m_size = m_size = m_compSize[Y_COMP] + m_compSize[U_COMP] + m_compSize[V_COMP] + m_compSize[A_COMP];
  
  //format->m_colorSpace   = m_colorSpace   = CM_RGB;
  format->m_chromaFormat = m_chromaFormat = CF_400;
  
  format->m_bitDepthComp[R_COMP] = m_bitDepthComp[R_COMP];
  format->m_bitDepthComp[G_COMP] = m_bitDepthComp[G_COMP] = 0 ;
  format->m_bitDepthComp[B_COMP] = m_bitDepthComp[B_COMP] = 0;
  format->m_bitDepthComp[A_COMP] = m_bitDepthComp[A_COMP] = 16;
  
  if (m_bitDepthComp[0] <= 8)
    m_ppm.BitsPerSample[0] = 8;
  else
    m_ppm.BitsPerSample[0] = 16;
  
  m_isInterleaved = FALSE;
  m_isInterlaced  = FALSE;
  
  m_chromaLocation[FP_TOP]    = format->m_chromaLocation[FP_TOP];
  m_chromaLocation[FP_BOTTOM] = format->m_chromaLocation[FP_BOTTOM];
  
  if (m_isInterlaced == FALSE && m_chromaLocation[FP_TOP] != m_chromaLocation[FP_BOTTOM]) {
    printf("Progressive Content. Chroma Type Location needs to be the same for both fields.\n");
    printf("Resetting Bottom field chroma location from type %d to type %d\n", m_chromaLocation[FP_BOTTOM], m_chromaLocation[FP_TOP]);
    m_chromaLocation[FP_BOTTOM] = format->m_chromaLocation[FP_BOTTOM] = m_chromaLocation[FP_TOP];
  }
  
  if (m_memoryAllocated == FALSE || m_size != m_prevSize) {
    if (format->m_bitDepthComp[Y_COMP] == 8) {
      m_data.resize((unsigned int) m_size);
      m_comp[Y_COMP]      = &m_data[0];
      m_comp[U_COMP]      = m_comp[Y_COMP] + m_compSize[Y_COMP];
      m_comp[V_COMP]      = m_comp[U_COMP] + m_compSize[U_COMP];
      
      m_ui16Comp[Y_COMP]  = NULL;
      m_ui16Comp[U_COMP]  = NULL;
      m_ui16Comp[V_COMP]  = NULL;
    }
    else {
      m_comp[Y_COMP]      = NULL;
      m_comp[U_COMP]      = NULL;
      m_comp[V_COMP]      = NULL;
      
      m_ui16Data.resize((unsigned int) m_size);
      m_ui16Comp[Y_COMP]  = &m_ui16Data[0];
      m_ui16Comp[U_COMP]  = m_ui16Comp[Y_COMP] + m_compSize[Y_COMP];
      m_ui16Comp[V_COMP]  = m_ui16Comp[U_COMP] + m_compSize[U_COMP];
    }
  }
  
  m_prevSize          = m_size;
  m_ui16Comp[A_COMP]  = NULL;
  m_floatComp[Y_COMP] = NULL;
  m_floatComp[U_COMP] = NULL;
  m_floatComp[V_COMP] = NULL;
  
  m_floatComp[A_COMP] = NULL;
  
  m_memoryAllocated = TRUE;
}

void InputPGM::freeMemory()
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

int InputPGM::readData (int vfile,  FrameFormat *source, uint8 *buf) {
  return 1;
}

int InputPGM::reformatData () {
  int k;
  
  if (m_ppm.BitsPerSample[0] == 8) {
    imgpel *comp0 = &m_comp[0][0];
    uint16 *curBuf = (uint16 *) &m_ppm.img[0];
    
    for (k = 0; k < m_size; k++) {
      *comp0++ = (imgpel) *curBuf++;
    }
  }
  else {
    uint16 *comp0 = &m_ui16Comp[0][0];
    uint16 *curBuf = (uint16 *) &m_ppm.img[0];
    
    for (k = 0; k < m_size; k++) {
      *comp0++ = *curBuf++;
    }
  }
  
  return 1;
}


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

/*!
 ************************************************************************
 * \brief
 *    Reads one new frame from a single PGM file
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
int InputPGM::readOneFrame (IOVideo *outputFile, int frameNumber, int fileHeader, int frameSkip) {
  char outFile [FILE_NAME_SIZE];
  FILE*         frameFile = NULL;
  
  int fileRead = 0;
  FrameFormat *format = &outputFile->m_format;
  openFrameFile( outputFile, outFile, frameNumber + frameSkip, sizeof(outFile));
  
  // open file for reading
  frameFile = IOFunctions::openFile(outFile, "r");
  
  if (frameFile != NULL) {
    fileRead = readFile( format, frameFile);
    reformatData ();
    
    IOFunctions::closeFile(frameFile);
  }
  
  return fileRead;
}
  
} // namespace hdrtoolslib
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------

