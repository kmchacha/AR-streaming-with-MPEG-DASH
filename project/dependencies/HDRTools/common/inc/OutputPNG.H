/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Apple Inc.
 * <ORGANIZATION> = Apple Inc.
 * <YEAR> = 2021
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
 * \file OutputPNG.H
 *
 * \brief
 *    OutputPNG class header file for OpenPNG files
 *
 * \author
 *     - Dimitri Podborski         <dpodborski@apple.com>
 *
 *************************************************************************************
 */

#ifndef __OutputPNG_H__
#define __OutputPNG_H__

#ifdef LIBPNG

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------
#include <vector>
#include <string>
#include "Output.H"
#include "PNGLibConfig.H"
#include <png.h>

namespace hdrtoolslib {

//-----------------------------------------------------------------------------
// Class definition
//-----------------------------------------------------------------------------

class OutputPNG : public Output {
  
private:
  std::string m_strICCProfilePath;
  std::string m_strICCProfileName;
  FILE* openFrameFile( IOVideo *outputFile, int FrameNumberInFile);
  int   writePNG(FrameFormat *format, FILE *fp);
  static void libpngOnError(png_structp png_ptr, png_const_charp error_msg);
  static void libpngOnWarning(png_structp png_ptr, png_const_charp warning_msg);
  void init(FrameFormat *format);
public:
  OutputPNG( IOVideo *inputFile, FrameFormat *format );
  virtual ~OutputPNG();
  virtual int writeOneFrame ( IOVideo *inputFile, int frameNumber, int fileHeader, int frameSkip );
};
} // namespace hdrtoolslib

#endif
#endif
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
