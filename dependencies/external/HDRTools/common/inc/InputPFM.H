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
 * \file InputPFM.H
 *
 * \brief
 *    InputPFM class header file for PFM graphic image files
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

#ifndef __InputPFM_H__
#define __InputPFM_H__

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "Input.H"
#include "IOPPM.H"

namespace hdrtoolslib {

//-----------------------------------------------------------------------------
// Class definition
//-----------------------------------------------------------------------------

class InputPFM : public Input {
  
private:
  bool          m_memoryAllocated;
  PGraphics     m_pfm;
  int64         m_pfmSize;
  int64         m_prevSize;

  int64         m_dataSize;
  FrameFormat   m_format;
  
  void          allocateMemory          ( FrameFormat *format );
  void          freeMemory              ();
  
  uint32        readImageData          ( FrameFormat *format, FILE* file );
  uint32        readImageFileHeader    ( FrameFormat *format, FILE* file );
  
  int           reformatData           ( );
  int           openFrameFile          ( IOVideo *inputFile, char *inpFile, int FrameNumberInFile, size_t inpSize);
  int           readFileIntoMemory     ( PGraphics * t, FILE *file, uint32 counter);
  int           readHeaderData         ( FrameFormat *format, FILE* file );
  int           readFile               ( FrameFormat *format, FILE* file);
  uint32        readImageData          ( PGraphics * t );

public:
  InputPFM( IOVideo *inputFile, FrameFormat *format );
  virtual ~InputPFM();
  
  virtual int readOneFrame ( IOVideo *inputFile, int frameNumber, int fileHeader, int frameSkip );
};
} // namespace hdrtoolslib

#endif
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
