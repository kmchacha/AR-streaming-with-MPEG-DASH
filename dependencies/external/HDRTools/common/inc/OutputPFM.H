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
 * \file OutputPFM.H
 *
 * \brief
 *    OutputPFM class header file for PFM graphics image files
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

#ifndef __OutputPFM_H__
#define __OutputPFM_H__

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "Output.H"
#include "IOPPM.H"

namespace hdrtoolslib {

//-----------------------------------------------------------------------------
// Class definition
//-----------------------------------------------------------------------------

class OutputPFM : public Output {
  
private:
  bool          m_memoryAllocated;
  PGraphics     m_ppm;
  int64         m_ppmSize;
  int64         m_dataSize;
  FrameFormat   m_format;
  
  void          allocateMemory          ( FrameFormat *format );
  void          freeMemory              ();
  
  uint32        writeImageData          ( FrameFormat *format, FILE* file );
  uint32        writeImageFileHeader    ( FrameFormat *format, FILE* file );
  
  int           reformatData            ( );
  int           openFrameFile           ( IOVideo *outputFile, char *outFile, int FrameNumberInFile, size_t outSize);
  int           writeData               ( int vfile,  FrameFormat *source, uint8 *buf );
  int           writeFileFromMemory     ( PGraphics * t, FILE *file, uint32 counter);
  int           writeHeaderData         ( FrameFormat *format, FILE* file );
  int           writeFile               ( FrameFormat *format, FILE* file);
  uint32        writeImageData          ( PGraphics * t );

public:
  OutputPFM( IOVideo *inputFile, FrameFormat *format );
  virtual ~OutputPFM();
  
  virtual int writeOneFrame ( IOVideo *inputFile, int frameNumber, int fileHeader, int frameSkip );
};
} // namespace hdrtoolslib

#endif
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
