/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Netflix Inc., Apple Inc.
 * <ORGANIZATION> = Netflix Inc., Apple Inc.
 * <YEAR> = 2020
 *
 * Copyright (c) 2020, Netflix Inc., Apple Inc.
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
 * \file AddAV1FilmGrain.H
 *
 * \brief
 *    AddAV1FilmGrain Class
 *    This function performs noise addition based on the film grain
 *    model specified in AV1 
 *
 * \author
 *     - Andrey Norkin                        <anorkin@netflix.com>
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */


#ifndef __AddAV1FilmGrain_H__
#define __AddAV1FilmGrain_H__

#include "Global.H"
#include "Frame.H"
#include "AddNoise.H"

#define TWO_PI 6.2831853071795864769252866

namespace hdrtoolslib {



class AddAV1FilmGrain : public AddNoise {
private:
  
  std::vector<int>     m_lumaGrainBlock;
  std::vector<int>     m_cbGrainBlock;
  std::vector<int>     m_crGrainBlock;
  int m_lumaGrainStride;
  //int m_chromaGrainStride;

  int m_lumaSubblockSizeY;
  int m_lumaSubblockSizeX;
  
  int m_chromaSubblockSizeY;
  int m_chromaSubblockSizeX;
  
  std::vector<int> m_scalingLutY;
  std::vector<int> m_scalingLutCb;
  std::vector<int> m_scalingLutCr;

  std::vector<std::vector<int> > m_predPosLuma;
  std::vector<std::vector<int> > m_predPosChroma;
  
  std::vector<int>  m_yLineBuf;
  std::vector<int>  m_cbLineBuf;
  std::vector<int>  m_crLineBuf;

  std::vector<int>  m_yColBuf;
  std::vector<int>  m_cbColBuf;
  std::vector<int>  m_crColBuf;

  
  int m_grainMin;
  int m_grainMax;
  
  uint16 m_randomRegister; 

  void initRandomGenerator(int lumaLine, uint16 seed);
  void initArrays(AV1FGParams *params, 
                  int lumaStride, int chromaStride,                  
                  int lumaGrainSamples, int chromaGrainSamples,
                  int chromaSubsampY, int chromaSubsampX);

  // get a number between 0 and 2^bits - 1
  int getRandomNumber(int bits) {
    uint16 bit;
    bit = ((m_randomRegister >> 0) ^ (m_randomRegister >> 1) ^
           (m_randomRegister >> 3) ^ (m_randomRegister >> 12)) &
          1;
    m_randomRegister = (m_randomRegister >> 1) | (bit << 15);
    return (m_randomRegister >> (16 - bits)) & ((1 << bits) - 1);
  }

  //
  
  bool m_haveSpare;
	double m_rand1, m_rand2;
  double m_variance;
  double m_mean;

  void addNoiseData (const uint16 *iData, uint16 *oData, int size, int maxSampleValue);
  void addNoiseData (const imgpel *iData, imgpel *oData, int size, int maxSampleValue);
  void addNoiseData (const float  *iData, float  *oData, int size, double maxSampleValue);

  int generateLumaGrainBlock(
                             AV1FGParams *params,
                             int lumaBlockSizeY, 
                             int lumaBlockSizeX, 
                             int lumaGrainStride,
                             int leftPad, 
                             int topPad, 
                             int rightPad, 
                             int bottomPad);
  int generateChromaGrainBlocks(
                            AV1FGParams *params, 
                            int lumaGrainStride, 
                            int chromaBlockSizeY,
                            int chromaBlockSizeX, 
                            int chromaGrainStride,
                            int leftPad, 
                            int topPad,
                            int rightPad, 
                            int bottomPad, 
                            int chromaSubsampY, 
                            int chromaSubsampX) ;
  void initScalingFunction(const int scaling_points[][2], int num_points,
                           int scalingLut[]);
  int ScaleLUT(int *scalingLut, int index, int bitDepth);
  void addNoiseToBlock(AV1FGParams *params, uint8_t *luma,
  uint8_t *cb, uint8_t *cr, int lumaStride,
  int chromaStride, int *luma_grain,
  int *cb_grain, int *cr_grain,
  int lumaGrainStride, int chromaGrainStride,
  int half_luma_height, int half_luma_width,
  int bitDepth, int chromaSubsampY,
                                        int chromaSubsampX, int mc_identity);
  void addNoiseToBlock(
  AV1FGParams *params, uint16_t *luma, uint16_t *cb, uint16_t *cr,
  int lumaStride, int chromaStride, int *luma_grain, int *cb_grain,
  int *cr_grain, int lumaGrainStride, int chromaGrainStride,
  int half_luma_height, int half_luma_width, int bitDepth,
                                        int chromaSubsampY, int chromaSubsampX, int mc_identity) ;
  
  void copyRect(uint8_t *inp, int src_stride, uint8_t *out,
  int dst_stride, int width, int height,
                                 int use_high_bit_depth);
  void copyArea(int *inp, int src_stride, int *out, int dst_stride,
                                 int width, int height);
  void extendEven(uint8_t *out, int dst_stride, int width, int height,
                  int use_high_bit_depth);
  void verBoundaryOverlap(int *left_block, int left_stride,
  int *right_block, int right_stride,
  int *dst_block, int dst_stride, int width,
                                           int height);
  void horBoundaryOverlap(int *top_block, int top_stride,
  int *bottom_block, int bottom_stride,
  int *dst_block, int dst_stride, int width,
                          int height);
  
  int AddFilmGrainRun(AV1FGParams *params, uint8_t *luma,
  uint8_t *cb, uint8_t *cr, int height, int width,
  int lumaStride, int chromaStride,
  int use_high_bit_depth, int chromaSubsampY,
                      int chromaSubsampX, int mc_identity);
public:
  // Construct/Deconstruct
  AddAV1FilmGrain(AV1FGParams *params);
  virtual ~AddAV1FilmGrain();
  int process(AV1FGParams *params, Frame *out,  Frame *inp);
  virtual void process(Frame *out,  Frame *inp);
  virtual void process(Frame *inp);
};
} //namespace hdrtoolslib

#endif

