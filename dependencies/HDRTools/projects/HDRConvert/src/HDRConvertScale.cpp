/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Apple Inc.
 * <ORGANIZATION> = Apple Inc.
 * <YEAR> = 2021
 *
 * Copyright (c) 2021, Apple Inc.
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
 * \file HDRConvertScale.cpp
 *
 * \brief
 *    HDRConvertScale class source files for performing YUV scaling 
 *    or Bitdepth conversion
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *************************************************************************************
 */

#include <time.h>
#include <string.h>
#include <math.h>
#include "HDRConvertScale.H"

HDRConvertScale::HDRConvertScale(ProjectParameters *inputParams) {
  m_oFrameStore       = NULL;
  m_iFrameStore       = NULL;
  
  m_convertFrameStore      =  NULL;
  m_convertBitDepth        =  NULL;
  m_inputFrame             =  NULL;
  m_outputFrame            =  NULL;
  m_scaledFrame            =  NULL;
  m_frameScale             =  NULL;
  
  m_inputFile              = &inputParams->m_inputFile;
  m_outputFile             = &inputParams->m_outputFile;
  m_startFrame             =  m_inputFile->m_startFrame;
  
  m_cropOffsetLeft         =  inputParams->m_cropOffsetLeft;
  m_cropOffsetTop          =  inputParams->m_cropOffsetTop;
  m_cropOffsetRight        =  inputParams->m_cropOffsetRight;
  m_cropOffsetBottom       =  inputParams->m_cropOffsetBottom;

  m_croppedFrameStore      =  NULL;
  
}

//-----------------------------------------------------------------------------
// deallocate memory - destroy objects
//-----------------------------------------------------------------------------

void HDRConvertScale::destroy() {   
  // output frame objects
  if (m_oFrameStore != NULL) {
    delete m_oFrameStore;
    m_oFrameStore = NULL;
  }
  // input frame objects
  if (m_iFrameStore != NULL) {
    delete m_iFrameStore;
    m_iFrameStore = NULL;
  }
  
  if (m_frameScale != NULL) {
    delete m_frameScale;
    m_frameScale = NULL;
  }
  
   if (m_convertFrameStore != NULL) {
    delete m_convertFrameStore;
    m_convertFrameStore = NULL;
  }
  
  if (m_convertBitDepth != NULL) {
    delete m_convertBitDepth;
    m_convertBitDepth = NULL;
  }
  
  // Cropped frame store
  if (m_croppedFrameStore != NULL) {
    delete m_croppedFrameStore;
    m_croppedFrameStore = NULL;
  }
   
  if (m_scaledFrame != NULL) {
    delete m_scaledFrame;
    m_scaledFrame = NULL;
  }
 
  if (m_inputFrame != NULL) {
    delete m_inputFrame;
    m_inputFrame = NULL;
  }
  
  if (m_outputFrame != NULL) {
    delete m_outputFrame;
    m_outputFrame = NULL;
  }
  
    
  hdrtoolslib::IOFunctions::closeFile(m_inputFile);
  hdrtoolslib::IOFunctions::closeFile(m_outputFile);
}

//-----------------------------------------------------------------------------
// allocate memory - create objects
//-----------------------------------------------------------------------------
void HDRConvertScale::init (ProjectParameters *inputParams) {
  using ::hdrtoolslib::FrameFormat;
  using ::hdrtoolslib::IOFunctions;
  using ::hdrtoolslib::Y_COMP;
  using ::hdrtoolslib::Frame;
  FrameFormat   *input  = &inputParams->m_source;
  FrameFormat   *output = &inputParams->m_output;
  
   // Input frame objects initialization
  IOFunctions::openFile (m_inputFile);
  
  if (m_inputFile->m_isConcatenated == false && strlen(m_inputFile->m_fTail) == 0) {
    // Update number of frames to be processed
    inputParams->m_numberOfFrames = 1;
  }
  
  // create memory for reading the input filesource
  m_inputFrame = hdrtoolslib::Input::create(m_inputFile, input, inputParams);
  
  // Output file
  IOFunctions::openFile (m_outputFile, OPENFLAGS_WRITE, OPEN_PERMISSIONS);
  
  // create frame memory as necessary
  // Input. This has the same format as the Input file.
  m_iFrameStore  = new Frame(m_inputFrame->m_width[Y_COMP], m_inputFrame->m_height[Y_COMP], m_inputFrame->m_isFloat, m_inputFrame->m_colorSpace, m_inputFrame->m_colorPrimaries, m_inputFrame->m_chromaFormat, m_inputFrame->m_sampleRange, m_inputFrame->m_bitDepthComp[Y_COMP], m_inputFrame->m_isInterlaced, m_inputFrame->m_transferFunction, m_inputFrame->m_systemGamma);
  m_iFrameStore->clear();
  
  // Here we may convert to the output's format type (e.g. from integer to float).
  m_convertFrameStore  = new Frame(m_inputFrame->m_width[Y_COMP], m_inputFrame->m_height[Y_COMP], false, m_inputFrame->m_colorSpace, m_inputFrame->m_colorPrimaries, m_inputFrame->m_chromaFormat, output->m_sampleRange, output->m_bitDepthComp[Y_COMP], output->m_isInterlaced, m_inputFrame->m_transferFunction, m_inputFrame->m_systemGamma);
  m_convertFrameStore->clear();
    
  if (m_cropOffsetLeft != 0 || m_cropOffsetTop != 0 || m_cropOffsetRight != 0 || m_cropOffsetBottom != 0) {
    m_cropWidth = m_width  = m_inputFrame->m_width[Y_COMP]  - m_cropOffsetLeft + m_cropOffsetRight;
    m_cropHeight = m_height = m_inputFrame->m_height[Y_COMP] - m_cropOffsetTop  + m_cropOffsetBottom;
    m_croppedFrameStore  = new Frame(m_width, m_height, m_inputFrame->m_isFloat, m_inputFrame->m_colorSpace, m_inputFrame->m_colorPrimaries, m_inputFrame->m_chromaFormat, m_inputFrame->m_sampleRange, m_inputFrame->m_bitDepthComp[Y_COMP], m_inputFrame->m_isInterlaced, m_inputFrame->m_transferFunction, m_inputFrame->m_systemGamma);
    m_croppedFrameStore->clear();
  }
  else {
    m_cropWidth = m_width  = m_inputFrame->m_width[Y_COMP];
    m_cropHeight = m_height = m_inputFrame->m_height[Y_COMP];
  }

  // Output (transfer function or denormalization). Since we don't support scaling, lets reset the width and height here.
  //m_outputFile->m_format.m_height[Y_COMP] = output->m_height[Y_COMP] = m_height;
  //m_outputFile->m_format.m_width [Y_COMP] = output->m_width [Y_COMP] = m_width;
  m_outputFile->m_format.m_height[Y_COMP] = output->m_height[Y_COMP];
  m_outputFile->m_format.m_width [Y_COMP] = output->m_width [Y_COMP];
  
  // Create output file
  m_outputFrame = hdrtoolslib::Output::create(m_outputFile, output);

  hdrtoolslib::ChromaFormat chromaFormat = (m_inputFrame->m_colorPrimaries != output->m_colorPrimaries || m_inputFrame->m_colorSpace == hdrtoolslib::CM_RGB)? hdrtoolslib::CF_444 : (input->m_chromaFormat == hdrtoolslib::CF_400 ? hdrtoolslib::CF_400 : output->m_chromaFormat);

  m_oFrameStore  = new Frame(output->m_width[Y_COMP], output->m_height[Y_COMP], output->m_isFloat, output->m_colorSpace, output->m_colorPrimaries, output->m_chromaFormat, output->m_sampleRange, output->m_bitDepthComp[Y_COMP], output->m_isInterlaced, output->m_transferFunction, output->m_systemGamma);
  m_oFrameStore->clear();
  
  // initiate the color transform process. Maybe we can move this in the process section though.
  // This does not currently handle YCbCr to YCbCr conversion cleanly (goes first to RGB which is not correct). Needs to be fixed.
  using ::hdrtoolslib::CM_YCbCr;
  using ::hdrtoolslib::ColorTransform;
  using ::hdrtoolslib::Convert;
  using ::hdrtoolslib::CM_RGB;
  inputParams->m_ctParams.m_max = inputParams->m_outNormalScale;
    
  m_convertBitDepth = Convert::create(&m_iFrameStore->m_format, &m_convertFrameStore->m_format, &inputParams->m_cvParams);
    
  m_frameScale   = hdrtoolslib::FrameScale::create(m_width, m_height, output->m_width[Y_COMP], output->m_height[Y_COMP], &inputParams->m_fsParams, inputParams->m_chromaDownsampleFilter, output->m_chromaLocation[hdrtoolslib::FP_FRAME], inputParams->m_useMinMax);
  
  m_scaledFrame  = new Frame(output->m_width[Y_COMP], output->m_height[Y_COMP], false, m_inputFrame->m_colorSpace, m_inputFrame->m_colorPrimaries, chromaFormat, output->m_sampleRange, output->m_bitDepthComp[Y_COMP], output->m_isInterlaced, hdrtoolslib::TF_NORMAL, 1.0);

  
}

//-----------------------------------------------------------------------------
// main filtering function
//-----------------------------------------------------------------------------

void HDRConvertScale::process( ProjectParameters *inputParams ) {
  int frameNumber;
  int iCurrentFrameToProcess = 0;
  float fDistance0 = ( inputParams->m_source.m_frameRate == 0.0f || inputParams->m_output.m_frameRate == 0.0f ) ? 1.0F : inputParams->m_source.m_frameRate / inputParams->m_output.m_frameRate;

  //FrameFormat   *output = &inputParams->m_output;
  //hdrtoolslib::FrameFormat   *input  = &inputParams->m_source;

  clock_t clk;  
  bool errorRead = false;
  
  hdrtoolslib::Frame *currentFrame = NULL;

  // Now process all frames
  for (frameNumber = 0; frameNumber < inputParams->m_numberOfFrames; frameNumber ++) {
    clk = clock();
    iCurrentFrameToProcess = int(frameNumber * fDistance0);

    // read frames
    m_iFrameStore->m_frameNo = frameNumber;
    if (m_inputFrame->readOneFrame(m_inputFile, iCurrentFrameToProcess, m_inputFile->m_fileHeader, m_startFrame) == true) {
      // Now copy input frame buffer to processing frame buffer for any subsequent processing
      
      m_inputFrame->copyFrame(m_iFrameStore);
    }
    else {
      inputParams->m_numberOfFrames = frameNumber;
      errorRead = true;
      break;
    }
    // optional forced clipping of the data given their defined range. This is done here without consideration of the upconversion process.
    if (inputParams->m_forceClipping == 1)
      m_iFrameStore->clipRange();
    
    if (errorRead == true) {
      break;
    }
    else if (inputParams->m_silentMode == false) {
      printf("%05d ", frameNumber );
    }
    
    currentFrame = m_iFrameStore;
    if (m_croppedFrameStore != NULL) {
      m_croppedFrameStore->copy(m_iFrameStore, m_cropOffsetLeft, m_cropOffsetTop, m_iFrameStore->m_width[hdrtoolslib::Y_COMP] + m_cropOffsetRight, m_iFrameStore->m_height[hdrtoolslib::Y_COMP] + m_cropOffsetBottom, 0, 0);

      currentFrame = m_croppedFrameStore;
    }
    
    // Convert to different bitdept if needed 
    m_convertBitDepth->process(m_convertFrameStore, currentFrame);
    currentFrame = m_convertFrameStore;

    m_frameScale->process(m_scaledFrame, currentFrame);

    // frame output
    m_outputFrame->copyFrame(m_scaledFrame);
    m_outputFrame->writeOneFrame(m_outputFile, frameNumber, m_outputFile->m_fileHeader, 0);
    
    clk = clock() - clk;
    if (inputParams->m_silentMode == false){
      printf("%7.3f", 1.0 * clk / CLOCKS_PER_SEC);
      printf("\n");
      fflush(stdout);
    }
    else {
      printf("Processing Frame : %d\r", frameNumber);
      fflush(stdout);
    }
  } //end for frameNumber
}


//-----------------------------------------------------------------------------
// footer output
//-----------------------------------------------------------------------------

void HDRConvertScale::outputFooter(ProjectParameters *inputParams) {
  clock_t clk = clock();
  FILE* f = hdrtoolslib::IOFunctions::openFile(inputParams->m_logFile, "at");

  if (f != NULL) {
    fprintf(f, "%s ", inputParams->m_inputFile.m_fName);
    fprintf(f, "%s ", inputParams->m_outputFile.m_fName);
    fprintf(f, "%d ", inputParams->m_numberOfFrames);
    fprintf(f, "%f \n", 1.0 * clk / inputParams->m_numberOfFrames / CLOCKS_PER_SEC);
  }

  printf("\n================================================================================================================\n");
  printf("Total of %d frames processed in %7.3f seconds\n",inputParams->m_numberOfFrames, 1.0 * clk / CLOCKS_PER_SEC);
  printf("Conversion Speed: %3.2ffps\n", (float) inputParams->m_numberOfFrames * CLOCKS_PER_SEC / clk);

  hdrtoolslib::IOFunctions::closeFile(f);
}

