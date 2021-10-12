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
 *    ProjectParameters class functions for HDRConvert project
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *     - Dimitri Podborski
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <fstream>
#include <algorithm>
#include <cctype>

#include "ProjectParameters.H"
#include "Global.H"
#include "IOFunctions.H"
#include "TransferFunction.H"
#include "ScaleFilter.H"
#include "Convert.H"

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
hdrtoolslib::ConvertParams        *cvp  = &pParams->m_cvParams;
hdrtoolslib::ToneMappingParams    *tmp  = &pParams->m_tmParams;
hdrtoolslib::FrameScaleParams     *fsp  = &pParams->m_fsParams;

static char defLogFile[]  = "log.txt";
static char def_out_file[] = "output_conv.yuv";

hdrtoolslib::StringParameter stringParameterList[] = {
  { "SourceFile",          pParams->m_inputFile.m_fName,              NULL, "Source file name"                            },
  { "OutputFile",          pParams->m_outputFile.m_fName,     def_out_file, "Output file name"                            },
  { "LogFile",             pParams->m_logFile,                  defLogFile, "Output Log file name"                        },
  { "YAdjustModelFile",    ctp->m_yAdjustModelFile,                   NULL, "Luma adjustment (2nd order) model file name" },
  { "ICCProfilePath",      out->m_strICCProfilePath,                  NULL, "Path to ICC profile file as defined in ISO 15076-1 to be added to the output." },
  { "",                    NULL,                                      NULL, "String Termination entry"                    }
};

hdrtoolslib::IntegerParameter intParameterList[] = {
  { "SourceWidth",             &src->m_width [hdrtoolslib::Y_COMP],                                        176,                        0,                          65536,    "Input source width"                       },
  { "SourceHeight",            &src->m_height[hdrtoolslib::Y_COMP],                                        144,                        0,                          65536,    "Input source height"                      },
  { "SourceChromaFormat",      (int *) &src->m_chromaFormat,                               hdrtoolslib::CF_420,      hdrtoolslib::CF_400,            hdrtoolslib::CF_444,    "Source Chroma Format"                     },
  { "SourceChromaLocationTop", (int *) &src->m_chromaLocation[hdrtoolslib::FP_TOP],       hdrtoolslib::CL_ZERO,     hdrtoolslib::CL_ZERO,      hdrtoolslib::CL_TOTAL - 1,    "Source Chroma Location Type (Top/Frame)"  },
  { "SourceChromaLocationBottom", (int *) &src->m_chromaLocation[hdrtoolslib::FP_BOTTOM], hdrtoolslib::CL_ZERO,     hdrtoolslib::CL_ZERO,      hdrtoolslib::CL_TOTAL - 1,    "Source Chroma Location Type (Bottom)"     },
  { "SourceFourCCCode",        (int *) &src->m_pixelFormat,                               hdrtoolslib::PF_UYVY,     hdrtoolslib::PF_UYVY,      hdrtoolslib::PF_TOTAL - 1,    "Source Pixel Format"                      },
  { "SourceBitDepthCmp0",      &src->m_bitDepthComp[hdrtoolslib::Y_COMP],                                    8,                        8,                             16,    "Source Bitdepth Cmp0"                     },
  { "SourceBitDepthCmp1",      &src->m_bitDepthComp[hdrtoolslib::U_COMP],                                    8,                        8,                             16,    "Source Bitdepth Cmp1"                     },
  { "SourceBitDepthCmp2",      &src->m_bitDepthComp[hdrtoolslib::V_COMP],                                    8,                        8,                             16,    "Source Bitdepth Cmp2"                     },
  { "SourceColorSpace",        (int *) &src->m_colorSpace,                               hdrtoolslib::CM_YCbCr,    hdrtoolslib::CM_YCbCr,      hdrtoolslib::CM_TOTAL - 1,    "Source Color Space"                       },
  { "SourceColorPrimaries",    (int *) &src->m_colorPrimaries,                             hdrtoolslib::CP_709,      hdrtoolslib::CP_709,      hdrtoolslib::CP_TOTAL - 1,    "Source Color Primaries"                   },
  { "SourceTransferFunction",  (int *) &src->m_transferFunction,                          hdrtoolslib::TF_NULL,     hdrtoolslib::TF_NULL,      hdrtoolslib::TF_TOTAL - 1,    "Source Transfer Function"                 },
  { "SourceSampleRange",       (int *) &src->m_sampleRange,                           hdrtoolslib::SR_STANDARD, hdrtoolslib::SR_STANDARD,      hdrtoolslib::SR_TOTAL - 1,    "Source Sample Range"                      },
  { "SourceDisplayAdjustment", (int *) &src->m_displayAdjustment,                         hdrtoolslib::DA_NULL,     hdrtoolslib::DA_NULL,            hdrtoolslib::DA_HLG,    "Source Gamma Display Adjustment"          },
 
  
  { "CropOffsetLeft",          &pParams->m_cropOffsetLeft,                                                   0,                   -65536,                          65536,    "Crop Offset Left position"                },
  { "CropOffsetTop",           &pParams->m_cropOffsetTop,                                                    0,                   -65536,                          65536,    "Crop Offset Top position"                 },
  { "CropOffsetRight",         &pParams->m_cropOffsetRight,                                                  0,                   -65536,                          65536,    "Crop Offset Right position"               },
  { "CropOffsetBottom",        &pParams->m_cropOffsetBottom,                                                 0,                   -65536,                          65536,    "Crop Offset Bottom position"              },

  
  // Currently we do not need rescaling so we can keep these parameters disabled (could be added in the future if needed).
  { "OutputWidth",             &out->m_width[0],                                                             0,                        0,                          65536,    "Output/Processing width"                  },
  { "OutputHeight",            &out->m_height[0],                                                            0,                        0,                          65536,    "Output/Processing height"                 },
  { "OutputChromaFormat",      (int *) &out->m_chromaFormat,                               hdrtoolslib::CF_420,      hdrtoolslib::CF_400,            hdrtoolslib::CF_444,    "Output Chroma Format"                     },
  { "OutputChromaLocationTop", (int *) &out->m_chromaLocation[hdrtoolslib::FP_TOP],       hdrtoolslib::CL_ZERO,     hdrtoolslib::CL_ZERO,      hdrtoolslib::CL_TOTAL - 1,    "Output Chroma Location Type (Top/Frame)"  },
  { "OutputChromaLocationBottom", (int *) &out->m_chromaLocation[hdrtoolslib::FP_BOTTOM], hdrtoolslib::CL_ZERO,     hdrtoolslib::CL_ZERO,      hdrtoolslib::CL_TOTAL - 1,    "Output Chroma Location Type (Bottom)"     },
  { "OutputFourCCCode",        (int *) &out->m_pixelFormat,                               hdrtoolslib::PF_UYVY,     hdrtoolslib::PF_UYVY,      hdrtoolslib::PF_TOTAL - 1,    "Output Pixel Format"                      },
  { "OutputBitDepthCmp0",      &out->m_bitDepthComp[hdrtoolslib::Y_COMP],                                    8,                        8,                             16,    "Output Bitdepth Cmp0"                     },
  { "OutputBitDepthCmp1",      &out->m_bitDepthComp[hdrtoolslib::U_COMP],                                    8,                        8,                             16,    "Output Bitdepth Cmp1"                     },
  { "OutputBitDepthCmp2",      &out->m_bitDepthComp[hdrtoolslib::V_COMP],                                    8,                        8,                             16,    "Output Bitdepth Cmp2"                     },
  { "OutputColorSpace",        (int *) &out->m_colorSpace,                               hdrtoolslib::CM_YCbCr,    hdrtoolslib::CM_YCbCr,      hdrtoolslib::CM_TOTAL - 1,    "Output Color Space"                       },
  { "OutputColorPrimaries",    (int *) &out->m_colorPrimaries,                             hdrtoolslib::CP_709,      hdrtoolslib::CP_709,      hdrtoolslib::CP_TOTAL - 1,    "Output Color Primaries"                   },
  { "OutputTransferFunction",  (int *) &out->m_transferFunction,                            hdrtoolslib::TF_PQ,     hdrtoolslib::TF_NULL,      hdrtoolslib::TF_TOTAL - 1,    "Output Transfer Function"                 },
  { "OutputSampleRange",       (int *) &out->m_sampleRange,                           hdrtoolslib::SR_STANDARD, hdrtoolslib::SR_STANDARD,      hdrtoolslib::SR_TOTAL - 1,    "Output Sample Range"                      },
  { "OutputDisplayAdjustment", (int *) &out->m_displayAdjustment,                         hdrtoolslib::DA_NULL,     hdrtoolslib::DA_NULL,      hdrtoolslib::DA_TOTAL - 1,    "Output Gamma Display Adjustment"          },
  { "OutputDisplayAdjustment", (int *) &ctp->m_displayAdjustment,                         hdrtoolslib::DA_NULL,     hdrtoolslib::DA_NULL,      hdrtoolslib::DA_TOTAL - 1,    "Output Gamma Display Adjustment"          },


  //! Various Params
  { "NumberOfFrames",          &pParams->m_numberOfFrames,                                                   1,                        1,                         INT_INF,    "Number of Frames to process"              },
  { "InputFileHeader",         &pParams->m_inputFile.m_fileHeader,                                           0,                        0,                         INT_INF,    "Source Header (bytes)"                    },
  { "StartFrame",              &pParams->m_inputFile.m_startFrame,                                           0,                        0,                         INT_INF,    "Source Start Frame"                       },
  { "FrameSkip",               &pParams->m_frameSkip,                                                        0,                        0,                         INT_INF,    "Source Frame Skipping"                    },

  { "ScalingMode",             (int *) &fsp->m_mode,                                  hdrtoolslib::SC_BILINEAR,       hdrtoolslib::SC_NN,       hdrtoolslib::SC_TOTAL - 1,    "Scaling Filter Mode"                      },
  { "LanczosLobes",            &fsp->m_lanczosLobes,                                                         6,                        1,                              16,    "Lanczos Lobes"               },

  { "ChromaDownsampleFilter",  &pParams->m_chromaDownsampleFilter,                          hdrtoolslib::DF_F0,       hdrtoolslib::DF_NN,       hdrtoolslib::DF_TOTAL - 1,    "Chroma Downsampling Filter"               },
  { "ChromaUpsampleFilter",    &pParams->m_chromaUpsampleFilter,                            hdrtoolslib::UF_NN,       hdrtoolslib::UF_NN,       hdrtoolslib::UF_TOTAL - 1,    "Chroma Upsampling Filter"                 },
  { "AddNoise",                &pParams->m_addNoise,                                   hdrtoolslib::NOISE_NULL,  hdrtoolslib::NOISE_NULL,    hdrtoolslib::NOISE_TOTAL - 1,    "Add noise to the input signal"            },
  { "ClosedLoopConversion",    (int *) &pParams->m_closedLoopConversion,                 hdrtoolslib::CLT_NULL,    hdrtoolslib::CLT_NULL,      hdrtoolslib::CLT_TOTAL - 1,    "Enable Closed Loop Conversion"            },
  { "ClosedLoopIterations",    &pParams->m_closedLoopIterations,                                            10,                        1,                         1000000,    "Number of Closed Loop Iterations"         },
  { "SourceConstantLuminance", &src->m_iConstantLuminance,                                                   0,                        0,                               3,    "Constant Luminance Source"                },
  { "OutputConstantLuminance", &out->m_iConstantLuminance,                                                   0,                        0,                               3,    "Co nstant Luminance Output"                },
  { "UseMinMaxFiltering",      &pParams->m_useMinMax,                                                        0,                        0,                               3,    "Use Min/Max Filtering"                    },
  { "ToneMappingMode",         &pParams->m_toneMapping,                                   hdrtoolslib::TM_NULL,     hdrtoolslib::TM_NULL,       hdrtoolslib::TM_TOTAL - 1,    "Tone Mapping Mode "                       },
  { "HighPrecisionColor",      &pParams->m_useHighPrecisionTransform,                                        0,                        0,                               2,    "High Precision Color Mode "               },
  { "UseAdaptiveUpsampling",   &pParams->m_useAdaptiveUpsampling,                        hdrtoolslib::ADF_NULL,    hdrtoolslib::ADF_NULL,      hdrtoolslib::ADF_TOTAL - 1,    "Use Adaptive Upsampler"                   },
  { "UseAdaptiveDownsampling", &pParams->m_useAdaptiveDownsampling,                      hdrtoolslib::ADF_NULL,    hdrtoolslib::ADF_NULL,      hdrtoolslib::ADF_TOTAL - 1,    "Use Adaptive Downsampler"                 },
  { "ForceClipping",           &pParams->m_forceClipping,                                                    0,                        0,                               2,    "Input Source Clipping"                    },
  { "",                        NULL,                                                                         0,                        0,                               0,    "Integer Termination entry"                }
};

hdrtoolslib::BoolParameter boolParameterList[] = {
  { "SourceInterleaved",          &pParams->m_inputFile.m_isInterleaved,      false,       false,         true,    "Interleaved Source"                         },
  { "SourceInterlaced",           &src->m_isInterlaced,                       false,       false,         true,    "Interlaced Source"                          },
  { "OutputInterleaved",          &pParams->m_outputFile.m_isInterleaved,     false,       false,         true,    "Interleaved Output"                         },
  { "SilentMode",                 &pParams->m_silentMode,                     false,       false,         true,    "Silent mode"                                },
  { "USeSingleTransferStep",      &pParams->m_useSingleTransferStep,          false,       false,         true,    "Use a single transfer step"                 },
  { "TransformPrecision",         &pParams->m_transformPrecision,              true,       false,         true,    "Color transform precision"                  },
  { "SetOutputSinglePrec",        &pParams->m_outputSinglePrecision,          false,       false,         true,    "OpenEXR Output data precision"              },
  { "SetOutputEXRRounding",       &out->m_useFloatRound,                      false,       false,         true,    "OpenEXR Output with Rounding Enabled"       },
  { "FilterUsingFloats",          &pParams->m_filterInFloat,                  false,       false,         true,    "Perform Filtering using Floats "            },
  { "LinearDownConversion",       &pParams->m_linearDownConversion,           false,       false,         true,    "Perform linear downconversion to 420"       },
  { "RGBDownConversion",          &pParams->m_rgbDownConversion,              false,       false,         true,    "Perform downconversion in RGB"              },
  { "UseChromaDeblocking",        &pParams->m_bUseChromaDeblocking,           false,       false,         true,    "Deblock Chroma before Upconversion"         },
  { "UseWienerFiltering",         &pParams->m_bUseWienerFiltering,            false,       false,         true,    "Wiener Filtering before conversion"         },
  { "UseNLMeansFiltering",        &pParams->m_bUseNLMeansFiltering,           false,       false,         true,    "NLMeans Filtering before conversion"        },
  { "Use2DSepFiltering",          &pParams->m_bUse2DSepFiltering,             false,       false,         true,    "Separable Filtering before conversion"      },
  { "2DSepFilteringMode",         &pParams->m_b2DSepMode,                     false,       false,         true,    "Edge Adaptation On or OFF"                  },
  { "EnableTFunctionLUT",         &pParams->m_enableTFunctionLUT,             false,       false,         true,    "Enable TF LUT for some operations"          },
  { "EnableTFunctionLUT",         &pParams->m_enableTFLUTs,                   false,       false,         true,    "Enable TF LUTs for closed loop conversions" },
  { "EnableTFunctionLUT",         &ctp->m_enableLUTs,                         false,       false,         true,    "Enable TF LUTs for closed loop conversions" },
  { "EnableTFDerivLUT",           &ctp->m_enableTFDerivLUTs,                  false,       false,         true,    "Enable TF derivative LUTs  "                },
  { "EnableDither",               &cvp->m_isDither,                           false,       false,         true,    "Enable Dithering"                           },
  { "EnableSkew",                 &cvp->m_useSkew,                            false,       false,         true,    "Enable Skew for dithering"                  },
  { "EnableGreenOffset",          &cvp->m_hasOffset,                          false,       false,         true,    "Enable Table offset for Green dither"       },
  { "TMScaleGammut",              &tmp->m_scaleGammut,                        false,       false,         true,    "Enable Color Gamut rescaling in TM"         },
  { "EnableLegacy",               &pParams->m_enableLegacy,                    true,       false,         true,    "Enable Legacy TF conversions"               },
  { "ScaleOnly",                  &pParams->m_scaleOnly,                      false,       false,         true,    "Enable Scaling only for YUV sources"        },
  // The option "OverrideContentValues" prevents the overwriting the options and keeps using the options from Config and CLI (this is due our corrent design).
  { "OverrideContentValues",      &src->m_keepConfigFileParams,               false,       false,         true,    "Always use the values from the config file or CLI" },

  { "",                           NULL,                                           0,           0,            0,    "Boolean Termination entry"                  }
};

hdrtoolslib::DoubleParameter doubleParameterList[] = {
  { "NoiseVariance",          &pParams->m_noiseVariance,                    1.0,         0.0,      20000.0,    "Additive Noise variance"                  },
  { "NoiseMean",              &pParams->m_noiseMean,                        0.0,         0.0,      20000.0,    "Additive Noise mean"                      },
  { "TMGamma",                &tmp->m_gamma,                                1.0,         0.0,         10.0,    "TM Gamma"                                 },
  { "TMMinValue",             &tmp->m_minValue,                           800.0,         0.0,     100000.0,    "TM Min value"                             },
  { "TMMaxValue",             &tmp->m_maxValue,                          4000.0,         0.0,     100000.0,    "TM Max value"                             },
  { "TMTargetValue",          &tmp->m_targetValue,                       1000.0,         0.0,     100000.0,    "TM Target value"                          },

  { "",                      NULL,                                          0.0,         0.0,          0.0,    "Double Termination entry"                 }
};


hdrtoolslib::FloatParameter floatParameterList[] = {
  { "SourceRate",                  &src->m_frameRate,                       24.00F,       0.01F,      120.00F,    "Input Source Frame Rate"                     },
  { "OutputRate",                  &out->m_frameRate,                        0.00F,       0.00F,      120.00F,    "Image Output Frame Rate"                     },
  { "SourceSystemGamma",           &src->m_systemGamma,                      1.00F,       0.00F,       10.00F,    "Overall System gamma for Hybrid Gamma TF"    },
  { "OutputSystemGamma",           &out->m_systemGamma,                      1.00F,       0.00F,       10.00F,    "Overall System gamma for Hybrid Gamma TF"    },
  { "SourceSystemGamma",           &ctp->m_iSystemGamma,                     1.00F,       0.00F,       10.00F,    "Copy for processing (input)"                 },
  { "OutputSystemGamma",           &ctp->m_oSystemGamma,                     1.00F,       0.00F,       10.00F,    "Copy for processing (output)"                },
  { "SourceNormalizationScale",    &pParams->m_srcNormalScale,           10000.00F,       0.01F,  1000000.00F,    "Source Normalization Scale for Linear Data"  },
  { "OutputNormalizationScale",    &pParams->m_outNormalScale,           10000.00F,       0.01F,  1000000.00F,    "Output Normalization Scale for Linear Data"  },
  { "SourceTransferMinBrightness", &pParams->m_srcMinValue,                  0.00F,       0.00F,  1000000.00F,    "Source Transfer Function Minimum Brightness" },
  { "SourceTransferMaxBrightness", &pParams->m_srcMaxValue,              10000.00F,       0.00F,  1000000.00F,    "Source Transfer Function Maximum Brightness" },
  { "OutputTransferMinBrightness", &pParams->m_outMinValue,                  0.00F,       0.00F,  1000000.00F,    "Output Transfer Function Minimum Brightness" },
  { "OutputTransferMinBrightness", &ctp->m_minValue,                         0.00F,       0.00F,  1000000.00F,    "Output Transfer Function Minimum Brightness" },
  { "OutputTransferMaxBrightness", &pParams->m_outMaxValue,              10000.00F,       0.00F,  1000000.00F,    "Output Transfer Function Maximum Brightness" },
  { "OutputTransferMaxBrightness", &ctp->m_maxValue,                     10000.00F,       0.00F,  1000000.00F,    "Output Transfer Function Maximum Brightness" },
  
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
  
  if (m_output.m_width[0] == 0) {
    // currently lets force the same resolution
    m_output.m_width[0]  = m_source.m_width[0] - m_cropOffsetLeft + m_cropOffsetRight;
    m_output.m_width[0]   = hdrtoolslib::ceilingRescale(m_output.m_width[0], 3);
    // we should not update the resolution here since maybe there could be
    // subsequent input width/height parameters in a configuration file.
  }
  if (m_output.m_height[0] == 0) {
    // currently lets force the same resolution
    m_output.m_height[0] = m_source.m_height[0] - m_cropOffsetTop + m_cropOffsetBottom;
    m_output.m_height[0]  = hdrtoolslib::ceilingRescale(m_output.m_height[0],3);
    // we should not update the resolution here since maybe there could be
    // subsequent input width/height parameters in a configuration file.
  }

  //if (m_output.m_frameRate == 0.0) {
  // currently lets force the same frame rate
    // m_output.m_frameRate = m_source.m_frameRate;
  //}

  setupFormat(&m_source);
  setupFormat(&m_output);
  
  using ::hdrtoolslib::Y_COMP;
  if (m_outputSinglePrecision == 0) {
    m_output.m_pixelType[Y_COMP] = m_output.m_pixelType[hdrtoolslib::U_COMP] =
    m_output.m_pixelType[hdrtoolslib::V_COMP] = m_output.m_pixelType[hdrtoolslib::A_COMP] = hdrtoolslib::HALF;
  }
  else {
    m_output.m_pixelType[Y_COMP] = m_output.m_pixelType[hdrtoolslib::U_COMP] =
    m_output.m_pixelType[hdrtoolslib::V_COMP] = m_output.m_pixelType[hdrtoolslib::A_COMP] = hdrtoolslib::FLOAT;
  }

  if (m_outputFile.m_format.m_pixelFormat == hdrtoolslib::PF_YCM)
    m_output.m_pixelFormat = hdrtoolslib::PF_YCM;
  if (m_inputFile.m_format.m_pixelFormat == hdrtoolslib::PF_YCM)
    m_source.m_pixelFormat = hdrtoolslib::PF_YCM;

  if (m_source.m_chromaFormat == hdrtoolslib::CF_420 && m_source.m_pixelFormat == hdrtoolslib::PF_YCM)
    m_inputFile.m_isInterleaved = true;
  if (m_output.m_chromaFormat == hdrtoolslib::CF_420 && m_output.m_pixelFormat == hdrtoolslib::PF_YCM)
    m_outputFile.m_isInterleaved = true;

  if (m_outputFile.m_format.m_pixelFormat == hdrtoolslib::PF_PCK) {
    if ((m_outputFile.m_format.m_bitDepthComp[0] != 10) && (m_outputFile.m_format.m_bitDepthComp[0] != 12)){
      fprintf(stderr, "The packed planar format is only supported with 10/12 bit data.\n");
      exit(EXIT_FAILURE);
    }
    if (m_output.m_chromaFormat != hdrtoolslib::CF_400 &&
        ((m_outputFile.m_format.m_bitDepthComp[0] != m_outputFile.m_format.m_bitDepthComp[1]) &&
         (m_outputFile.m_format.m_bitDepthComp[0] != m_outputFile.m_format.m_bitDepthComp[2])))  {
      fprintf(stderr, "Bitdepth should be the same for all color planes.\n");
      exit(EXIT_FAILURE);
    }
    m_output.m_pixelFormat = hdrtoolslib::PF_PCK;
  }
  if (m_inputFile.m_format.m_pixelFormat == hdrtoolslib::PF_PCK) {
    if ((m_inputFile.m_format.m_bitDepthComp[0] != 10) && (m_inputFile.m_format.m_bitDepthComp[0] != 12)){
      fprintf(stderr, "The packed planar format is only supported with 10/12 bit data.\n");
      exit(EXIT_FAILURE);
    }
    if (m_source.m_chromaFormat != hdrtoolslib::CF_400 &&
        ((m_inputFile.m_format.m_bitDepthComp[0] != m_inputFile.m_format.m_bitDepthComp[1]) &&
         (m_inputFile.m_format.m_bitDepthComp[0] != m_inputFile.m_format.m_bitDepthComp[2]))) {
      fprintf(stderr, "Bitdepth should be the same for all color planes.\n");
      exit(EXIT_FAILURE);
    }
    m_source.m_pixelFormat = hdrtoolslib::PF_PCK;
  }

  if (m_source.m_pixelFormat == hdrtoolslib::PF_PCK)
    m_inputFile.m_isInterleaved = true;
  if (m_output.m_pixelFormat == hdrtoolslib::PF_PCK)
    m_outputFile.m_isInterleaved = true;

  if (m_numberOfFrames == -1)  {
    IOFunctions::openFile (&m_inputFile);
    getNumberOfFrames (&m_source, &m_inputFile, m_inputFile.m_startFrame);
    IOFunctions::closeFile (&m_inputFile);
  }

  if (m_source.m_chromaFormat == hdrtoolslib::CF_420 && (((m_source.m_width[0] & 0x01) == 1) || ((m_source.m_height[0] & 0x01) == 1))) {
    fprintf(stderr, "Only even dimensions supported for 4:2:0 data. Please examine source dimensions.\n");
    exit(EXIT_FAILURE);
  }
  if (m_output.m_chromaFormat == hdrtoolslib::CF_420 && (((m_output.m_width[0] & 0x01) == 1) || ((m_output.m_height[0] & 0x01) == 1))) {
    fprintf(stderr, "Only even dimensions supported for 4:2:0 data. Please examine output dimensions.\n");
    exit(EXIT_FAILURE);
  }
  if (m_source.m_chromaFormat == hdrtoolslib::CF_422 && ((m_source.m_width[0] & 0x01) == 1)) {
    fprintf(stderr, "Only even width supported for 4:2:2 data. Please examine source dimensions.\n");
    exit(EXIT_FAILURE);
  }
  if (m_output.m_chromaFormat == hdrtoolslib::CF_422 && ((m_output.m_width[0] & 0x01) == 1)) {
    fprintf(stderr, "Only even width supported for 4:2:2 data. Please examine output dimensions.\n");
    exit(EXIT_FAILURE);
  }

  m_inputFile.m_format = m_source;
}


void ProjectParameters::setFormatSpecifiedFlagsFromCFG( char *filename ) {
  std::ifstream ifs(filename);
  if (!ifs) return;
  std::string strLine;
  while (std::getline(ifs, strLine)) {
    setFormatSpecifiedFlagsFromString(strLine);
  }
  ifs.close();
}


void ProjectParameters::setFormatSpecifiedFlagsFromCLI( char **cl_params, int numCLParams ) {
  for ( int par = 0; par < numCLParams; par++ ) {
    std::string strLine(cl_params[par]);
    setFormatSpecifiedFlagsFromString(strLine);
  }
}

void ProjectParameters::setFormatSpecifiedFlagsFromString(std::string strConfigLine) {
  // remove all the comments
  std::string strLineStripped = strConfigLine.substr(0, strConfigLine.find('#'));
  // trim right
  strLineStripped.erase(std::find_if(strLineStripped.rbegin(), strLineStripped.rend(), [](unsigned char ch) {
    return !std::isspace(ch);
  }).base(), strLineStripped.end());
  // trim left
  strLineStripped.erase(strLineStripped.begin(), std::find_if(strLineStripped.begin(), strLineStripped.end(), [](unsigned char ch) {
    return !std::isspace(ch);
  }));
  // set the flags for input format
  if ( strLineStripped.find("SourceWidth") != std::string::npos ) src->m_specifiedFlags.m_width = true;
  if ( strLineStripped.find("SourceHeight") != std::string::npos ) src->m_specifiedFlags.m_height = true;
  if ( strLineStripped.find("SourceChromaFormat") != std::string::npos ) src->m_specifiedFlags.m_chromaFormat = true;
  if ( strLineStripped.find("SourceChromaLocationTop") != std::string::npos ) src->m_specifiedFlags.m_chromaLocation = true;
  if ( strLineStripped.find("SourceChromaLocationBottom") != std::string::npos ) src->m_specifiedFlags.m_chromaLocation = true;
  if ( strLineStripped.find("SourceFourCCCode") != std::string::npos ) src->m_specifiedFlags.m_pixelFormat = true;
  if ( strLineStripped.find("SourceBitDepthCmp0") != std::string::npos ) src->m_specifiedFlags.m_bitDepthComp = true;
  if ( strLineStripped.find("SourceBitDepthCmp1") != std::string::npos ) src->m_specifiedFlags.m_bitDepthComp = true;
  if ( strLineStripped.find("SourceBitDepthCmp2") != std::string::npos ) src->m_specifiedFlags.m_bitDepthComp = true;
  if ( strLineStripped.find("SourceColorSpace") != std::string::npos ) src->m_specifiedFlags.m_colorSpace = true;
  if ( strLineStripped.find("SourceColorPrimaries") != std::string::npos ) src->m_specifiedFlags.m_colorPrimaries = true;
  if ( strLineStripped.find("SourceTransferFunction") != std::string::npos ) src->m_specifiedFlags.m_transferFunction = true;
  if ( strLineStripped.find("SourceSampleRange") != std::string::npos ) src->m_specifiedFlags.m_sampleRange = true;
  if ( strLineStripped.find("SourceDisplayAdjustment") != std::string::npos ) src->m_specifiedFlags.m_displayAdjustment = true;
  if ( strLineStripped.find("SourceConstantLuminance") != std::string::npos ) src->m_specifiedFlags.m_iConstantLuminance = true;
  if ( strLineStripped.find("SourceInterlaced") != std::string::npos ) src->m_specifiedFlags.m_isInterlaced = true;
  if ( strLineStripped.find("SourceRate") != std::string::npos ) src->m_specifiedFlags.m_frameRate = true;
  if ( strLineStripped.find("SourceSystemGamma") != std::string::npos ) src->m_specifiedFlags.m_systemGamma = true;
  // set the flags for output format
  if ( strLineStripped.find("OutputWidth") != std::string::npos ) out->m_specifiedFlags.m_width = true;
  if ( strLineStripped.find("OutputHeight") != std::string::npos ) out->m_specifiedFlags.m_height = true;
  if ( strLineStripped.find("OutputChromaFormat") != std::string::npos ) out->m_specifiedFlags.m_chromaFormat = true;
  if ( strLineStripped.find("OutputChromaLocationTop") != std::string::npos ) out->m_specifiedFlags.m_chromaLocation = true;
  if ( strLineStripped.find("OutputChromaLocationBottom") != std::string::npos ) out->m_specifiedFlags.m_chromaLocation = true;
  if ( strLineStripped.find("OutputFourCCCode") != std::string::npos ) out->m_specifiedFlags.m_pixelFormat = true;
  if ( strLineStripped.find("OutputBitDepthCmp0") != std::string::npos ) out->m_specifiedFlags.m_bitDepthComp = true;
  if ( strLineStripped.find("OutputBitDepthCmp1") != std::string::npos ) out->m_specifiedFlags.m_bitDepthComp = true;
  if ( strLineStripped.find("OutputBitDepthCmp2") != std::string::npos ) out->m_specifiedFlags.m_bitDepthComp = true;
  if ( strLineStripped.find("OutputColorSpace") != std::string::npos ) out->m_specifiedFlags.m_colorSpace = true;
  if ( strLineStripped.find("OutputColorPrimaries") != std::string::npos ) out->m_specifiedFlags.m_colorPrimaries = true;
  if ( strLineStripped.find("OutputTransferFunction") != std::string::npos ) out->m_specifiedFlags.m_transferFunction = true;
  if ( strLineStripped.find("OutputSampleRange") != std::string::npos ) out->m_specifiedFlags.m_sampleRange = true;
  if ( strLineStripped.find("OutputDisplayAdjustment") != std::string::npos ) out->m_specifiedFlags.m_displayAdjustment = true;
  if ( strLineStripped.find("OutputConstantLuminance") != std::string::npos ) out->m_specifiedFlags.m_iConstantLuminance = true;
  if ( strLineStripped.find("SetOutputEXRRounding") != std::string::npos ) out->m_specifiedFlags.m_useFloatRound = true;
  if ( strLineStripped.find("OutputRate") != std::string::npos ) out->m_specifiedFlags.m_frameRate = true;
  if ( strLineStripped.find("OutputSystemGamma") != std::string::npos ) out->m_specifiedFlags.m_systemGamma = true;
}

//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
