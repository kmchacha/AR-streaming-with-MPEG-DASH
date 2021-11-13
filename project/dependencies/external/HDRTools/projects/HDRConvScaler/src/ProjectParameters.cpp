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
 * \file ProjectParameters.cpp
 *
 * \brief
 *    ProjectParameters class functions for HDRConvScaler project
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
#include "ScaleFilter.H"

//-----------------------------------------------------------------------------
// Local classes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
ProjectParameters ccParams;
ProjectParameters *pParams = &ccParams;

hdrtoolslib::FrameFormat          *src  = &pParams->m_source;
hdrtoolslib::FrameFormat          *out  = &pParams->m_output;
hdrtoolslib::ColorTransformParams *ctp  = &pParams->m_ctParams;
hdrtoolslib::FrameScaleParams     *fsp  = &pParams->m_fsParams;

static char defLogFile[]  = "log.txt";
static char def_out_file[] = "output_conv.yuv";

hdrtoolslib::StringParameter stringParameterList[] = {
  { "SourceFile",          pParams->m_inputFile.m_fName,              NULL, "Source file name"                            },
  { "OutputFile",          pParams->m_outputFile.m_fName,     def_out_file, "Output file name"                            },
  { "LogFile",             pParams->m_logFile,                  defLogFile, "Output Log file name"                        },
  { "YAdjustModelFile",    ctp->m_yAdjustModelFile,                   NULL, "Luma adjustment (2nd order) model file name" },
  { "",                    NULL,                                      NULL, "String Termination entry"                    }
};

hdrtoolslib::IntegerParameter intParameterList[] = {
  { "SourceWidth",             &src->m_width [hdrtoolslib::Y_COMP],                                        176,                        0,                       65536,    "Input source width"                       },
  { "SourceHeight",            &src->m_height[hdrtoolslib::Y_COMP],                                        144,                        0,                       65536,    "Input source height"                      },
  { "SourceChromaFormat",      (int *) &src->m_chromaFormat,                               hdrtoolslib::CF_420,      hdrtoolslib::CF_400,         hdrtoolslib::CF_444,    "Source Chroma Format"                     },
  { "SourceChromaLocationTop", (int *) &src->m_chromaLocation[hdrtoolslib::FP_TOP],       hdrtoolslib::CL_ZERO,     hdrtoolslib::CL_ZERO,   hdrtoolslib::CL_TOTAL - 1,    "Source Chroma Location Type (Top/Frame)"  },
  { "SourceChromaLocationBottom", (int *) &src->m_chromaLocation[hdrtoolslib::FP_BOTTOM], hdrtoolslib::CL_ZERO,     hdrtoolslib::CL_ZERO,   hdrtoolslib::CL_TOTAL - 1,    "Source Chroma Location Type (Bottom)"     },
  { "SourceFourCCCode",        (int *) &src->m_pixelFormat,                               hdrtoolslib::PF_UYVY,     hdrtoolslib::PF_UYVY,   hdrtoolslib::PF_TOTAL - 1,    "Source Pixel Format"                      },
  { "SourceBitDepthCmp0",      &src->m_bitDepthComp[hdrtoolslib::Y_COMP],                                    8,                        8,                          16,    "Source Bitdepth Cmp0"                     },
  { "SourceBitDepthCmp1",      &src->m_bitDepthComp[hdrtoolslib::U_COMP],                                    8,                        8,                          16,    "Source Bitdepth Cmp1"                     },
  { "SourceBitDepthCmp2",      &src->m_bitDepthComp[hdrtoolslib::V_COMP],                                    8,                        8,                          16,    "Source Bitdepth Cmp2"                     },
  { "SourceColorSpace",        (int *) &src->m_colorSpace,                               hdrtoolslib::CM_YCbCr,    hdrtoolslib::CM_YCbCr,   hdrtoolslib::CM_TOTAL - 1,    "Source Color Space"                       },
  { "SourceColorPrimaries",    (int *) &src->m_colorPrimaries,                             hdrtoolslib::CP_709,      hdrtoolslib::CP_709,   hdrtoolslib::CP_TOTAL - 1,    "Source Color Primaries"                   },
  { "SourceTransferFunction",  (int *) &src->m_transferFunction,                          hdrtoolslib::TF_NULL,     hdrtoolslib::TF_NULL,   hdrtoolslib::TF_TOTAL - 1,    "Source Transfer Function"                 },
  { "SourceSampleRange",       (int *) &src->m_sampleRange,                           hdrtoolslib::SR_STANDARD, hdrtoolslib::SR_STANDARD,   hdrtoolslib::SR_TOTAL - 1,    "Source Sample Range"                      },
 
  
  { "CropOffsetLeft",          &pParams->m_cropOffsetLeft,                                                   0,                   -65536,                       65536,    "Crop Offset Left position"                },
  { "CropOffsetTop",           &pParams->m_cropOffsetTop,                                                    0,                   -65536,                       65536,    "Crop Offset Top position"                 },
  { "CropOffsetRight",         &pParams->m_cropOffsetRight,                                                  0,                   -65536,                       65536,    "Crop Offset Right position"               },
  { "CropOffsetBottom",        &pParams->m_cropOffsetBottom,                                                 0,                   -65536,                       65536,    "Crop Offset Bottom position"              },

  
  // Currently we do not need rescaling so we can keep these parameters disabled (could be added in the future if needed).
  { "OutputWidth",             &out->m_width[hdrtoolslib::Y_COMP],                                         176,                        0,                       65536,    "Output/Processing width"                  },
  { "OutputHeight",            &out->m_height[hdrtoolslib::Y_COMP],                                        144,                        0,                       65536,    "Output/Processing height"                 },
  { "OutputChromaFormat",      (int *) &out->m_chromaFormat,                               hdrtoolslib::CF_420,      hdrtoolslib::CF_400,         hdrtoolslib::CF_444,    "Output Chroma Format"                     },
  { "OutputChromaLocationTop", (int *) &out->m_chromaLocation[hdrtoolslib::FP_TOP],       hdrtoolslib::CL_ZERO,     hdrtoolslib::CL_ZERO,   hdrtoolslib::CL_TOTAL - 1,    "Output Chroma Location Type (Top/Frame)"  },
  { "OutputChromaLocationBottom", (int *) &out->m_chromaLocation[hdrtoolslib::FP_BOTTOM], hdrtoolslib::CL_ZERO,     hdrtoolslib::CL_ZERO,   hdrtoolslib::CL_TOTAL - 1,    "Output Chroma Location Type (Bottom)"     },
  { "OutputFourCCCode",        (int *) &out->m_pixelFormat,                               hdrtoolslib::PF_UYVY,     hdrtoolslib::PF_UYVY,   hdrtoolslib::PF_TOTAL - 1,    "Output Pixel Format"                      },
  { "OutputBitDepthCmp0",      &out->m_bitDepthComp[hdrtoolslib::Y_COMP],                                    8,                        8,                          16,    "Output Bitdepth Cmp0"                     },
  { "OutputBitDepthCmp1",      &out->m_bitDepthComp[hdrtoolslib::U_COMP],                                    8,                        8,                          16,    "Output Bitdepth Cmp1"                     },
  { "OutputBitDepthCmp2",      &out->m_bitDepthComp[hdrtoolslib::V_COMP],                                    8,                        8,                          16,    "Output Bitdepth Cmp2"                     },
  { "OutputColorSpace",        (int *) &out->m_colorSpace,                               hdrtoolslib::CM_YCbCr,    hdrtoolslib::CM_YCbCr,   hdrtoolslib::CM_TOTAL - 1,    "Output Color Space"                       },
  { "OutputColorPrimaries",    (int *) &out->m_colorPrimaries,                             hdrtoolslib::CP_709,      hdrtoolslib::CP_709,   hdrtoolslib::CP_TOTAL - 1,    "Output Color Primaries"                   },
  { "OutputTransferFunction",  (int *) &out->m_transferFunction,                            hdrtoolslib::TF_PQ,     hdrtoolslib::TF_NULL,   hdrtoolslib::TF_TOTAL - 1,    "Output Transfer Function"                 },
  { "OutputSampleRange",       (int *) &out->m_sampleRange,                           hdrtoolslib::SR_STANDARD, hdrtoolslib::SR_STANDARD,   hdrtoolslib::SR_TOTAL - 1,    "Output Sample Range"                      },
  //! Various Params
  { "NumberOfFrames",          &pParams->m_numberOfFrames,                                                   1,                        1,                     INT_INF,    "Number of Frames to process"              },
  { "InputFileHeader",         &pParams->m_inputFile.m_fileHeader,                                           0,                        0,                     INT_INF,    "Source Header (bytes)"                    },
  { "StartFrame",              &pParams->m_inputFile.m_startFrame,                                           0,                        0,                     INT_INF,    "Source Start Frame"                       },
  { "FrameSkip",               &pParams->m_frameSkip,                                                        0,                        0,                     INT_INF,    "Source Frame Skipping"                    },
  { "ScalingMode",             (int *) &fsp->m_mode,                                        hdrtoolslib::SC_NN,       hdrtoolslib::SC_NN,   hdrtoolslib::SC_TOTAL - 1,    "Scaling Filter Mode"                      },
  { "LanczosLobes",            &fsp->m_lanczosLobes,                                                         6,                        1,                          16,    "Lanczos Lobes"                            },

  { "ChromaDownsampleFilter",  &pParams->m_chromaDownsampleFilter,                          hdrtoolslib::DF_F0,       hdrtoolslib::DF_NN,   hdrtoolslib::DF_TOTAL - 1,    "Chroma Downsampling Filter"               },
  { "ChromaUpsampleFilter",    &pParams->m_chromaUpsampleFilter,                            hdrtoolslib::UF_NN,       hdrtoolslib::UF_NN,   hdrtoolslib::UF_TOTAL - 1,    "Chroma Upsampling Filter"                 },
  { "AddNoise",                &pParams->m_addNoise,                                   hdrtoolslib::NOISE_NULL,  hdrtoolslib::NOISE_NULL,   hdrtoolslib::NOISE_NORMAL,    "Add noise to the input signal"            },
  { "ClosedLoopConversion",    (int *) &pParams->m_closedLoopConversion,                 hdrtoolslib::CLT_NULL,    hdrtoolslib::CLT_NULL,  hdrtoolslib::CLT_TOTAL - 1,    "Enable Closed Loop Conversion"            },
  { "ClosedLoopIterations",    &pParams->m_closedLoopIterations,                                            10,                        1,                          50,    "Number of Closed Loop Iterations"         },
  { "SourceConstantLuminance", &src->m_iConstantLuminance,                                                   0,                        0,                           3,    "Constant Luminance Source"                },
  { "OutputConstantLuminance", &out->m_iConstantLuminance,                                                   0,                        0,                           3,    "Constant Luminance Output"                },
  { "UseMinMaxFiltering",      &pParams->m_useMinMax,                                                        0,                        0,                           3,    "Use Min/Max Filtering"                    },
  { "HighPrecisionColor",      &pParams->m_useHighPrecisionTransform,                                        0,                        0,                           2,    "High Precision Color Mode "               },
  { "UseAdaptiveUpsampling",   &pParams->m_useAdaptiveUpsampling,                        hdrtoolslib::ADF_NULL,    hdrtoolslib::ADF_NULL,  hdrtoolslib::ADF_TOTAL - 1,    "Use Adaptive Upsampler"                   },
  { "UseAdaptiveDownsampling", &pParams->m_useAdaptiveDownsampling,                      hdrtoolslib::ADF_NULL,    hdrtoolslib::ADF_NULL,  hdrtoolslib::ADF_TOTAL - 1,    "Use Adaptive Downsampler"                 },

  { "",                        NULL,                                                                         0,                        0,                           0,    "Integer Termination entry"                }
};

hdrtoolslib::BoolParameter boolParameterList[] = {
  { "SourceInterleaved",          &pParams->m_inputFile.m_isInterleaved,      false,       false,         true,    "Interleaved Source"                          },
  { "SourceInterlaced",           &src->m_isInterlaced,                       false,       false,         true,    "Interlaced Source"                           },
  { "OutputInterleaved",          &pParams->m_outputFile.m_isInterleaved,     false,       false,         true,    "Interleaved Output"                          },
  { "SilentMode",                 &pParams->m_silentMode,                     false,       false,         true,    "Silent mode"                                 },
  { "USeSingleTransferStep",      &pParams->m_useSingleTransferStep,          false,       false,         true,    "Use a single transfer step"                  },
  { "TransformPrecision",         &pParams->m_transformPrecision,              true,       false,         true,    "Color transform precision"                   },
  { "SetOutputSinglePrec",        &pParams->m_outputSinglePrecision,          false,       false,         true,    "OpenEXR Output data precision"               },
  { "SetOutputEXRRounding",       &out->m_useFloatRound,                      false,       false,         true,    "OpenEXR Output with Rounding Enabled"        },
  { "FilterUsingFloats",          &pParams->m_filterInFloat,                  false,       false,         true,    "Perform Filtering using Floats "             },
  { "LinearDownConversion",       &pParams->m_linearDownConversion,           false,       false,         true,    "Perform linear downconversion to 420"        },
  { "RGBDownConversion",          &pParams->m_rgbDownConversion,              false,       false,         true,    "Perform downconversion in RGB"               },
  { "UseChromaDeblocking",        &pParams->m_bUseChromaDeblocking,           false,       false,         true,    "Deblock Chroma before Upconversion"          },
  { "UseWienerFiltering",         &pParams->m_bUseWienerFiltering,            false,       false,         true,    "Wiener Filtering before conversion"          },
  { "Use2DSepFiltering",          &pParams->m_bUse2DSepFiltering,             false,       false,         true,    "Separable Filtering before conversion"       },
  { "2DSepFilteringMode",         &pParams->m_b2DSepMode,                     false,       false,         true,    "Edge Adaptation On or OFF"                   },
  { "EnableTFunctionLUT",         &pParams->m_enableTFunctionLUT,             false,       false,         true,    "Enable TF LUT for some operations"           },
  { "EnableTFunctionLUT",         &pParams->m_enableTFLUTs,                   false,       false,         true,    "Enable TF LUTs for closed loop conversions"  },
  { "EnableTFunctionLUT",         &ctp->m_enableLUTs,                         false,       false,         true,    "Enable TF LUTs for closed loop conversions"  },
  { "EnableTFDerivLUT",           &ctp->m_enableTFDerivLUTs,                  false,       false,         true,    "Enable TF derivative LUTs"                   },
  { "",                           NULL,                                       false,       false,        false,    "Boolean Termination entry"                   }
};

hdrtoolslib::DoubleParameter doubleParameterList[] = {
  { "NoiseVariance",          &pParams->m_noiseVariance,                     1.0,         0.0,      20000.0,    "Additive Noise variance"                  },
  { "NoiseMean",              &pParams->m_noiseMean,                         0.0,         0.0,      20000.0,    "Additive Noise mean"                      },
  { "",                       NULL,                                          0.0,         0.0,          0.0,    "Double Termination entry"                 }
};


hdrtoolslib::FloatParameter floatParameterList[] = {
  { "SourceRate",                  &src->m_frameRate,                       24.00F,       0.01F,      120.00F,    "Input Source Frame Rate"                     },
  { "SourceSystemGamma",           &src->m_systemGamma,                      1.00F,       0.00F,       10.00F,    "Overall System gamma for Hybrid Gamma TF"    },
  { "OutputRate",                  &out->m_frameRate,                       24.00F,       0.01F,      120.00F,    "Image Output Frame Rate"                     },
  { "OutputSystemGamma",           &out->m_systemGamma,                      1.00F,       0.00F,       10.00F,    "Overall System gamma for Hybrid Gamma TF"    },
  { "SourceNormalizationScale",    &pParams->m_srcNormalScale,           10000.00F,       0.01F,  1000000.00F,    "Source Normalization Scale for Linear Data"  },
  { "OutputNormalizationScale",    &pParams->m_outNormalScale,           10000.00F,       0.01F,  1000000.00F,    "Output Normalization Scale for Linear Data"  },
  { "SourceTransferMinBrightness", &pParams->m_srcMinValue,                  0.00F,       0.00F,  1000000.00F,    "Source Transfer Function Minimum Brightness" },
  { "SourceTransferMaxBrightness", &pParams->m_srcMaxValue,              10000.00F,       0.00F,  1000000.00F,    "Source Transfer Function Maximum Brightness" },
  { "OutputTransferMinBrightness", &pParams->m_outMinValue,                  0.00F,       0.00F,  1000000.00F,    "Output Transfer Function Minimum Brightness" },
  { "OutputTransferMaxBrightness", &pParams->m_outMaxValue,              10000.00F,       0.00F,  1000000.00F,    "Output Transfer Function Maximum Brightness" },
  { "",                            NULL,                                     0.00F,       0.00F,        0.00F,    "Float Termination entry"                     }
};


//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------


void ProjectParameters::refresh() {
  int i;

  using ::hdrtoolslib::ZERO;
  for (i = ZERO; intParameterList[i].ptr != NULL; i++) {
    *(intParameterList[i].ptr) = intParameterList[i].default_val;
  }

  for (i = ZERO; boolParameterList[i].ptr != NULL; i++)
    *(boolParameterList[i].ptr) = boolParameterList[i].default_val;

  for (i = ZERO; floatParameterList[i].ptr != NULL; i++)
    *(floatParameterList[i].ptr) = floatParameterList[i].default_val;

  for (i = ZERO; doubleParameterList[i].ptr != NULL; i++)
    *(doubleParameterList[i].ptr) = doubleParameterList[i].default_val;  

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
  IOFunctions::parseVideoType   (&m_outputFile);
  IOFunctions::parseFrameFormat (&m_outputFile);
  
  // Read resolution from file name
  if (m_source.m_width[0] == 0 || m_source.m_height[0] == 0) {
    if (IOFunctions::parseFrameSize (&m_inputFile, m_source.m_width, m_source.m_height, &(m_source.m_frameRate)) == 0) {
      fprintf(stderr, "File name does not contain resolution information.\n");
      exit(EXIT_FAILURE);
    }
  }
  
  // currently lets force the same resolution
  //m_output.m_width[0]  = m_source.m_width[0] - m_cropOffsetLeft + m_cropOffsetRight;
  //m_output.m_height[0] = m_source.m_height[0] - m_cropOffsetTop + m_cropOffsetBottom;
   
  //m_output.m_width[0]   = ceilingRescale(m_output.m_width[0], 3);
  //m_output.m_height[0]  = ceilingRescale(m_output.m_height[0],3);
  
  setupFormat(&m_source);
  setupFormat(&m_output);
  
  
  if (m_outputSinglePrecision == 0) {
    m_output.m_pixelType[hdrtoolslib::Y_COMP] = m_output.m_pixelType[hdrtoolslib::U_COMP] =
    m_output.m_pixelType[hdrtoolslib::V_COMP] = m_output.m_pixelType[hdrtoolslib::A_COMP] = hdrtoolslib::HALF;
  }
  else {
    m_output.m_pixelType[hdrtoolslib::Y_COMP] = m_output.m_pixelType[hdrtoolslib::U_COMP] =
    m_output.m_pixelType[hdrtoolslib::V_COMP] = m_output.m_pixelType[hdrtoolslib::A_COMP] = hdrtoolslib::FLOAT;
  }
  
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
