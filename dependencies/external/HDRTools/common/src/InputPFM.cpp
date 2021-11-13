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
 * \file InputPFM.cpp
 *
 * \brief
 *    InputPFM class C++ file for allowing input of PFM graphic image files
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
#include "InputPFM.H"
#include "Global.H"
#include "IOFunctions.H"

//-----------------------------------------------------------------------------
// Macros/Defines
//-----------------------------------------------------------------------------

namespace hdrtoolslib {

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

InputPFM::InputPFM(IOVideo *videoFile, FrameFormat *format) {
  // PFM files are Floating point files only
  format->m_isFloat = m_isFloat = TRUE;
  videoFile->m_format.m_isFloat = m_isFloat;
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

InputPFM::~InputPFM() {
  
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
int InputPFM::readFileIntoMemory (PGraphics * t, FILE *file, uint32 counter)
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
  
  if (m_pfm.le == machineLittleEndian)  { // endianness of machine matches file
    t->getU16 = getU16;
    t->getU32 = getU32;
  }
  else {                               // endianness of machine does not match file
    t->getU16 = getSwappedU16;
    t->getU32 = getSwappedU32;
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
uint32 InputPFM::readImageData (PGraphics * t)
{
  uint32  i;
  uint8  *mp;
  uint32 *p;
  
  uint32 size = t->m_height * t->m_width * 3;
  
  t->img.resize(size * sizeof(float));
  mp = t->mp;                       // save memory pointer
  t->mp = (uint8 *) &t->fileInMemory[0];
  p = (uint32 *) &t->img[0];
  for (i = 0; i < size; ++i) {
    *p++ = (uint32) t->getU32( t);
  }
  
  t->mp = mp;                       // restore memory pointer
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
uint32 InputPFM::readImageFileHeader (FrameFormat *format, FILE* file)
{
  char line[MAX_LEN + 1];
  int   countLine = 0;
  float endianess;
  uint32 byteCounter = 0;
  
  while(TRUE) {
    //fscanf(file, "%[^\n]\n", line);
    if (fgets(line, 100, file) == NULL) 
       exit(EXIT_FAILURE);
    // First line
    if (countLine == 0)  {
      if (strncmp(&line[0],  "PF", strlen(line) - 1 ) != 0) {
        printf("Only binary PFM files supported. Error!\n");
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
          sscanf(line, "%f", &endianess);
          if (endianess == -1.0f) {
            m_pfm.le = 1;
          }
          else {
            m_pfm.le = 0;
          }
          break;
        }
        countLine++;
      }
    }
  }
  
  m_dataSize = 3 * m_width[Y_COMP] * m_height[Y_COMP] * sizeof(float);
  
  return byteCounter;
}

/*!
 *****************************************************************************
 * \brief
 *    Read the PFM file.
 *
 *****************************************************************************
 */
int InputPFM::readFile (FrameFormat *format, FILE* file) {
  
  if (readImageFileHeader( format, file))
    goto Error;
  
  allocateMemory(format);
  if (readFileIntoMemory( &m_pfm, file, (uint32) (m_dataSize)))
    goto Error;
  
  if (readImageData( &m_pfm))
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
int InputPFM::openFrameFile( IOVideo *inputFile, char *inFile, int FrameNumberInFile, size_t inSize)
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

void InputPFM::allocateMemory(FrameFormat *format)
{
  format->m_height[Y_COMP] = format->m_height[U_COMP] = format->m_height[V_COMP] = m_height[Y_COMP];
  format->m_width [Y_COMP] = format->m_width [U_COMP] = format->m_width [V_COMP] = m_width [Y_COMP];
  
  // We are only dealing with 4:4:4 data here
  m_pfm.m_height = m_height [V_COMP] = m_height [U_COMP] = m_height [Y_COMP];
  m_pfm.m_width  = m_width  [V_COMP] = m_width  [U_COMP] = m_width  [Y_COMP];
  
  format->m_compSize[R_COMP] = m_compSize[R_COMP] = m_height[R_COMP] * m_width[R_COMP];
  format->m_compSize[G_COMP] = m_compSize[G_COMP] = m_height[G_COMP] * m_width[G_COMP];
  format->m_compSize[B_COMP] = m_compSize[B_COMP] = m_height[B_COMP] * m_width[B_COMP];
  
  m_height [A_COMP] = 0;
  m_width  [A_COMP] = 0;
  
  format->m_height[A_COMP] = m_height[A_COMP];
  format->m_width [A_COMP] = m_width [A_COMP];
  format->m_compSize[A_COMP] = m_compSize[A_COMP] = m_height[A_COMP] * m_width[A_COMP];
  
  format->m_pixelType[R_COMP] = m_pixelType[R_COMP] = FLOAT;
  format->m_pixelType[G_COMP] = m_pixelType[G_COMP] = FLOAT;
  format->m_pixelType[B_COMP] = m_pixelType[B_COMP] = FLOAT;
  format->m_pixelType[A_COMP] = m_pixelType[A_COMP] = FLOAT;

  // Note that we do not read alpha but discard it
  format->m_size = m_size = m_compSize[Y_COMP] + m_compSize[U_COMP] + m_compSize[V_COMP] + m_compSize[A_COMP];
  
  //format->m_colorSpace   = m_colorSpace   = CM_RGB;
  format->m_chromaFormat = m_chromaFormat = CF_444;
  
  format->m_bitDepthComp[R_COMP] = m_bitDepthComp[R_COMP];
  format->m_bitDepthComp[G_COMP] = m_bitDepthComp[G_COMP] ;
  format->m_bitDepthComp[B_COMP] = m_bitDepthComp[B_COMP];
  format->m_bitDepthComp[A_COMP] = m_bitDepthComp[A_COMP] = 16;
  
  if (m_bitDepthComp[0] <= 8)
  m_pfm.BitsPerSample[0] = 8;
  else
  m_pfm.BitsPerSample[0] = 16;
  
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
    m_comp[Y_COMP]      = NULL;
    m_comp[U_COMP]      = NULL;
    m_comp[V_COMP]      = NULL;
    
    m_ui16Comp[Y_COMP]  = NULL;
    m_ui16Comp[U_COMP]  = NULL;
    m_ui16Comp[V_COMP]  = NULL;
    
    m_floatData.resize((unsigned int) m_size);
    m_floatComp[Y_COMP]  = &m_floatData[0];
    m_floatComp[U_COMP]  = m_floatComp[Y_COMP] + m_compSize[Y_COMP];
    m_floatComp[V_COMP]  = m_floatComp[U_COMP] + m_compSize[U_COMP];
  }
  
  m_prevSize          = m_size;
  m_ui16Comp[A_COMP]  = NULL;
  m_floatComp[A_COMP] = NULL;
  
  m_memoryAllocated = TRUE;
}

void InputPFM::freeMemory()
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

int InputPFM::reformatData () {
  int i, k;
  
  float *comp0 = NULL;
  float *comp1 = NULL;
  float *comp2 = NULL;
  
  float *curBuf = (float *) &m_pfm.img[0];
  
  // Unpack the data appropriately (interleaving is done at the row level).
  for (k = 0; k < m_height[Y_COMP]; k++) {
    comp0   = &m_floatComp[0][k * m_width[0]];
    comp1   = &m_floatComp[1][k * m_width[1]];
    comp2   = &m_floatComp[2][k * m_width[2]];
    for (i = 0; i < m_width[Y_COMP]; i++) {
      *comp0++ = *curBuf++;
      *comp1++ = *curBuf++;
      *comp2++ = *curBuf++;
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
 *    Reads one new frame from a single PFM file
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
int InputPFM::readOneFrame (IOVideo *inputFile, int frameNumber, int fileHeader, int frameSkip) {
  char inFile [FILE_NAME_SIZE];
  FILE*         frameFile = NULL;
  
  int fileRead = 0;
  FrameFormat *format = &inputFile->m_format;
  openFrameFile( inputFile, inFile, frameNumber + frameSkip, sizeof(inFile));
  
  // open file for reading
  frameFile = IOFunctions::openFile(inFile, "r");
  
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

