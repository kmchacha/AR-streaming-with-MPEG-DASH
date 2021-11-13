/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Apple Inc.
 * <ORGANIZATION> = Apple Inc.
 * <YEAR> = 2016
 *
 * Copyright (c) 2016, Apple Inc.
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
 * \file ProjectParameters.cpp
 *
 * \brief
 *    ProjectParameters class functions for GamutTest project
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "ProjectParameters.H"
#include "Global.H"
#include "IOFunctions.H"
#include "TransferFunction.H"
#include "ConvertColorFormat.H"
#include "AnalyzeGamut.H"

//-----------------------------------------------------------------------------
// Local classes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
ProjectParameters ccParams;
ProjectParameters *pParams = &ccParams;

hdrtoolslib::FrameFormat             *src     = &pParams->m_source;
hdrtoolslib::DistortionParameters    *dParams = &pParams->m_distortionParameters;
hdrtoolslib::VIFParams               *vif     = &dParams->m_VIF;
hdrtoolslib::SSIMParams              *ssim    = &dParams->m_SSIM;
hdrtoolslib::PSNRParams              *snr     = &dParams->m_PSNR;
hdrtoolslib::AnalyzeGamutParameters  *gmt     = &pParams->m_params;

static char defLogFile[]  = "distortion.txt";
static char histFile[]     = "histFile.txt";
static char xyYhistFile[]  = "xyYhistFile.txt";
static char uvYhistFile[]  = "uvYhistFile.txt";

hdrtoolslib::StringParameter stringParameterList[] = {
  { "VideoFile",           pParams->m_inputFile.m_fName,           NULL, "Input file name"              },
  { "LogFile",             pParams->m_logFile,               defLogFile, "Output Log file name"         },
  { "HistFile",            gmt->m_histFile,                    histFile, "Histogram file name"          },
  { "xyHistFile",          gmt->m_xyYhistFile,              xyYhistFile, "xyY 2D Histogram file name"   },
  { "upvpHistFile",        gmt->m_uvYhistFile,              uvYhistFile, "Yu'v' 2D Histogram file name" },
  { "",                    NULL,                                   NULL, "String Termination entry"     }
};

hdrtoolslib::IntegerParameter intParameterList[] = {
  { "Width",                   &src->m_width[0],                                               176,                        0,                       65536,    "Input source width"                   },
  { "Height",                  &src->m_height[0],                                              144,                        0,                       65536,    "Input source height"                  },
  { "ChromaFormat",           (int *) &src->m_chromaFormat,                    hdrtoolslib::CF_420,      hdrtoolslib::CF_400,         hdrtoolslib::CF_444,    "Input Chroma Format"                  },
  { "FourCCCode",             (int *) &src->m_pixelFormat,                    hdrtoolslib::PF_UYVY,     hdrtoolslib::PF_UYVY,   hdrtoolslib::PF_TOTAL - 1,    "Input Pixel Format"                   },
  { "BitDepthCmp0",            &src->m_bitDepthComp[hdrtoolslib::Y_COMP],                        8,                        8,                          16,    "Input Bitdepth Cmp0"                  },
  { "BitDepthCmp1",            &src->m_bitDepthComp[hdrtoolslib::U_COMP],                        8,                        8,                          16,    "Input Bitdepth Cmp1"                  },
  { "BitDepthCmp2",            &src->m_bitDepthComp[hdrtoolslib::V_COMP],                        8,                        8,                          16,    "Input Bitdepth Cmp2"                  },
  { "ColorSpace",             (int *) &src->m_colorSpace,                    hdrtoolslib::CM_YCbCr,    hdrtoolslib::CM_YCbCr,   hdrtoolslib::CM_TOTAL - 1,    "Input Color Space"                    },
  { "ColorPrimaries",         (int *) &src->m_colorPrimaries,                  hdrtoolslib::CP_709,      hdrtoolslib::CP_709,   hdrtoolslib::CP_TOTAL - 1,    "Input Color Primaries"                },
  { "TransferFunction",       (int *) &src->m_transferFunction,               hdrtoolslib::TF_NULL,     hdrtoolslib::TF_NULL,   hdrtoolslib::TF_TOTAL - 1,    "Input Transfer Function"              },
  { "SampleRange",            (int *) &src->m_sampleRange,                hdrtoolslib::SR_STANDARD, hdrtoolslib::SR_STANDARD,   hdrtoolslib::SR_TOTAL - 1,    "Input Sample Range"                   },
  { "FileHeader",              &pParams->m_inputFile.m_fileHeader,                               0,                        0,                     INT_INF,    "Input Header (bytes)"                 },
  { "StartFrame",              &pParams->m_inputFile.m_startFrame,                               0,                        0,                     INT_INF,    "Input Start Frame"                    },
  { "FrameSkip",               &pParams->m_frameSkip,                                            0,                        0,                     INT_INF,    "Input Frame Skipping"                 },
  { "CropOffsetLeft",          &pParams->m_cropOffsetLeft,                                       0,                   -65536,                       65536,    "Input Crop Offset Left position"      },
  { "CropOffsetTop",           &pParams->m_cropOffsetTop,                                        0,                   -65536,                       65536,    "Input Crop Offset Top position"       },
  { "CropOffsetRight",         &pParams->m_cropOffsetRight,                                      0,                   -65536,                       65536,    "Input Crop Offset Right position"     },
  { "CropOffsetBottom",        &pParams->m_cropOffsetBottom,                                     0,                   -65536,                       65536,    "Input Crop Offset Bottom position"    },
  
  { "NumberOfFrames",          &pParams->m_numberOfFrames,                                       1,                        1,                     INT_INF,    "Number of Frames to process"          },
  // SSIM parameters
  { "SSIMBlockDistance",       &ssim->m_blockDistance,                                           1,                        1,                         128,    "Block Distance for SSIM computation"  },
  { "SSIMBlockSizeX",          &ssim->m_blockSizeX,                                              4,                        4,                         128,    "Block Width for SSIM computation"     },
  { "SSIMBlockSizeY",          &ssim->m_blockSizeY,                                              4,                        4,                         128,    "Block Height for SSIM computation"    },
  
  { "TFPSNRDistortion",       (int *) &snr->m_tfDistortion,               hdrtoolslib::DIF_PQPH10K,  hdrtoolslib::DIF_PQPH10K, hdrtoolslib::DIF_TOTAL - 1,    "TF for tPSNR and other metrics"       },
  { "TFPSNRDistortion",       (int *) &ssim->m_tfDistortion,              hdrtoolslib::DIF_PQPH10K,  hdrtoolslib::DIF_PQPH10K, hdrtoolslib::DIF_TOTAL - 1,    "TF for tSSIM and other metrics"       },
  { "DeltaEPointsEnable",      &dParams->m_deltaEPointsEnable,                                   1,                         1,                          7,    "Delta E points to Enable"             },
  { "RPSNRBlockDistanceX",     &snr->m_rPSNROverlapX,                                            4,                         1,                      65536,    "Block Horz. Distance for rPSNR"       },
  { "RPSNRBlockDistanceY",     &snr->m_rPSNROverlapY,                                            4,                         1,                      65536,    "Block Vert. Distance for rPSNR"       },
  { "RPSNRBlockSizeX",         &snr->m_rPSNRBlockSizeX,                                          4,                         4,                      65536,    "Block Width for rPSNR"                },
  { "RPSNRBlockSizeY",         &snr->m_rPSNRBlockSizeY,                                          4,                         4,                      65536,    "Block Height for rPSNR"               },
  // FIV parameters
  { "VIFBitDepthY",            &vif->m_vifBitDepth,                                              8,                         8,                         16,    "VIF YUV Bitdepth control param"       },
  { "HistogramBinsMod3",       &gmt->m_histBins,                                              2000,                         1,                       2000,    "Histogram Bins (module 3)"            },
  { "Histogram2DBins",         &gmt->m_histBin2D,                                              100,                         1,                       2000,    "2D Histogram Bins"                    },

  { "",                         NULL,                                                            0,                         0,                          0,    "Integer Termination entry"            }
};

hdrtoolslib::BoolParameter boolParameterList[] = {
  { "Interleaved",               &pParams->m_inputFile.m_isInterleaved,             false,  false,  true,    "Input Interleaved Source"                      },
  { "Interlaced",                &src->m_isInterlaced,                              false,  false,  true,    "Input Interleaved Source"                      },
  { "EnablexyYHist",             &gmt->m_enableHistxyY,                             false,  false,  true,    "xyY Histogram"                                 },
  { "EnableYupvpHist",           &gmt->m_enableHistYupvp,                           false,  false,  true,    "Yu'v' Histogram"                               },
  { "EnableLuminanceStats",      &gmt->m_enableLumStats,                            false,  false,  true,    "Luminance Statistics"                          },
  { "SilentMode",                &pParams->m_silentMode,                             true,  false,  true,    "Silent mode"                                   },
  { "",                          NULL,                                              false,  false, false,    "Boolean Termination entry"                     }
};

hdrtoolslib::DoubleParameter doubleParameterList[] = {
  { "MaxSampleValue",        &dParams->m_maxSampleValue,                             10000.0,      0.001, 99999999.0,    "Maximum sample value for floats"   },
  { "",                      NULL,                                                       0.0,        0.0,        0.0,    "Double Termination entry"          }
};

hdrtoolslib::FloatParameter floatParameterList[] = {
  { "FrameRate",            &src->m_frameRate,                        24.00F,    0.01F,     120.00F,    "Input Source Frame Rate"                    },
  { "NormalizationScale",   &gmt->m_normalScale,                   10000.00F,    0.01F, 1000000.00F,    "Input Normalization Scale for Linear Data"  },

  { "",                      NULL,                                     0.00F,    0.00F,       0.00F,    "Float Termination entry"                    }
};

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------
ProjectParameters::ProjectParameters()
{
  for (int i = hdrtoolslib::DIST_NULL; i < hdrtoolslib::DIST_METRICS; i++) {
    m_enableMetric[i] = false;
  }
}

void ProjectParameters::refresh() {
  int i;
  using ::hdrtoolslib::ZERO;
  for (i = ZERO; intParameterList[i].ptr != NULL; i++)
    *(intParameterList[i].ptr) = intParameterList[i].default_val;

  for (i = ZERO; boolParameterList[i].ptr != NULL; i++)
    *(boolParameterList[i].ptr) = boolParameterList[i].default_val;

  for (i = ZERO; floatParameterList[i].ptr != NULL; i++)
    *(floatParameterList[i].ptr) = floatParameterList[i].default_val;

  for (i = ZERO; doubleParameterList[i].ptr != NULL; i++) {
    *(doubleParameterList[i].ptr) = doubleParameterList[i].default_val;  
  }
  
  for (i = ZERO; stringParameterList[i].ptr != NULL; i++) {
    if (stringParameterList[i].default_val != NULL)
      strcpy(stringParameterList[i].ptr, stringParameterList[i].default_val);
    else
      stringParameterList[i].ptr[0] = 0;    
  }
}

void ProjectParameters::update() {
  using ::hdrtoolslib::IOFunctions;
  IOFunctions::parseVideoType   (&m_inputFile);
  IOFunctions::parseFrameFormat (&m_inputFile);
  
  // Read resolution from file name
  if (m_source.m_width[0] == 0 || m_source.m_height[0] == 0) {
    if (IOFunctions::parseFrameSize (&m_inputFile, m_source.m_width, m_source.m_height, &(m_source.m_frameRate)) == 0) {
      fprintf(stderr, "File name does not contain resolution information.\n");
      exit(EXIT_FAILURE);
    }
  }
    
  setupFormat(&m_source);
  
  if (m_numberOfFrames == -1)  {
    IOFunctions::openFile (&m_inputFile);
    getNumberOfFrames (&m_source, &m_inputFile, m_inputFile.m_startFrame);
    IOFunctions::closeFile (&m_inputFile);
  }

  m_inputFile.m_format = m_source;
  
}



//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
