/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = ITU/ISO/IEC
 * <ORGANIZATION> = ITU/ISO/IEC
 * <YEAR> = 2021
 *
 * Copyright (c) (c) 2010-2021, ITU/ISO/IEC
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
 * \file DistortionMetricJVETMSSSIM.cpp
 *
 * \brief
 *    JVET based MS-SSIM (Multi-Scale Structural Similarity) distortion computation Class
 *    Code is ported from the VTM software: 
 *          https://vcgit.hhi.fraunhofer.de/jvet/VVCSoftware_VTM
 *
 * \author
 *     - Unknown
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "DistortionMetricJVETMSSSIM.H"
#include <string.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

static const int MAX_MSSSIM_SCALE  = 5;
static const int WEIGHTING_MID_TAP = 5;
static const int WEIGHTING_SIZE    = WEIGHTING_MID_TAP*2+1;

static const double exponentWeights[MAX_MSSSIM_SCALE][MAX_MSSSIM_SCALE] = 
              {{1.0,    0,      0,      0,      0     },
                {0.1356, 0.8644, 0,      0,      0     },
                {0.0711, 0.4530, 0.4760, 0,      0     },
                {0.0517, 0.3295, 0.3462, 0.2726, 0     },
                {0.0448, 0.2856, 0.3001, 0.2363, 0.1333}};


//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

namespace hdrtoolslib {

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

DistortionMetricJVETMSSSIM::DistortionMetricJVETMSSSIM(const FrameFormat *format, SSIMParams *params, double maxSampleValue, bool enableHexMetric)
: DistortionMetric()
{
  
  m_blockDistance   = params->m_blockDistance;
  m_blockSizeX      = params->m_blockSizeX;
  m_blockSizeY      = params->m_blockSizeY;
  m_useLogSSIM      = params->m_useLog;
  m_enableHexMetric = enableHexMetric;

  m_colorSpace    = format->m_colorSpace;
    
  for (int c = 0; c < T_COMP; c++) {
    m_metric[c] = 0.0;
    m_metricStats[c].reset();
    m_maxValue[c] = (double) maxSampleValue;
    m_hexMetricMSSSIM[c] = 0;
  }
}

DistortionMetricJVETMSSSIM::~DistortionMetricJVETMSSSIM()
{
}

//-----------------------------------------------------------------------------
// Private methods (non-MMX)
//-----------------------------------------------------------------------------


float DistortionMetricJVETMSSSIM::compute(uint8 *inp0Data, uint8 *inp1Data, int height, int width, int comp, int maxPixelValue)
{
  uint32_t maxScale = 5;

  // For low resolution videos determine number of scales 
  if (width < 22 || height < 22)  {
    maxScale = 1; 
  }
  else if (width < 44 || height < 44)  {
    maxScale = 2; 
  }
  else if (width < 88 || height < 88)  {
    maxScale = 3; 
  }
  else if (width < 176 || height < 176)  {
    maxScale = 4; 
  }
  else  {
    maxScale = 5;
  }

  //Normalized Gaussian mask design, 11*11, s.d. 1.5
  double weights[WEIGHTING_SIZE][WEIGHTING_SIZE];
  double coeffSum=0.0;
  for(int y=0; y<WEIGHTING_SIZE; y++)  {
    for(int x=0; x<WEIGHTING_SIZE; x++) {
      weights[y][x] =
        exp(-((y - WEIGHTING_MID_TAP) * (y - WEIGHTING_MID_TAP) + (x - WEIGHTING_MID_TAP) * (x - WEIGHTING_MID_TAP))
            / (WEIGHTING_MID_TAP - 0.5));
      coeffSum += weights[y][x];
    }
  }

  for(int y=0; y<WEIGHTING_SIZE; y++)  {
    for(int x=0; x<WEIGHTING_SIZE; x++) {
      weights[y][x] /=coeffSum;
    }
  }

  //Downsampling of data:
  std::vector<double> original[MAX_MSSSIM_SCALE];
  std::vector<double> recon[MAX_MSSSIM_SCALE];

  for(uint32_t scale=0; scale<maxScale; scale++)  {
    const int scaledHeight = height >> scale;
    const int scaledWidth  = width  >> scale;
    original[scale].resize(scaledHeight*scaledWidth, double(0));
    recon[scale].resize(scaledHeight*scaledWidth, double(0));
  }

  // Initial [0] arrays to be a copy of the source data (but stored in array "double", not Pel array).
  for(int y=0; y<height; y++)  {
    for(int x=0; x<width; x++) {
      original[0][y*width+x] = inp0Data[y * width+x];
      recon[0][   y*width+x] = inp1Data[y * width+x];
    }
  }

  // Set up other arrays to be average value of each 2x2 sample.
  for(uint32_t scale=1; scale<maxScale; scale++)
  {
    const int scaledHeight = height >> scale;
    const int scaledWidth  = width  >> scale;
    for(int y=0; y<scaledHeight; y++)
    {
      for(int x=0; x<scaledWidth; x++)
      {
        original[scale][y*scaledWidth+x]= (original[scale-1][ 2*y   *(2*scaledWidth)+2*x  ] +
                                           original[scale-1][ 2*y   *(2*scaledWidth)+2*x+1] +
                                           original[scale-1][(2*y+1)*(2*scaledWidth)+2*x  ] +
                                           original[scale-1][(2*y+1)*(2*scaledWidth)+2*x+1]) / 4.0;
        recon[scale][y*scaledWidth+x]=    (   recon[scale-1][ 2*y   *(2*scaledWidth)+2*x  ] +
                                              recon[scale-1][ 2*y   *(2*scaledWidth)+2*x+1] +
                                              recon[scale-1][(2*y+1)*(2*scaledWidth)+2*x  ] +
                                              recon[scale-1][(2*y+1)*(2*scaledWidth)+2*x+1]) / 4.0;
      }
    }
  }
  
  // Calculate MS-SSIM:
  const uint32_t   maxValue  = maxPixelValue;
  const double c1        = (0.01*maxValue)*(0.01*maxValue);
  const double c2        = (0.03*maxValue)*(0.03*maxValue);
  
  double finalMSSSIM = 1.0;

  for(uint32_t scale=0; scale<maxScale; scale++)
  {
    const int scaledHeight    = height >> scale;
    const int scaledWidth     = width  >> scale;
    const int blocksPerRow    = scaledWidth-WEIGHTING_SIZE+1;
    const int blocksPerColumn = scaledHeight-WEIGHTING_SIZE+1;
    const int totalBlocks     = blocksPerRow*blocksPerColumn;

    double meanSSIM= 0.0;

    for(int blockIndexY=0; blockIndexY<blocksPerColumn; blockIndexY++)
    {
      for(int blockIndexX=0; blockIndexX<blocksPerRow; blockIndexX++)
      {
        double muOrg          =0.0;
        double muRec          =0.0;
        double muOrigSqr      =0.0;
        double muRecSqr       =0.0;
        double muOrigMultRec  =0.0;

        for(int y=0; y<WEIGHTING_SIZE; y++)
        {
          for(int x=0;x<WEIGHTING_SIZE; x++)
          {
            const double gaussianWeight=weights[y][x];
            const int    sampleOffset=(blockIndexY+y)*scaledWidth+(blockIndexX+x);
            const double orgPel=original[scale][sampleOffset];
            const double recPel=   recon[scale][sampleOffset];

            muOrg        +=orgPel*       gaussianWeight;
            muRec        +=recPel*       gaussianWeight;
            muOrigSqr    +=orgPel*orgPel*gaussianWeight;
            muRecSqr     +=recPel*recPel*gaussianWeight;
            muOrigMultRec+=orgPel*recPel*gaussianWeight;
          }
        }

        const double sigmaSqrOrig = muOrigSqr    -(muOrg*muOrg);
        const double sigmaSqrRec  = muRecSqr     -(muRec*muRec);
        const double sigmaOrigRec = muOrigMultRec-(muOrg*muRec);

        double blockSSIMVal = ((2.0*sigmaOrigRec + c2)/(sigmaSqrOrig+sigmaSqrRec + c2));
        if(scale == maxScale-1)
        {
          blockSSIMVal*=(2.0*muOrg*muRec + c1)/(muOrg*muOrg+muRec*muRec + c1);
        }

        meanSSIM += blockSSIMVal;
      }
    }

    meanSSIM /=totalBlocks;

    finalMSSSIM *= pow(meanSSIM, exponentWeights[maxScale-1][scale]);
  }

  if (m_useLogSSIM)
    return (float) ssimSNR(finalMSSSIM);
  else
    return finalMSSSIM;
}


float DistortionMetricJVETMSSSIM::compute(uint16 *inp0Data, uint16 *inp1Data, int height, int width, int comp, int maxPixelValue)
{
  uint32_t maxScale = 5;

  // For low resolution videos determine number of scales 
  if (width < 22 || height < 22)  {
    maxScale = 1; 
  }
  else if (width < 44 || height < 44)  {
    maxScale = 2; 
  }
  else if (width < 88 || height < 88)  {
    maxScale = 3; 
  }
  else if (width < 176 || height < 176)  {
    maxScale = 4; 
  }
  else  {
    maxScale = 5;
  }

  //Normalized Gaussian mask design, 11*11, s.d. 1.5
  double weights[WEIGHTING_SIZE][WEIGHTING_SIZE];
  double coeffSum=0.0;
  for(int y=0; y<WEIGHTING_SIZE; y++)  {
    for(int x=0; x<WEIGHTING_SIZE; x++) {
      weights[y][x] =
        exp(-((y - WEIGHTING_MID_TAP) * (y - WEIGHTING_MID_TAP) + (x - WEIGHTING_MID_TAP) * (x - WEIGHTING_MID_TAP))
            / (WEIGHTING_MID_TAP - 0.5));
      coeffSum += weights[y][x];
    }
  }

  for(int y=0; y<WEIGHTING_SIZE; y++)  {
    for(int x=0; x<WEIGHTING_SIZE; x++) {
      weights[y][x] /=coeffSum;
    }
  }

  //Downsampling of data:
  std::vector<double> original[MAX_MSSSIM_SCALE];
  std::vector<double> recon[MAX_MSSSIM_SCALE];

  for(uint32_t scale=0; scale<maxScale; scale++)  {
    const int scaledHeight = height >> scale;
    const int scaledWidth  = width  >> scale;
    original[scale].resize(scaledHeight*scaledWidth, double(0));
    recon[scale].resize(scaledHeight*scaledWidth, double(0));
  }

  // Initial [0] arrays to be a copy of the source data (but stored in array "double", not Pel array).
  for(int y=0; y<height; y++)  {
    for(int x=0; x<width; x++) {
      original[0][y*width+x] = inp0Data[y * width+x];
      recon[0][   y*width+x] = inp1Data[y * width+x];
    }
  }

  // Set up other arrays to be average value of each 2x2 sample.
  for(uint32_t scale=1; scale<maxScale; scale++)
  {
    const int scaledHeight = height >> scale;
    const int scaledWidth  = width  >> scale;
    for(int y=0; y<scaledHeight; y++)
    {
      for(int x=0; x<scaledWidth; x++)
      {
        original[scale][y*scaledWidth+x]= (original[scale-1][ 2*y   *(2*scaledWidth)+2*x  ] +
                                           original[scale-1][ 2*y   *(2*scaledWidth)+2*x+1] +
                                           original[scale-1][(2*y+1)*(2*scaledWidth)+2*x  ] +
                                           original[scale-1][(2*y+1)*(2*scaledWidth)+2*x+1]) / 4.0;
        recon[scale][y*scaledWidth+x]=    (   recon[scale-1][ 2*y   *(2*scaledWidth)+2*x  ] +
                                              recon[scale-1][ 2*y   *(2*scaledWidth)+2*x+1] +
                                              recon[scale-1][(2*y+1)*(2*scaledWidth)+2*x  ] +
                                              recon[scale-1][(2*y+1)*(2*scaledWidth)+2*x+1]) / 4.0;
      }
    }
  }
  
  // Calculate MS-SSIM:
  const uint32_t   maxValue  = maxPixelValue;
  const double c1        = (0.01*maxValue)*(0.01*maxValue);
  const double c2        = (0.03*maxValue)*(0.03*maxValue);
  
  double finalMSSSIM = 1.0;

  for(uint32_t scale=0; scale<maxScale; scale++)
  {
    const int scaledHeight    = height >> scale;
    const int scaledWidth     = width  >> scale;
    const int blocksPerRow    = scaledWidth-WEIGHTING_SIZE+1;
    const int blocksPerColumn = scaledHeight-WEIGHTING_SIZE+1;
    const int totalBlocks     = blocksPerRow*blocksPerColumn;

    double meanSSIM= 0.0;

    for(int blockIndexY=0; blockIndexY<blocksPerColumn; blockIndexY++)
    {
      for(int blockIndexX=0; blockIndexX<blocksPerRow; blockIndexX++)
      {
        double muOrg          =0.0;
        double muRec          =0.0;
        double muOrigSqr      =0.0;
        double muRecSqr       =0.0;
        double muOrigMultRec  =0.0;

        for(int y=0; y<WEIGHTING_SIZE; y++)
        {
          for(int x=0;x<WEIGHTING_SIZE; x++)
          {
            const double gaussianWeight=weights[y][x];
            const int    sampleOffset=(blockIndexY+y)*scaledWidth+(blockIndexX+x);
            const double orgPel=original[scale][sampleOffset];
            const double recPel=   recon[scale][sampleOffset];

            muOrg        +=orgPel*       gaussianWeight;
            muRec        +=recPel*       gaussianWeight;
            muOrigSqr    +=orgPel*orgPel*gaussianWeight;
            muRecSqr     +=recPel*recPel*gaussianWeight;
            muOrigMultRec+=orgPel*recPel*gaussianWeight;
          }
        }

        const double sigmaSqrOrig = muOrigSqr    -(muOrg*muOrg);
        const double sigmaSqrRec  = muRecSqr     -(muRec*muRec);
        const double sigmaOrigRec = muOrigMultRec-(muOrg*muRec);

        double blockSSIMVal = ((2.0*sigmaOrigRec + c2)/(sigmaSqrOrig+sigmaSqrRec + c2));
        if(scale == maxScale-1)
        {
          blockSSIMVal*=(2.0*muOrg*muRec + c1)/(muOrg*muOrg+muRec*muRec + c1);
        }

        meanSSIM += blockSSIMVal;
      }
    }

    meanSSIM /=totalBlocks;

    finalMSSSIM *= pow(meanSSIM, exponentWeights[maxScale-1][scale]);
  }

  if (m_useLogSSIM)
    return (float) ssimSNR(finalMSSSIM);
  else  
  return finalMSSSIM;
}

float DistortionMetricJVETMSSSIM::compute(float *inp0Data, float *inp1Data, int height, int width, int comp, float maxPixelValue)
{
  uint32_t maxScale = 5;

  // For low resolution videos determine number of scales 
  if (width < 22 || height < 22)  {
    maxScale = 1; 
  }
  else if (width < 44 || height < 44)  {
    maxScale = 2; 
  }
  else if (width < 88 || height < 88)  {
    maxScale = 3; 
  }
  else if (width < 176 || height < 176)  {
    maxScale = 4; 
  }
  else  {
    maxScale = 5;
  }

  //Normalized Gaussian mask design, 11*11, s.d. 1.5
  double weights[WEIGHTING_SIZE][WEIGHTING_SIZE];
  double coeffSum=0.0;
  for(int y=0; y<WEIGHTING_SIZE; y++)  {
    for(int x=0; x<WEIGHTING_SIZE; x++) {
      weights[y][x] =
        exp(-((y - WEIGHTING_MID_TAP) * (y - WEIGHTING_MID_TAP) + (x - WEIGHTING_MID_TAP) * (x - WEIGHTING_MID_TAP))
            / (WEIGHTING_MID_TAP - 0.5));
      coeffSum += weights[y][x];
    }
  }

  for(int y=0; y<WEIGHTING_SIZE; y++)  {
    for(int x=0; x<WEIGHTING_SIZE; x++) {
      weights[y][x] /=coeffSum;
    }
  }

  //Downsampling of data:
  std::vector<double> original[MAX_MSSSIM_SCALE];
  std::vector<double> recon[MAX_MSSSIM_SCALE];

  for(uint32_t scale=0; scale<maxScale; scale++)  {
    const int scaledHeight = height >> scale;
    const int scaledWidth  = width  >> scale;
    original[scale].resize(scaledHeight*scaledWidth, double(0));
    recon[scale].resize(scaledHeight*scaledWidth, double(0));
  }

  // Initial [0] arrays to be a copy of the source data (but stored in array "double", not Pel array).
  for(int y=0; y<height; y++)  {
    for(int x=0; x<width; x++) {
      original[0][y*width+x] = inp0Data[y * width+x];
      recon[0][   y*width+x] = inp1Data[y * width+x];
    }
  }

  // Set up other arrays to be average value of each 2x2 sample.
  for(uint32_t scale=1; scale<maxScale; scale++)
  {
    const int scaledHeight = height >> scale;
    const int scaledWidth  = width  >> scale;
    for(int y=0; y<scaledHeight; y++)
    {
      for(int x=0; x<scaledWidth; x++)
      {
        original[scale][y*scaledWidth+x]= (original[scale-1][ 2*y   *(2*scaledWidth)+2*x  ] +
                                           original[scale-1][ 2*y   *(2*scaledWidth)+2*x+1] +
                                           original[scale-1][(2*y+1)*(2*scaledWidth)+2*x  ] +
                                           original[scale-1][(2*y+1)*(2*scaledWidth)+2*x+1]) / 4.0;
        recon[scale][y*scaledWidth+x]=    (   recon[scale-1][ 2*y   *(2*scaledWidth)+2*x  ] +
                                              recon[scale-1][ 2*y   *(2*scaledWidth)+2*x+1] +
                                              recon[scale-1][(2*y+1)*(2*scaledWidth)+2*x  ] +
                                              recon[scale-1][(2*y+1)*(2*scaledWidth)+2*x+1]) / 4.0;
      }
    }
  }
  
  // Calculate MS-SSIM:
  const uint32_t   maxValue  = maxPixelValue;
  const double c1        = (0.01*maxValue)*(0.01*maxValue);
  const double c2        = (0.03*maxValue)*(0.03*maxValue);
  
  double finalMSSSIM = 1.0;

  for(uint32_t scale=0; scale<maxScale; scale++)
  {
    const int scaledHeight    = height >> scale;
    const int scaledWidth     = width  >> scale;
    const int blocksPerRow    = scaledWidth-WEIGHTING_SIZE+1;
    const int blocksPerColumn = scaledHeight-WEIGHTING_SIZE+1;
    const int totalBlocks     = blocksPerRow*blocksPerColumn;

    double meanSSIM= 0.0;

    for(int blockIndexY=0; blockIndexY<blocksPerColumn; blockIndexY++)
    {
      for(int blockIndexX=0; blockIndexX<blocksPerRow; blockIndexX++)
      {
        double muOrg          =0.0;
        double muRec          =0.0;
        double muOrigSqr      =0.0;
        double muRecSqr       =0.0;
        double muOrigMultRec  =0.0;

        for(int y=0; y<WEIGHTING_SIZE; y++)
        {
          for(int x=0;x<WEIGHTING_SIZE; x++)
          {
            const double gaussianWeight=weights[y][x];
            const int    sampleOffset=(blockIndexY+y)*scaledWidth+(blockIndexX+x);
            const double orgPel=original[scale][sampleOffset];
            const double recPel=   recon[scale][sampleOffset];

            muOrg        +=orgPel*       gaussianWeight;
            muRec        +=recPel*       gaussianWeight;
            muOrigSqr    +=orgPel*orgPel*gaussianWeight;
            muRecSqr     +=recPel*recPel*gaussianWeight;
            muOrigMultRec+=orgPel*recPel*gaussianWeight;
          }
        }

        const double sigmaSqrOrig = muOrigSqr    -(muOrg*muOrg);
        const double sigmaSqrRec  = muRecSqr     -(muRec*muRec);
        const double sigmaOrigRec = muOrigMultRec-(muOrg*muRec);

        double blockSSIMVal = ((2.0*sigmaOrigRec + c2)/(sigmaSqrOrig+sigmaSqrRec + c2));
        if(scale == maxScale-1)
        {
          blockSSIMVal*=(2.0*muOrg*muRec + c1)/(muOrg*muOrg+muRec*muRec + c1);
        }

        meanSSIM += blockSSIMVal;
      }
    }

    meanSSIM /=totalBlocks;

    finalMSSSIM *= pow(meanSSIM, exponentWeights[maxScale-1][scale]);
  }

  if (m_useLogSSIM)
    return (float) ssimSNR(finalMSSSIM);
  else
    return finalMSSSIM;
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void DistortionMetricJVETMSSSIM::computeMetric (Frame* inp0, Frame* inp1)
{
  // it is assumed here that the frames are of the same type
  if (inp0->equalType(inp1)) {
    if (inp0->m_isFloat == TRUE) {    // floating point data
      for (int c = Y_COMP; c < inp0->m_noComponents; c++) {
        m_metric[c] = (double) compute(inp0->m_floatComp[c], inp1->m_floatComp[c], inp0->m_height[c], inp0->m_width[c], c, (float) m_maxValue[c]);
        m_metricStats[c].updateStats(m_metric[c]);
      }
    }
    else if (inp0->m_bitDepth == 8) {   // 8 bit data
      for (int c = Y_COMP; c < inp0->m_noComponents; c++) {
        m_metric[c] = (double) compute(inp0->m_comp[c], inp1->m_comp[c], inp0->m_height[c], inp0->m_width[c], c, (int) m_maxValue[c]);

        m_metricStats[c].updateStats(m_metric[c]);
      }
    }
    else { // 16 bit data
      for (int c = Y_COMP; c < inp0->m_noComponents; c++) {
        m_metric[c] = (double) compute(inp0->m_ui16Comp[c], inp1->m_ui16Comp[c], inp0->m_height[c], inp0->m_width[c], c, (int) m_maxValue[c]);
        m_metricStats[c].updateStats(m_metric[c]);
      }
    }
  }
  else {
    printf("Frames of different type being compared. Computation will not be performed for this frame.\n");
  }
}


void DistortionMetricJVETMSSSIM::computeMetric (Frame* inp0, Frame* inp1, int component)
{
  // it is assumed here that the frames are of the same type
  // Currently no TF is applied on the data. However, we may wish to apply some TF, e.g. PQ or combination of PQ and PH.
  
  if (inp0->equalType(inp1)) {
    if (inp0->m_isFloat == TRUE) {    // floating point data
      m_metric[component] = (double) compute(inp0->m_floatComp[component], inp1->m_floatComp[component], inp0->m_height[component], inp0->m_width[component], component, (float) m_maxValue[component]);
      m_metricStats[component].updateStats(m_metric[component]);
    }
    else if (inp0->m_bitDepth == 8) {   // 8 bit data
      m_metric[component] = (double) compute(inp0->m_comp[component], inp1->m_comp[component], inp0->m_height[component], inp0->m_width[component], component, (int) m_maxValue[component]);
      
      m_metricStats[component].updateStats(m_metric[component]);
    }
    else { // 16 bit data
      m_metric[component] = (double) compute(inp0->m_ui16Comp[component], inp1->m_ui16Comp[component], inp0->m_height[component], inp0->m_width[component], component, (int) m_maxValue[component]);
      m_metricStats[component].updateStats(m_metric[component]);
    }
  }
  else {
    printf("Frames of different type being compared. Computation will not be performed for this frame.\n");
  }
}

void DistortionMetricJVETMSSSIM::reportMetric  ()
{
  if (m_useLogSSIM)
    printf("%10.3f %10.3f %10.3f ", m_metric[Y_COMP], m_metric[U_COMP], m_metric[V_COMP]);
  else
    printf("%10.5f %10.5f %10.5f ", m_metric[Y_COMP], m_metric[U_COMP], m_metric[V_COMP]);
  if (m_enableHexMetric == TRUE) {
    for (int c = 0; c < T_COMP; c++) {
      std::copy(reinterpret_cast<uint8_t *>(&m_metric[c]),
                reinterpret_cast<uint8_t *>(&m_metric[c]) + sizeof(m_metric[c]),
                reinterpret_cast<uint8_t *>(&m_hexMetricMSSSIM[c]));
    }
    printf(" %16" PRIx64 " %16" PRIx64 " %16" PRIx64, m_hexMetricMSSSIM[Y_COMP], m_hexMetricMSSSIM[U_COMP], m_hexMetricMSSSIM[V_COMP]);
  }
}

void DistortionMetricJVETMSSSIM::reportSummary  ()
{
  if (m_useLogSSIM)
    printf("%10.3f %10.3f %10.3f ", m_metricStats[Y_COMP].getAverage(), m_metricStats[U_COMP].getAverage(), m_metricStats[V_COMP].getAverage());
  else
  printf("%10.5f %10.5f %10.5f ", m_metricStats[Y_COMP].getAverage(), m_metricStats[U_COMP].getAverage(), m_metricStats[V_COMP].getAverage());
  if (m_enableHexMetric == TRUE) {
    for (int c = 0; c < T_COMP; c++) {
      double val = m_metricStats[c].getAverage();
      std::copy(reinterpret_cast<uint8_t *>(&val),
                reinterpret_cast<uint8_t *>(&val) + sizeof(val),
                reinterpret_cast<uint8_t *>(&m_hexMetricMSSSIM[c]));
    }
    printf(" %16" PRIx64 " %16" PRIx64 " %16" PRIx64, m_hexMetricMSSSIM[Y_COMP], m_hexMetricMSSSIM[U_COMP], m_hexMetricMSSSIM[V_COMP]);
  }
}

void DistortionMetricJVETMSSSIM::reportMinimum  ()
{
  if (m_useLogSSIM)
    printf("%10.3f %10.3f %10.3f ", m_metricStats[Y_COMP].minimum, m_metricStats[U_COMP].minimum, m_metricStats[V_COMP].minimum);
  else
    printf("%10.5f %10.5f %10.5f ", m_metricStats[Y_COMP].minimum, m_metricStats[U_COMP].minimum, m_metricStats[V_COMP].minimum);
  if (m_enableHexMetric == TRUE) {
    for (int c = 0; c < T_COMP; c++) {
      std::copy(reinterpret_cast<uint8_t *>(&m_metricStats[c].minimum),
                reinterpret_cast<uint8_t *>(&m_metricStats[c].minimum) + sizeof(m_metricStats[c].minimum),
                reinterpret_cast<uint8_t *>(&m_hexMetricMSSSIM[c]));
    }
    printf(" %16" PRIx64 " %16" PRIx64 " %16" PRIx64, m_hexMetricMSSSIM[Y_COMP], m_hexMetricMSSSIM[U_COMP], m_hexMetricMSSSIM[V_COMP]);
  }
}

void DistortionMetricJVETMSSSIM::reportMaximum  ()
{
  if (m_useLogSSIM)
    printf("%10.3f %10.3f %10.3f ", m_metricStats[Y_COMP].maximum, m_metricStats[U_COMP].maximum, m_metricStats[V_COMP].maximum);
else
  printf("%10.5f %10.5f %10.5f ", m_metricStats[Y_COMP].maximum, m_metricStats[U_COMP].maximum, m_metricStats[V_COMP].maximum);
  if (m_enableHexMetric == TRUE) {
    for (int c = 0; c < T_COMP; c++) {
      std::copy(reinterpret_cast<uint8_t *>(&m_metricStats[c].maximum),
                reinterpret_cast<uint8_t *>(&m_metricStats[c].maximum) + sizeof(m_metricStats[c].maximum),
                reinterpret_cast<uint8_t *>(&m_hexMetricMSSSIM[c]));
    }
    printf(" %16" PRIx64 " %16" PRIx64 " %16" PRIx64, m_hexMetricMSSSIM[Y_COMP], m_hexMetricMSSSIM[U_COMP], m_hexMetricMSSSIM[V_COMP]);
  }
}

void DistortionMetricJVETMSSSIM::printHeader()
{
  if (m_isWindow == FALSE ) {
    switch (m_colorSpace) {
      case CM_YCbCr:
        printf(" JMSSSIM-Y "); // 11
        printf(" JMSSSIM-U "); // 11
        printf(" JMSSSIM-V "); // 11
        if (m_enableHexMetric == TRUE) {
          printf("   hexJMSSSIM-Y  "); // 17
          printf("   hexJMSSSIM-U  "); // 17
          printf("   hexJMSSSIM-V  "); // 17
        }
        break;
      case CM_RGB:
        printf(" JMSSSIM-R "); // 11
        printf(" JMSSSIM-G "); // 11
        printf(" JMSSSIM-B "); // 11
        if (m_enableHexMetric == TRUE) {
          printf("   hexJMSSSIM-R  "); // 17
          printf("   hexJMSSSIM-G  "); // 17
          printf("   hexJMSSSIM-B  "); // 17
        }
        break;
      case CM_XYZ:
        printf(" JMSSSIM-X "); // 11
        printf(" JMSSSIM-Y "); // 11
        printf(" JMSSSIM-Z "); // 11
        if (m_enableHexMetric == TRUE) {
          printf("   hexJMSSSIM-X  "); // 17
          printf("   hexJMSSSIM-Y  "); // 17
          printf("   hexJMSSSIM-Z  "); // 17
        }
        break;
      default:
        printf("JMSSSIM-C0 "); // 11
        printf("JMSSSIM-C1 "); // 11
        printf("JMSSSIM-C2 "); // 11
        if (m_enableHexMetric == TRUE) {
          printf("   hexJMSSSIM-C0 "); // 17
          printf("   hexJMSSSIM-C1 "); // 17
          printf("   hexJMSSSIM-C2 "); // 17
        }
        break;
    }
  }
  else {
    switch (m_colorSpace) {
      case CM_YCbCr:
        printf("wJMSSSIM-Y "); // 11
        printf("wJMSSSIM-U "); // 11
        printf("wJMSSSIM-V "); // 11
        if (m_enableHexMetric == TRUE) {
          printf("  hexwJMSSSIM-Y  "); // 17
          printf("  hexwJMSSSIM-U  "); // 17
          printf("  hexwJMSSSIM-V  "); // 17
        }
        break;
      case CM_RGB:
        printf("wJMSSSIM-R "); // 11
        printf("wJMSSSIM-G "); // 11
        printf("wJMSSSIM-B "); // 11
        if (m_enableHexMetric == TRUE) {
          printf("  hexwJMSSSIM-R  "); // 17
          printf("  hexwJMSSSIM-G  "); // 17
          printf("  hexwJMSSSIM-B  "); // 17
        }
        break;
      case CM_XYZ:
        printf("wJMSSSIM-X "); // 11
        printf("wJMSSSIM-Y "); // 11
        printf("wJMSSSIM-Z "); // 11
        if (m_enableHexMetric == TRUE) {
          printf("  hexwJMSSSIM-X  "); // 17
          printf("  hexwJMSSSIM-Y  "); // 17
          printf("  hexwJMSSSIM-Z  "); // 17
        }
        break;
      default:
        printf("wJMSSSIM-C0"); // 11
        printf("wJMSSSIM-C1"); // 11
        printf("wJMSSSIM-C2"); // 11
        if (m_enableHexMetric == TRUE) {
          printf("  hexwJMSSSIM-X  "); // 17
          printf("  hexwJMSSSIM-Y  "); // 17
          printf("  hexwJMSSSIM-Z  "); // 17
        }
        break;
    }
  }
}

void DistortionMetricJVETMSSSIM::printSeparator(){
  printf("-----------");
  printf("-----------");
  printf("-----------");
  if (m_enableHexMetric == TRUE) {
    printf("-----------------");
    printf("-----------------");
    printf("-----------------");
  }
}
} // namespace hdrtoolslib
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
