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
 * \file AddAV1FilmGrain.cpp
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

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "Global.H"
#include "AddAV1FilmGrain.H"

#include <string.h>

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

namespace hdrtoolslib {

//--------------
// Constants
//--------------
static const int kGaussianSequence[2048] = {
  56,    568,   -180,  172,   124,   -84,   172,   -64,   -900,  24,   820,
  224,   1248,  996,   272,   -8,    -916,  -388,  -732,  -104,  -188, 800,
  112,   -652,  -320,  -376,  140,   -252,  492,   -168,  44,    -788, 588,
  -584,  500,   -228,  12,    680,   272,   -476,  972,   -100,  652,  368,
  432,   -196,  -720,  -192,  1000,  -332,  652,   -136,  -552,  -604, -4,
  192,   -220,  -136,  1000,  -52,   372,   -96,   -624,  124,   -24,  396,
  540,   -12,   -104,  640,   464,   244,   -208,  -84,   368,   -528, -740,
  248,   -968,  -848,  608,   376,   -60,   -292,  -40,   -156,  252,  -292,
  248,   224,   -280,  400,   -244,  244,   -60,   76,    -80,   212,  532,
  340,   128,   -36,   824,   -352,  -60,   -264,  -96,   -612,  416,  -704,
  220,   -204,  640,   -160,  1220,  -408,  900,   336,   20,    -336, -96,
  -792,  304,   48,    -28,   -1232, -1172, -448,  104,   -292,  -520, 244,
  60,    -948,  0,     -708,  268,   108,   356,   -548,  488,   -344, -136,
  488,   -196,  -224,  656,   -236,  -1128, 60,    4,     140,   276,  -676,
  -376,  168,   -108,  464,   8,     564,   64,    240,   308,   -300, -400,
  -456,  -136,  56,    120,   -408,  -116,  436,   504,   -232,  328,  844,
  -164,  -84,   784,   -168,  232,   -224,  348,   -376,  128,   568,  96,
  -1244, -288,  276,   848,   832,   -360,  656,   464,   -384,  -332, -356,
  728,   -388,  160,   -192,  468,   296,   224,   140,   -776,  -100, 280,
  4,     196,   44,    -36,   -648,  932,   16,    1428,  28,    528,  808,
  772,   20,    268,   88,    -332,  -284,  124,   -384,  -448,  208,  -228,
  -1044, -328,  660,   380,   -148,  -300,  588,   240,   540,   28,   136,
  -88,   -436,  256,   296,   -1000, 1400,  0,     -48,   1056,  -136, 264,
  -528,  -1108, 632,   -484,  -592,  -344,  796,   124,   -668,  -768, 388,
  1296,  -232,  -188,  -200,  -288,  -4,    308,   100,   -168,  256,  -500,
  204,   -508,  648,   -136,  372,   -272,  -120,  -1004, -552,  -548, -384,
  548,   -296,  428,   -108,  -8,    -912,  -324,  -224,  -88,   -112, -220,
  -100,  996,   -796,  548,   360,   -216,  180,   428,   -200,  -212, 148,
  96,    148,   284,   216,   -412,  -320,  120,   -300,  -384,  -604, -572,
  -332,  -8,    -180,  -176,  696,   116,   -88,   628,   76,    44,   -516,
  240,   -208,  -40,   100,   -592,  344,   -308,  -452,  -228,  20,   916,
  -1752, -136,  -340,  -804,  140,   40,    512,   340,   248,   184,  -492,
  896,   -156,  932,   -628,  328,   -688,  -448,  -616,  -752,  -100, 560,
  -1020, 180,   -800,  -64,   76,    576,   1068,  396,   660,   552,  -108,
  -28,   320,   -628,  312,   -92,   -92,   -472,  268,   16,    560,  516,
  -672,  -52,   492,   -100,  260,   384,   284,   292,   304,   -148, 88,
  -152,  1012,  1064,  -228,  164,   -376,  -684,  592,   -392,  156,  196,
  -524,  -64,   -884,  160,   -176,  636,   648,   404,   -396,  -436, 864,
  424,   -728,  988,   -604,  904,   -592,  296,   -224,  536,   -176, -920,
  436,   -48,   1176,  -884,  416,   -776,  -824,  -884,  524,   -548, -564,
  -68,   -164,  -96,   692,   364,   -692,  -1012, -68,   260,   -480, 876,
  -1116, 452,   -332,  -352,  892,   -1088, 1220,  -676,  12,    -292, 244,
  496,   372,   -32,   280,   200,   112,   -440,  -96,   24,    -644, -184,
  56,    -432,  224,   -980,  272,   -260,  144,   -436,  420,   356,  364,
  -528,  76,    172,   -744,  -368,  404,   -752,  -416,  684,   -688, 72,
  540,   416,   92,    444,   480,   -72,   -1416, 164,   -1172, -68,  24,
  424,   264,   1040,  128,   -912,  -524,  -356,  64,    876,   -12,  4,
  -88,   532,   272,   -524,  320,   276,   -508,  940,   24,    -400, -120,
  756,   60,    236,   -412,  100,   376,   -484,  400,   -100,  -740, -108,
  -260,  328,   -268,  224,   -200,  -416,  184,   -604,  -564,  -20,  296,
  60,    892,   -888,  60,    164,   68,    -760,  216,   -296,  904,  -336,
  -28,   404,   -356,  -568,  -208,  -1480, -512,  296,   328,   -360, -164,
  -1560, -776,  1156,  -428,  164,   -504,  -112,  120,   -216,  -148, -264,
  308,   32,    64,    -72,   72,    116,   176,   -64,   -272,  460,  -536,
  -784,  -280,  348,   108,   -752,  -132,  524,   -540,  -776,  116,  -296,
  -1196, -288,  -560,  1040,  -472,  116,   -848,  -1116, 116,   636,  696,
  284,   -176,  1016,  204,   -864,  -648,  -248,  356,   972,   -584, -204,
  264,   880,   528,   -24,   -184,  116,   448,   -144,  828,   524,  212,
  -212,  52,    12,    200,   268,   -488,  -404,  -880,  824,   -672, -40,
  908,   -248,  500,   716,   -576,  492,   -576,  16,    720,   -108, 384,
  124,   344,   280,   576,   -500,  252,   104,   -308,  196,   -188, -8,
  1268,  296,   1032,  -1196, 436,   316,   372,   -432,  -200,  -660, 704,
  -224,  596,   -132,  268,   32,    -452,  884,   104,   -1008, 424,  -1348,
  -280,  4,     -1168, 368,   476,   696,   300,   -8,    24,    180,  -592,
  -196,  388,   304,   500,   724,   -160,  244,   -84,   272,   -256, -420,
  320,   208,   -144,  -156,  156,   364,   452,   28,    540,   316,  220,
  -644,  -248,  464,   72,    360,   32,    -388,  496,   -680,  -48,  208,
  -116,  -408,  60,    -604,  -392,  548,   -840,  784,   -460,  656,  -544,
  -388,  -264,  908,   -800,  -628,  -612,  -568,  572,   -220,  164,  288,
  -16,   -308,  308,   -112,  -636,  -760,  280,   -668,  432,   364,  240,
  -196,  604,   340,   384,   196,   592,   -44,   -500,  432,   -580, -132,
  636,   -76,   392,   4,     -412,  540,   508,   328,   -356,  -36,  16,
  -220,  -64,   -248,  -60,   24,    -192,  368,   1040,  92,    -24,  -1044,
  -32,   40,    104,   148,   192,   -136,  -520,  56,    -816,  -224, 732,
  392,   356,   212,   -80,   -424,  -1008, -324,  588,   -1496, 576,  460,
  -816,  -848,  56,    -580,  -92,   -1372, -112,  -496,  200,   364,  52,
  -140,  48,    -48,   -60,   84,    72,    40,    132,   -356,  -268, -104,
  -284,  -404,  732,   -520,  164,   -304,  -540,  120,   328,   -76,  -460,
  756,   388,   588,   236,   -436,  -72,   -176,  -404,  -316,  -148, 716,
  -604,  404,   -72,   -88,   -888,  -68,   944,   88,    -220,  -344, 960,
  472,   460,   -232,  704,   120,   832,   -228,  692,   -508,  132,  -476,
  844,   -748,  -364,  -44,   1116,  -1104, -1056, 76,    428,   552,  -692,
  60,    356,   96,    -384,  -188,  -612,  -576,  736,   508,   892,  352,
  -1132, 504,   -24,   -352,  324,   332,   -600,  -312,  292,   508,  -144,
  -8,    484,   48,    284,   -260,  -240,  256,   -100,  -292,  -204, -44,
  472,   -204,  908,   -188,  -1000, -256,  92,    1164,  -392,  564,  356,
  652,   -28,   -884,  256,   484,   -192,  760,   -176,  376,   -524, -452,
  -436,  860,   -736,  212,   124,   504,   -476,  468,   76,    -472, 552,
  -692,  -944,  -620,  740,   -240,  400,   132,   20,    192,   -196, 264,
  -668,  -1012, -60,   296,   -316,  -828,  76,    -156,  284,   -768, -448,
  -832,  148,   248,   652,   616,   1236,  288,   -328,  -400,  -124, 588,
  220,   520,   -696,  1032,  768,   -740,  -92,   -272,  296,   448,  -464,
  412,   -200,  392,   440,   -200,  264,   -152,  -260,  320,   1032, 216,
  320,   -8,    -64,   156,   -1016, 1084,  1172,  536,   484,   -432, 132,
  372,   -52,   -256,  84,    116,   -352,  48,    116,   304,   -384, 412,
  924,   -300,  528,   628,   180,   648,   44,    -980,  -220,  1320, 48,
  332,   748,   524,   -268,  -720,  540,   -276,  564,   -344,  -208, -196,
  436,   896,   88,    -392,  132,   80,    -964,  -288,  568,   56,   -48,
  -456,  888,   8,     552,   -156,  -292,  948,   288,   128,   -716, -292,
  1192,  -152,  876,   352,   -600,  -260,  -812,  -468,  -28,   -120, -32,
  -44,   1284,  496,   192,   464,   312,   -76,   -516,  -380,  -456, -1012,
  -48,   308,   -156,  36,    492,   -156,  -808,  188,   1652,  68,   -120,
  -116,  316,   160,   -140,  352,   808,   -416,  592,   316,   -480, 56,
  528,   -204,  -568,  372,   -232,  752,   -344,  744,   -4,    324,  -416,
  -600,  768,   268,   -248,  -88,   -132,  -420,  -432,  80,    -288, 404,
  -316,  -1216, -588,  520,   -108,  92,    -320,  368,   -480,  -216, -92,
  1688,  -300,  180,   1020,  -176,  820,   -68,   -228,  -260,  436,  -904,
  20,    40,    -508,  440,   -736,  312,   332,   204,   760,   -372, 728,
  96,    -20,   -632,  -520,  -560,  336,   1076,  -64,   -532,  776,  584,
  192,   396,   -728,  -520,  276,   -188,  80,    -52,   -612,  -252, -48,
  648,   212,   -688,  228,   -52,   -260,  428,   -412,  -272,  -404, 180,
  816,   -796,  48,    152,   484,   -88,   -216,  988,   696,   188,  -528,
  648,   -116,  -180,  316,   476,   12,    -564,  96,    476,   -252, -364,
  -376,  -392,  556,   -256,  -576,  260,   -352,  120,   -16,   -136, -260,
  -492,  72,    556,   660,   580,   616,   772,   436,   424,   -32,  -324,
  -1268, 416,   -324,  -80,   920,   160,   228,   724,   32,    -516, 64,
  384,   68,    -128,  136,   240,   248,   -204,  -68,   252,   -932, -120,
  -480,  -628,  -84,   192,   852,   -404,  -288,  -132,  204,   100,  168,
  -68,   -196,  -868,  460,   1080,  380,   -80,   244,   0,     484,  -888,
  64,    184,   352,   600,   460,   164,   604,   -196,  320,   -64,  588,
  -184,  228,   12,    372,   48,    -848,  -344,  224,   208,   -200, 484,
  128,   -20,   272,   -468,  -840,  384,   256,   -720,  -520,  -464, -580,
  112,   -120,  644,   -356,  -208,  -608,  -528,  704,   560,   -424, 392,
  828,   40,    84,    200,   -152,  0,     -144,  584,   280,   -120, 80,
  -556,  -972,  -196,  -472,  724,   80,    168,   -32,   88,    160,  -688,
  0,     160,   356,   372,   -776,  740,   -128,  676,   -248,  -480, 4,
  -364,  96,    544,   232,   -1032, 956,   236,   356,   20,    -40,  300,
  24,    -676,  -596,  132,   1120,  -104,  532,   -1096, 568,   648,  444,
  508,   380,   188,   -376,  -604,  1488,  424,   24,    756,   -220, -192,
  716,   120,   920,   688,   168,   44,    -460,  568,   284,   1144, 1160,
  600,   424,   888,   656,   -356,  -320,  220,   316,   -176,  -724, -188,
  -816,  -628,  -348,  -228,  -380,  1012,  -452,  -660,  736,   928,  404,
  -696,  -72,   -268,  -892,  128,   184,   -344,  -780,  360,   336,  400,
  344,   428,   548,   -112,  136,   -228,  -216,  -820,  -516,  340,  92,
  -136,  116,   -300,  376,   -244,  100,   -316,  -520,  -284,  -12,  824,
  164,   -548,  -180,  -128,  116,   -924,  -828,  268,   -368,  -580, 620,
  192,   160,   0,     -1676, 1068,  424,   -56,   -360,  468,   -156, 720,
  288,   -528,  556,   -364,  548,   -148,  504,   316,   152,   -648, -620,
  -684,  -24,   -376,  -384,  -108,  -920,  -1032, 768,   180,   -264, -508,
  -1268, -260,  -60,   300,   -240,  988,   724,   -376,  -576,  -212, -736,
  556,   192,   1092,  -620,  -880,  376,   -56,   -4,    -216,  -32,  836,
  268,   396,   1332,  864,   -600,  100,   56,    -412,  -92,   356,  180,
  884,   -468,  -436,  292,   -388,  -804,  -704,  -840,  368,   -348, 140,
  -724,  1536,  940,   372,   112,   -372,  436,   -480,  1136,  296,  -32,
  -228,  132,   -48,   -220,  868,   -1016, -60,   -1044, -464,  328,  916,
  244,   12,    -736,  -296,  360,   468,   -376,  -108,  -92,   788,  368,
  -56,   544,   400,   -672,  -420,  728,   16,    320,   44,    -284, -380,
  -796,  488,   132,   204,   -596,  -372,  88,    -152,  -908,  -636, -572,
  -624,  -116,  -692,  -200,  -56,   276,   -88,   484,   -324,  948,  864,
  1000,  -456,  -184,  -276,  292,   -296,  156,   676,   320,   160,  908,
  -84,   -1236, -288,  -116,  260,   -372,  -644,  732,   -756,  -96,  84,
  344,   -520,  348,   -688,  240,   -84,   216,   -1044, -136,  -676, -396,
  -1500, 960,   -40,   176,   168,   1516,  420,   -504,  -344,  -364, -360,
  1216,  -940,  -380,  -212,  252,   -660,  -708,  484,   -444,  -152, 928,
  -120,  1112,  476,   -260,  560,   -148,  -344,  108,   -196,  228,  -288,
  504,   560,   -328,  -88,   288,   -1008, 460,   -228,  468,   -836, -196,
  76,    388,   232,   412,   -1168, -716,  -644,  756,   -172,  -356, -504,
  116,   432,   528,   48,    476,   -168,  -608,  448,   160,   -532, -272,
  28,    -676,  -12,   828,   980,   456,   520,   104,   -104,  256,  -344,
  -4,    -28,   -368,  -52,   -524,  -572,  -556,  -200,  768,   1124, -208,
  -512,  176,   232,   248,   -148,  -888,  604,   -600,  -304,  804,  -156,
  -212,  488,   -192,  -804,  -256,  368,   -360,  -916,  -328,  228,  -240,
  -448,  -472,  856,   -556,  -364,  572,   -12,   -156,  -368,  -340, 432,
  252,   -752,  -152,  288,   268,   -580,  -848,  -592,  108,   -76,  244,
  312,   -716,  592,   -80,   436,   360,   4,     -248,  160,   516,  584,
  732,   44,    -468,  -280,  -292,  -156,  -588,  28,    308,   912,  24,
  124,   156,   180,   -252,  944,   -924,  -772,  -520,  -428,  -624, 300,
  -212,  -1144, 32,    -724,  800,   -1128, -212,  -1288, -848,  180,  -416,
  440,   192,   -576,  -792,  -76,   -1080, 80,    -532,  -352,  -132, 380,
  -820,  148,   1112,  128,   164,   456,   700,   -924,  144,   -668, -384,
  648,   -832,  508,   552,   -52,   -100,  -656,  208,   -568,  748,  -88,
  680,   232,   300,   192,   -408,  -1012, -152,  -252,  -268,  272,  -876,
  -664,  -648,  -332,  -136,  16,    12,    1152,  -28,   332,   -536, 320,
  -672,  -460,  -316,  532,   -260,  228,   -40,   1052,  -816,  180,  88,
  -496,  -556,  -672,  -368,  428,   92,    356,   404,   -408,  252,  196,
  -176,  -556,  792,   268,   32,    372,   40,    96,    -332,  328,  120,
  372,   -900,  -40,   472,   -264,  -592,  952,   128,   656,   112,  664,
  -232,  420,   4,     -344,  -464,  556,   244,   -416,  -32,   252,  0,
  -412,  188,   -696,  508,   -476,  324,   -1096, 656,   -312,  560,  264,
  -136,  304,   160,   -64,   -580,  248,   336,   -720,  560,   -348, -288,
  -276,  -196,  -500,  852,   -544,  -236,  -1128, -992,  -776,  116,  56,
  52,    860,   884,   212,   -12,   168,   1020,  512,   -552,  924,  -148,
  716,   188,   164,   -340,  -520,  -184,  880,   -152,  -680,  -208, -1156,
  -300,  -528,  -472,  364,   100,   -744,  -1056, -32,   540,   280,  144,
  -676,  -32,   -232,  -280,  -224,  96,    568,   -76,   172,   148,  148,
  104,   32,    -296,  -32,   788,   -80,   32,    -16,   280,   288,  944,
  428,   -484
};

static const int kGaussBits = 11;

static const int kMinLumaLegalRange = 16;
static const int kMaxLumaLegalRange = 235;

static const int kMinChromaLegalRange = 16;
static const int kMaxChromaLegalRange = 240;

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

AddAV1FilmGrain::AddAV1FilmGrain(AV1FGParams *params) {
  int i;
  
  m_lumaSubblockSizeY = 32;
  m_lumaSubblockSizeX = 32;
  
  m_chromaSubblockSizeY = 16;
  m_chromaSubblockSizeX = 16;
  
  for (i = 0; i < 256; i++) {
    m_scalingLutY[i] = 0;
    m_scalingLutCb[i] = 0;
    m_scalingLutCr[i] = 0;
  }
  
  m_scalingLutY.resize(256);
  m_scalingLutCb.resize(256);
  m_scalingLutCr.resize(256);
  
  m_grainMin = 0;
  m_grainMax = 0;
  
  m_randomRegister = 0; 

  // to be removed from here to the end
  m_variance  = 1.0;
  m_mean      = 0.0;
  m_haveSpare = FALSE;
	m_rand1     = 0.0;
  m_rand2     = 0.0;
  
  // init noise
  srand (0);

}

AddAV1FilmGrain::~AddAV1FilmGrain() {
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
void AddAV1FilmGrain::initArrays(AV1FGParams *params, 
                                 int lumaStride, int chromaStride, 
                        int lumaGrainSamples, int chromaGrainSamples,
                        int chromaSubsampY, int chromaSubsampX) {
  
  std::fill(m_scalingLutY.begin(), m_scalingLutY.end(), 0);
  std::fill(m_scalingLutCb.begin(), m_scalingLutCb.end(), 0);
  std::fill(m_scalingLutCr.begin(), m_scalingLutCr.end(), 0);

  int numPosLuma = 2 * params->m_arCoeffLag * (params->m_arCoeffLag + 1);
  int numPosChroma = numPosLuma;
  if (params->m_numYPoints > 0) 
    numPosChroma++;

  m_predPosLuma.resize(numPosLuma);
  for (int i = 0; i < numPosLuma; i++)
    m_predPosLuma[i].resize(3);
  
  m_predPosChroma.resize(numPosChroma);
  for (int i = 0; i < numPosChroma; i++)
    m_predPosChroma[i].resize(3);

  int posArIndex = 0;

  for (int row = -params->m_arCoeffLag; row < 0; row++) {
    for (int col = -params->m_arCoeffLag; col < params->m_arCoeffLag + 1;
         col++) {
      m_predPosLuma[posArIndex][0] = row;
      m_predPosLuma[posArIndex][1] = col;
      m_predPosLuma[posArIndex][2] = 0;

      m_predPosChroma[posArIndex][0] = row;
      m_predPosChroma[posArIndex][1] = col;
      m_predPosChroma[posArIndex][2] = 0;
      ++posArIndex;
    }
  }

  for (int col = -params->m_arCoeffLag; col < 0; col++) {
    m_predPosLuma[posArIndex][0] = 0;
    m_predPosLuma[posArIndex][1] = col;
    m_predPosLuma[posArIndex][2] = 0;

    m_predPosChroma[posArIndex][0] = 0;
    m_predPosChroma[posArIndex][1] = col;
    m_predPosChroma[posArIndex][2] = 0;

    ++posArIndex;
  }

  if (params->numPosLuma > 0) {
    m_predPosChroma[posArIndex][0] = 0;
    m_predPosChroma[posArIndex][1] = 0;
    m_predPosChroma[posArIndex][2] = 1;
  }

  m_yLineBuf.resize(lumaStride * 2);
  m_cbLineBuf.resize(chromaStride * (2 >> chromaSubsampY));
  m_crLineBuf.resize(chromaStride * (2 >> chromaSubsampY));

  m_yColBuf.resize((m_lumaSubblockSizeY + 2) * 2);
  m_cbColBuf.resize((m_chromaSubblockSizeY + (2 >> chromaSubsampY)) * (2 >> chromaSubsampX));
  m_crColBuf.resize((m_chromaSubblockSizeY + (2 >> chromaSubsampY)) * (2 >> chromaSubsampX));
  m_crColBuf.resize(chromaStride * (2 >> chromaSubsampY));

  m_lumaGrainBlock.resize(lumaGrainSamples);
  m_cbGrainBlock.resize(chromaGrainSamples);
  m_crGrainBlock.resize(chromaGrainSamples);
}

/**
 * Initialize the random register based on the seed provided 
 * and the current luma line.
 *
 * @param lumaLine  Current Luma line
 * @param seed      seed used for the register
 *               
*/
void AddAV1FilmGrain::initRandomGenerator(int lumaLine, uint16 seed) {
  // msb & lsb remain fixed for the entire frame
  // maybe we should consider moving their computation elsewhere
  // or create a different register for the value (msb << 8) + lsb
  uint16 msb = (seed >> 8) & 255;
  uint16 lsb = seed & 255;
  
  m_randomRegister = (msb << 8) + lsb;
  
  //  This value changes for each row
  int lumaNum = lumaLine >> 5;
  
  m_randomRegister ^= ((lumaNum * 37 + 178) & 255) << 8;
  m_randomRegister ^= ((lumaNum * 173 + 105) & 255);
}

// Return 0 for success, -1 for failure
int AddAV1FilmGrain::generateLumaGrainBlock(
                                            AV1FGParams *params,
                                            int lumaBlockSizeY, 
                                            int lumaBlockSizeX, 
                                            int lumaGrainStride,
                                            int leftPad, 
                                            int topPad, 
                                            int rightPad, 
                                            int bottomPad) {
  
  if (params->m_numYPoints == 0) {
    std::fill(m_lumaGrainBlock.begin(), m_lumaGrainBlock.end(), 0);
    return 0;
  }

  int bitDepth = params->m_bitDepth;
  int gaussSecShift = 12 - bitDepth + params->m_grainScaleShift;

  int numPosLuma = 2 * params->m_arCoeffLag * (params->m_arCoeffLag + 1);
  int roundingOffset = (1 << (params->m_arCoeffShift - 1));

  for (int i = 0; i < lumaBlockSizeY; i++)
    for (int j = 0; j < lumaBlockSizeX; j++)
      m_lumaGrainBlock[i * lumaGrainStride + j] =
      (kGaussianSequence[getRandomNumber(kGaussBits)] +
       ((1 << gaussSecShift) >> 1)) >> gaussSecShift;
  
  for (int i = topPad; i < lumaBlockSizeY - bottomPad; i++){
    int iPos = i * lumaGrainStride;
    for (int j = leftPad; j < lumaBlockSizeX - rightPad; j++) {
      int wSum = roundingOffset;
      for (int pos = 0; pos < numPosLuma; pos++) {
        wSum += params->m_arCoeffsY[pos] *
        m_lumaGrainBlock[iPos + m_predPosLuma[pos][0] * lumaGrainStride +
                         j + m_predPosLuma[pos][1]];
      }
      m_lumaGrainBlock[iPos + j] =
      iClip(m_lumaGrainBlock[iPos + j] + (wSum >> params->m_arCoeffShift),
            m_grainMin, m_grainMax);
    }
  }
  return 0;
}

// Return 0 for success, -1 for failure
int AddAV1FilmGrain::generateChromaGrainBlocks(
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
                                               int chromaSubsampX) {
  int bitDepth = params->m_bitDepth;
  int gauss_sec_shift = 12 - bitDepth + params->m_grainScaleShift;

  int numPosChroma = 2 * params->m_arCoeffLag * (params->m_arCoeffLag + 1);
  if (params->m_numYPoints > 0) ++numPosChroma;
  int roundingOffset = (1 << (params->m_arCoeffShift - 1));
  //int chromaGrainBlockSize = chromaBlockSizeY * chromaGrainStride;

  if (params->m_numCbPoints || params->m_chromaScalingFromLuma) {
    initRandomGenerator(7 << 5, params->m_randomSeed);

    for (int i = 0; i < chromaBlockSizeY; i++)
      for (int j = 0; j < chromaBlockSizeX; j++)
        m_cbGrainBlock[i * chromaGrainStride + j] =
            (kGaussianSequence[getRandomNumber(kGaussBits)] +
             ((1 << gauss_sec_shift) >> 1)) >>
            gauss_sec_shift;
  } else {
    std::fill(m_cbGrainBlock.begin(), m_cbGrainBlock.end(), 0);
  }

  if (params->m_numCrPoints || params->m_chromaScalingFromLuma) {
    initRandomGenerator(11 << 5, params->m_randomSeed);

    for (int i = 0; i < chromaBlockSizeY; i++)
      for (int j = 0; j < chromaBlockSizeX; j++)
        m_crGrainBlock[i * chromaGrainStride + j] =
            (kGaussianSequence[getRandomNumber(kGaussBits)] +
             ((1 << gauss_sec_shift) >> 1)) >>
            gauss_sec_shift;
  } else {
    std::fill(m_crGrainBlock.begin(), m_crGrainBlock.end(), 0);
  }

  for (int i = topPad; i < chromaBlockSizeY - bottomPad; i++)
    for (int j = leftPad; j < chromaBlockSizeX - rightPad; j++) {
      int wSumCb = 0;
      int wSumCr = 0;
      for (int pos = 0; pos < numPosChroma; pos++) {
        if (m_predPosChroma[pos][2] == 0) {
          wSumCb += params->m_arCoeffsCb[pos] *
                                  m_cbGrainBlock[(i + m_predPosChroma[pos][0]) *
                                                     chromaGrainStride +
                                                 j + m_predPosChroma[pos][1]];
          wSumCr += params->m_arCoeffsCr[pos] *
                                  m_crGrainBlock[(i + m_predPosChroma[pos][0]) *
                                                     chromaGrainStride +
                                                 j + m_predPosChroma[pos][1]];
        } else if (m_predPosChroma[pos][2] == 1) {
          int avLuma = 0;
          int lumaCoordY = ((i - topPad) << chromaSubsampY) + topPad;
          int lumaCoordX = ((j - leftPad) << chromaSubsampX) + leftPad;

          for (int k = lumaCoordY; k < lumaCoordY + chromaSubsampY + 1; k++)
            for (int l = lumaCoordX; l < lumaCoordX + chromaSubsampX + 1; l++)
              avLuma += m_lumaGrainBlock[k * m_lumaGrainStride + l];

          avLuma =
              (avLuma + ((1 << (chromaSubsampY + chromaSubsampX)) >> 1)) >>
              (chromaSubsampY + chromaSubsampX);

          wSumCb += params->m_arCoeffsCb[pos] * avLuma;
          wSumCr += params->m_arCoeffsCr[pos] * avLuma;
        } else {
          fprintf(
              stderr,
              "Grain synthesis: prediction between two chroma components is "
              "not supported!");
          return -1;
        }
      }
      if (params->m_numCbPoints || params->m_chromaScalingFromLuma)
        m_cbGrainBlock[i * chromaGrainStride + j] =
            iClip(m_cbGrainBlock[i * chromaGrainStride + j] +
                      ((wSumCb + roundingOffset) >> params->m_arCoeffShift),
                  m_grainMin, m_grainMax);
      
      if (params->m_numCrPoints || params->m_chromaScalingFromLuma)
        m_crGrainBlock[i * chromaGrainStride + j] =
            iClip(m_crGrainBlock[i * chromaGrainStride + j] +
                      ((wSumCr + roundingOffset) >> params->m_arCoeffShift),
                  m_grainMin, m_grainMax);
    }
  return 0;
}

void AddAV1FilmGrain::initScalingFunction(const int scaling_points[][2], int num_points,
                                  int scalingLut[]) {
  if (num_points == 0) return;

  for (int i = 0; i < scaling_points[0][0]; i++)
    scalingLut[i] = scaling_points[0][1];

  for (int point = 0; point < num_points - 1; point++) {
    int delta_y = scaling_points[point + 1][1] - scaling_points[point][1];
    int delta_x = scaling_points[point + 1][0] - scaling_points[point][0];

    int64_t delta = delta_y * ((65536 + (delta_x >> 1)) / delta_x);

    for (int x = 0; x < delta_x; x++) {
      scalingLut[scaling_points[point][0] + x] =
          scaling_points[point][1] + (int)((x * delta + 32768) >> 16);
    }
  }

  for (int i = scaling_points[num_points - 1][0]; i < 256; i++)
    scalingLut[i] = scaling_points[num_points - 1][1];
}

// function that extracts samples from a LUT (and interpolates intemediate
// frames for 10- and 12-bit video)
int AddAV1FilmGrain::ScaleLUT(int *scalingLut, int index, int bitDepth) {
  int x = index >> (bitDepth - 8);

  if (!(bitDepth - 8) || x == 255)
    return scalingLut[x];
  else
    return scalingLut[x] + (((scalingLut[x + 1] - scalingLut[x]) *
                                  (index & ((1 << (bitDepth - 8)) - 1)) +
                              (1 << (bitDepth - 9))) >> (bitDepth - 8));
}


void AddAV1FilmGrain::addNoiseToBlock(AV1FGParams *params, uint8_t *luma,
                               uint8_t *cb, uint8_t *cr, int lumaStride,
                               int chromaStride, int *luma_grain,
                               int *cb_grain, int *cr_grain,
                               int lumaGrainStride, int chromaGrainStride,
                               int half_luma_height, int half_luma_width,
                               int bitDepth, int chromaSubsampY,
                               int chromaSubsampX, int mc_identity) {
  int cb_mult = params->m_cbMult - 128;            // fixed scale
  int cb_luma_mult = params->m_cbLumaMult - 128;  // fixed scale
  int cb_offset = params->m_cbOffset - 256;

  int cr_mult = params->m_crMult - 128;            // fixed scale
  int cr_luma_mult = params->m_crLumaMult - 128;  // fixed scale
  int cr_offset = params->m_crOffset - 256;

  int rounding_offset = (1 << (params->m_scalingShift - 1));

  int apply_y = params->m_numYPoints > 0 ? 1 : 0;
  int apply_cb =
      (params->m_numCbPoints > 0 || params->m_chromaScalingFromLuma) ? 1 : 0;
  int apply_cr =
      (params->m_numCrPoints > 0 || params->m_chromaScalingFromLuma) ? 1 : 0;

  if (params->m_chromaScalingFromLuma) {
    cb_mult = 0;        // fixed scale
    cb_luma_mult = 64;  // fixed scale
    cb_offset = 0;

    cr_mult = 0;        // fixed scale
    cr_luma_mult = 64;  // fixed scale
    cr_offset = 0;
  }

  int min_luma, max_luma, min_chroma, max_chroma;

  if (params->m_clipToRestrictedRange) {
    min_luma = kMinLumaLegalRange;
    max_luma = kMaxLumaLegalRange;

    if (mc_identity) {
      min_chroma = kMinLumaLegalRange;
      max_chroma = kMaxLumaLegalRange;
    } else {
      min_chroma = kMinChromaLegalRange;
      max_chroma = kMaxChromaLegalRange;
    }
  } else {
    min_luma = min_chroma = 0;
    max_luma = max_chroma = 255;
  }

  for (int i = 0; i < (half_luma_height << (1 - chromaSubsampY)); i++) {
    for (int j = 0; j < (half_luma_width << (1 - chromaSubsampX)); j++) {
      int average_luma = 0;
      if (chromaSubsampX) {
        average_luma = (luma[(i << chromaSubsampY) * lumaStride +
                             (j << chromaSubsampX)] +
                        luma[(i << chromaSubsampY) * lumaStride +
                             (j << chromaSubsampX) + 1] +
                        1) >>
                       1;
      } else {
        average_luma = luma[(i << chromaSubsampY) * lumaStride + j];
      }

      if (apply_cb) {
        cb[i * chromaStride + j] = iClip(
            cb[i * chromaStride + j] +
                ((ScaleLUT(&m_scalingLutCb[0],
                            iClip(((average_luma * cb_luma_mult +
                                    cb_mult * cb[i * chromaStride + j]) >>
                                   6) +
                                      cb_offset,
                                  0, (256 << (bitDepth - 8)) - 1),
                            8) *
                      cb_grain[i * chromaGrainStride + j] +
                  rounding_offset) >>
                 params->m_scalingShift),
            min_chroma, max_chroma);
      }

      if (apply_cr) {
        cr[i * chromaStride + j] = iClip(
            cr[i * chromaStride + j] +
                ((ScaleLUT(&m_scalingLutCr[0],
                            iClip(((average_luma * cr_luma_mult +
                                    cr_mult * cr[i * chromaStride + j]) >>
                                   6) +
                                      cr_offset,
                                  0, (256 << (bitDepth - 8)) - 1),
                            8) *
                      cr_grain[i * chromaGrainStride + j] +
                  rounding_offset) >>
                 params->m_scalingShift),
            min_chroma, max_chroma);
      }
    }
  }

  if (apply_y) {
    for (int i = 0; i < (half_luma_height << 1); i++) {
      for (int j = 0; j < (half_luma_width << 1); j++) {
        luma[i * lumaStride + j] =
            iClip(luma[i * lumaStride + j] +
                      ((ScaleLUT(&m_scalingLutY[0], luma[i * lumaStride + j], 8) *
                            luma_grain[i * lumaGrainStride + j] +
                        rounding_offset) >>
                       params->m_scalingShift),
                  min_luma, max_luma);
      }
    }
  }
}

void AddAV1FilmGrain::addNoiseToBlock(
    AV1FGParams *params, uint16_t *luma, uint16_t *cb, uint16_t *cr,
    int lumaStride, int chromaStride, int *luma_grain, int *cb_grain,
    int *cr_grain, int lumaGrainStride, int chromaGrainStride,
    int half_luma_height, int half_luma_width, int bitDepth,
    int chromaSubsampY, int chromaSubsampX, int mc_identity) {
  int cb_mult = params->m_cbMult - 128;            // fixed scale
  int cb_luma_mult = params->m_cbLumaMult - 128;  // fixed scale
  // offset value depends on the bit depth
  int cb_offset = (params->m_cbOffset << (bitDepth - 8)) - (1 << bitDepth);

  int cr_mult = params->m_crMult - 128;            // fixed scale
  int cr_luma_mult = params->m_crLumaMult - 128;  // fixed scale
  // offset value depends on the bit depth
  int cr_offset = (params->m_crOffset << (bitDepth - 8)) - (1 << bitDepth);

  int rounding_offset = (1 << (params->m_scalingShift - 1));

  int apply_y = params->m_numYPoints > 0 ? 1 : 0;
  int apply_cb =
      (params->m_numCbPoints > 0 || params->m_chromaScalingFromLuma) > 0 ? 1
                                                                          : 0;
  int apply_cr =
      (params->m_numCrPoints > 0 || params->m_chromaScalingFromLuma) > 0 ? 1
                                                                          : 0;

  if (params->m_chromaScalingFromLuma) {
    cb_mult = 0;        // fixed scale
    cb_luma_mult = 64;  // fixed scale
    cb_offset = 0;

    cr_mult = 0;        // fixed scale
    cr_luma_mult = 64;  // fixed scale
    cr_offset = 0;
  }

  int min_luma, max_luma, min_chroma, max_chroma;

  if (params->m_clipToRestrictedRange) {
    min_luma = kMinLumaLegalRange << (bitDepth - 8);
    max_luma = kMaxLumaLegalRange << (bitDepth - 8);

    if (mc_identity) {
      min_chroma = kMinLumaLegalRange << (bitDepth - 8);
      max_chroma = kMaxLumaLegalRange << (bitDepth - 8);
    } else {
      min_chroma = kMinChromaLegalRange << (bitDepth - 8);
      max_chroma = kMaxChromaLegalRange << (bitDepth - 8);
    }
  } else {
    min_luma = min_chroma = 0;
    max_luma = max_chroma = (256 << (bitDepth - 8)) - 1;
  }

  for (int i = 0; i < (half_luma_height << (1 - chromaSubsampY)); i++) {
    for (int j = 0; j < (half_luma_width << (1 - chromaSubsampX)); j++) {
      int average_luma = 0;
      if (chromaSubsampX) {
        average_luma = (luma[(i << chromaSubsampY) * lumaStride +
                             (j << chromaSubsampX)] +
                        luma[(i << chromaSubsampY) * lumaStride +
                             (j << chromaSubsampX) + 1] +
                        1) >>
                       1;
      } else {
        average_luma = luma[(i << chromaSubsampY) * lumaStride + j];
      }

      if (apply_cb) {
        cb[i * chromaStride + j] = iClip(
            cb[i * chromaStride + j] +
                ((ScaleLUT(&m_scalingLutCb[0],
                            iClip(((average_luma * cb_luma_mult +
                                    cb_mult * cb[i * chromaStride + j]) >>
                                   6) +
                                      cb_offset,
                                  0, (256 << (bitDepth - 8)) - 1),
                            bitDepth) *
                      cb_grain[i * chromaGrainStride + j] +
                  rounding_offset) >>
                 params->m_scalingShift),
            min_chroma, max_chroma);
      }
      if (apply_cr) {
        cr[i * chromaStride + j] = iClip(
            cr[i * chromaStride + j] +
                ((ScaleLUT(&m_scalingLutCr[0],
                            iClip(((average_luma * cr_luma_mult +
                                    cr_mult * cr[i * chromaStride + j]) >>
                                   6) +
                                      cr_offset,
                                  0, (256 << (bitDepth - 8)) - 1),
                            bitDepth) *
                      cr_grain[i * chromaGrainStride + j] +
                  rounding_offset) >>
                 params->m_scalingShift),
            min_chroma, max_chroma);
      }
    }
  }

  if (apply_y) {
    for (int i = 0; i < (half_luma_height << 1); i++) {
      for (int j = 0; j < (half_luma_width << 1); j++) {
        luma[i * lumaStride + j] =
            iClip(luma[i * lumaStride + j] +
                      ((ScaleLUT(&m_scalingLutY[0], luma[i * lumaStride + j],
                                  bitDepth) *
                            luma_grain[i * lumaGrainStride + j] +
                        rounding_offset) >>
                       params->m_scalingShift),
                  min_luma, max_luma);
      }
    }
  }
}

void AddAV1FilmGrain::copyRect(uint8_t *inp, int src_stride, uint8_t *out,
                      int dst_stride, int width, int height,
                      int use_high_bit_depth) {
  int hbd_coeff = use_high_bit_depth ? 2 : 1;
  while (height) {
    memcpy(out, inp, width * sizeof(uint8_t) * hbd_coeff);
    inp += src_stride;
    out += dst_stride;
    --height;
  }
  return;
}

void AddAV1FilmGrain::copyArea(int *inp, int src_stride, int *out, int dst_stride,
                      int width, int height) {
  while (height) {
    memcpy(out, inp, width * sizeof(*inp));
    inp += src_stride;
    out += dst_stride;
    --height;
  }
  return;
}

void AddAV1FilmGrain::extendEven(uint8_t *out, int dst_stride, int width, int height,
                        int use_high_bit_depth) {
  if ((width & 1) == 0 && (height & 1) == 0) return;
  if (use_high_bit_depth) {
    uint16_t *dst16 = (uint16_t *)out;
    int dst16_stride = dst_stride / 2;
    if (width & 1) {
      for (int i = 0; i < height; ++i)
        dst16[i * dst16_stride + width] = dst16[i * dst16_stride + width - 1];
    }
    width = (width + 1) & (~1);
    if (height & 1) {
      memcpy(&dst16[height * dst16_stride], &dst16[(height - 1) * dst16_stride],
             sizeof(*dst16) * width);
    }
  } else {
    if (width & 1) {
      for (int i = 0; i < height; ++i)
        out[i * dst_stride + width] = out[i * dst_stride + width - 1];
    }
    width = (width + 1) & (~1);
    if (height & 1) {
      memcpy(&out[height * dst_stride], &out[(height - 1) * dst_stride],
             sizeof(*out) * width);
    }
  }
}

void AddAV1FilmGrain::verBoundaryOverlap(int *left_block, int left_stride,
                                 int *right_block, int right_stride,
                                 int *dst_block, int dst_stride, int width,
                                 int height) {
  if (width == 1) {
    while (height) {
      *dst_block = iClip((*left_block * 23 + *right_block * 22 + 16) >> 5, m_grainMin, m_grainMax);
      left_block += left_stride;
      right_block += right_stride;
      dst_block += dst_stride;
      --height;
    }
    return;
  } else if (width == 2) {
    while (height) {
      dst_block[0] = iClip((27 * left_block[0] + 17 * right_block[0] + 16) >> 5, m_grainMin, m_grainMax);
      dst_block[1] = iClip((17 * left_block[1] + 27 * right_block[1] + 16) >> 5, m_grainMin, m_grainMax);
      left_block += left_stride;
      right_block += right_stride;
      dst_block += dst_stride;
      --height;
    }
    return;
  }
}


void AddAV1FilmGrain::horBoundaryOverlap(int *top_block, int top_stride,
                                 int *bottom_block, int bottom_stride,
                                 int *dst_block, int dst_stride, int width,
                                 int height) {
  if (height == 1) {
    while (width) {
      *dst_block = iClip((*top_block * 23 + *bottom_block * 22 + 16) >> 5, m_grainMin, m_grainMax);
      ++top_block;
      ++bottom_block;
      ++dst_block;
      --width;
    }
    return;
  } else if (height == 2) {
    while (width) {
      dst_block[0] = iClip((27 * top_block[0] + 17 * bottom_block[0] + 16) >> 5, m_grainMin, m_grainMax);
      dst_block[dst_stride] = iClip((17 * top_block[top_stride] + 27 * bottom_block[bottom_stride] + 16) >> 5, m_grainMin, m_grainMax);
      ++top_block;
      ++bottom_block;
      ++dst_block;
      --width;
    }
    return;
  }
}


int AddAV1FilmGrain::process(AV1FGParams *params, Frame *out,
                       Frame *inp) {
  uint8_t *luma = NULL, *cb = NULL, *cr = NULL;
  int height, width, lumaStride = 0, chromaStride = 0;
  int use_high_bit_depth = 0;
  int chromaSubsampX = 0;
  int chromaSubsampY = 0;
  int mc_identity = (inp->m_colorSpace == CM_RGB) ? 1 : 0;

  out->m_frameNo = inp->m_frameNo;
  out->m_isAvailable = TRUE;

  switch (inp->m_chromaFormat) {
    case CF_400:
      chromaSubsampX = 1;
      chromaSubsampY = 1;
      break;
    case CF_420:
      chromaSubsampX = 1;
      chromaSubsampY = 1;
      break;
    case CF_422:
      chromaSubsampX = 1;
      chromaSubsampY = 0;
      break;
    case CF_444:
      chromaSubsampX = 0;
      chromaSubsampY = 0;      
      break;
    case CF_UNKNOWN:
    default:
      printf("AddAV1FilmGrain::Process not supported\n");
      return -1;
      break;
  }
  
  if (inp->equalType(out)) {
    if (inp->m_isFloat == TRUE) {
      // not supported here
      //for (int c = Y_COMP; c < inp->m_noComponents; c++)
        //addNoiseData (inp->m_floatComp[c], out->m_floatComp[c], inp->m_compSize[c], inp->m_maxPelValue[c]);
    }
    else if (inp->m_bitDepth > 8) {
      for (int c = Y_COMP; c < inp->m_noComponents; c++)
        addNoiseData (inp->m_ui16Comp[c], out->m_ui16Comp[c], inp->m_compSize[c], inp->m_maxPelValue[c]);
    }
    else { // 8 bit data
      use_high_bit_depth = 0;

      for (int c = Y_COMP; c < inp->m_noComponents; c++)
        addNoiseData (inp->m_comp[c], out->m_comp[c], inp->m_compSize[c], inp->m_maxPelValue[c]);
    }
  }
  else {
    printf("AddAV1FilmGrain::Output frame buffer of different type than input frame buffer. Check your implementation\n");
  }

  width  = inp->m_width[0] % 2 ? inp->m_width[0] + 1 : inp->m_width[0];
  height = inp->m_height[0] % 2 ? inp->m_height[0] + 1 : inp->m_height[0];
/*
  copyRect(inp->planes[AOM_PLANE_Y], inp->stride[AOM_PLANE_Y],
            out->planes[AOM_PLANE_Y], out->stride[AOM_PLANE_Y], inp->m_width[0],
            inp->m_height[0], use_high_bit_depth);
  */
  // Note that out is already assumed to be aligned to even.
  /*
  extendEven(out->planes[AOM_PLANE_Y], out->stride[AOM_PLANE_Y], inp->m_width[0] ,
              inp->m_height[0], use_high_bit_depth);
*/
  if (inp->m_chromaFormat != CF_400) {
   /* copyRect(inp->planes[AOM_PLANE_U], inp->stride[AOM_PLANE_U],
              out->planes[AOM_PLANE_U], out->stride[AOM_PLANE_U],
              width >> chromaSubsampX, height >> chromaSubsampY,
              use_high_bit_depth);

    copyRect(inp->planes[AOM_PLANE_V], inp->stride[AOM_PLANE_V],
              out->planes[AOM_PLANE_V], out->stride[AOM_PLANE_V],
              width >> chromaSubsampX, height >> chromaSubsampY,
              use_high_bit_depth);
    */
  }
/*
  luma = out->planes[AOM_PLANE_Y];
  cb = out->planes[AOM_PLANE_U];
  cr = out->planes[AOM_PLANE_V];
*/
  // luma and chroma strides in samples
  
  //lumaStride = out->stride[AOM_PLANE_Y] >> use_high_bit_depth;
  //chromaStride = out->stride[AOM_PLANE_U] >> use_high_bit_depth;

  return AddFilmGrainRun(
      params, luma, cb, cr, height, width, lumaStride, chromaStride,
      use_high_bit_depth, chromaSubsampY, chromaSubsampX, mc_identity);
}

int AddAV1FilmGrain::AddFilmGrainRun(AV1FGParams *params, uint8_t *luma,
                           uint8_t *cb, uint8_t *cr, int height, int width,
                           int lumaStride, int chromaStride,
                           int use_high_bit_depth, int chromaSubsampY,
                           int chromaSubsampX, int mc_identity) {
  //int **m_predPosLuma = NULL;
  //int **m_predPosChroma = NULL;
  int *lumaGrainBlock = NULL;
  int *cbGrainBlock = NULL;
  int *crGrainBlock = NULL;

  int *yLineBuf = NULL;
  int *cbLineBuf = NULL;
  int *crLineBuf = NULL;

  int *yColBuf = NULL;
  int *cbColBuf = NULL;
  int *crColBuf = NULL;

  m_randomRegister = params->m_randomSeed;

  int leftPad = 3;
  int right_pad = 3;  // padding to offset for AR coefficients
  int topPad = 3;
  int bottom_pad = 0;

  int ar_padding = 3;  // maximum lag used for stabilization of AR coefficients

  m_lumaSubblockSizeY = 32;
  m_lumaSubblockSizeX = 32;

  m_chromaSubblockSizeY = m_lumaSubblockSizeY >> chromaSubsampY;
  m_chromaSubblockSizeX = m_lumaSubblockSizeX >> chromaSubsampX;

  // Initial padding is only needed for generation of
  // film grain templates (to stabilize the AR process)
  // Only a 64x64 luma and 32x32 chroma part of a template
  // is used later for adding grain, padding can be discarded

  int lumaBlockSizeY =
      topPad + 2 * ar_padding + m_lumaSubblockSizeY * 2 + bottom_pad;
  int lumaBlockSizeX = leftPad + 2 * ar_padding + m_lumaSubblockSizeX * 2 +
                          2 * ar_padding + right_pad;

  int chromaBlockSizeY = topPad + (2 >> chromaSubsampY) * ar_padding +
                            m_chromaSubblockSizeY * 2 + bottom_pad;
  int chromaBlockSizeX = leftPad + (2 >> chromaSubsampX) * ar_padding +
                            m_chromaSubblockSizeX * 2 +
                            (2 >> chromaSubsampX) * ar_padding + right_pad;

  int lumaGrainStride = lumaBlockSizeX;
  int chromaGrainStride = chromaBlockSizeX;

  int overlap = params->m_overlapFlag;
  int bitDepth = params->m_bitDepth;

  const int grainCenter = 128 << (bitDepth - 8);
  m_grainMin = 0 - grainCenter;
  m_grainMax = grainCenter - 1;

  initArrays(params, lumaStride, chromaStride,
             lumaBlockSizeY * lumaBlockSizeX,
             chromaBlockSizeY * chromaBlockSizeX, 
             chromaSubsampY, chromaSubsampX);

  if (generateLumaGrainBlock(params, 
                             lumaBlockSizeY, lumaBlockSizeX,
                             lumaGrainStride, 
                             leftPad, topPad, 
                             right_pad, bottom_pad))
    return -1;

  if (generateChromaGrainBlocks(
          params,
          lumaGrainStride, chromaBlockSizeY, chromaBlockSizeX,
          chromaGrainStride, leftPad, topPad, right_pad, bottom_pad,
          chromaSubsampY, chromaSubsampX))
    return -1;

  initScalingFunction(params->m_scalingPointsY, params->m_numYPoints,
                        &m_scalingLutY[0]);

  if (params->m_chromaScalingFromLuma) {
    m_scalingLutCb.assign(m_scalingLutY.begin(), m_scalingLutY.end());
    m_scalingLutCr.assign(m_scalingLutY.begin(), m_scalingLutY.end());
  } else {
    initScalingFunction(params->m_scalingPointsCb, params->m_numCbPoints,
                          &m_scalingLutCb[0]);
    initScalingFunction(params->m_scalingPointsCr, params->m_numCrPoints,
                          &m_scalingLutCr[0]);
  }
  
  for (int y = 0; y < height / 2; y += (m_lumaSubblockSizeY >> 1)) {
    initRandomGenerator(y * 2, params->m_randomSeed);

    for (int x = 0; x < width / 2; x += (m_lumaSubblockSizeX >> 1)) {
      int offsetY = getRandomNumber(8);
      int offsetX = (offsetY >> 4) & 15;
      offsetY &= 15;

      int lumaOffsetY = leftPad + 2 * ar_padding + (offsetY << 1);
      int lumaOffsetX = topPad + 2 * ar_padding + (offsetX << 1);

      int chromaOffsetY = topPad + (2 >> chromaSubsampY) * ar_padding +
                            offsetY * (2 >> chromaSubsampY);
      int chromaOffsetX = leftPad + (2 >> chromaSubsampX) * ar_padding +
                            offsetX * (2 >> chromaSubsampX);

      if (overlap && x) {
        verBoundaryOverlap(
            yColBuf, 2,
            lumaGrainBlock + lumaOffsetY * lumaGrainStride +
                lumaOffsetX,
            lumaGrainStride, yColBuf, 2, 2,
            iMin(m_lumaSubblockSizeY + 2, height - (y << 1)));

        verBoundaryOverlap(
            cbColBuf, 2 >> chromaSubsampX,
            cbGrainBlock + chromaOffsetY * chromaGrainStride +
                chromaOffsetX,
            chromaGrainStride, cbColBuf, 2 >> chromaSubsampX,
            2 >> chromaSubsampX,
            iMin(m_chromaSubblockSizeY + (2 >> chromaSubsampY),
                   (height - (y << 1)) >> chromaSubsampY));

        verBoundaryOverlap(
            crColBuf, 2 >> chromaSubsampX,
            crGrainBlock + chromaOffsetY * chromaGrainStride +
                chromaOffsetX,
            chromaGrainStride, crColBuf, 2 >> chromaSubsampX,
            2 >> chromaSubsampX,
            iMin(m_chromaSubblockSizeY + (2 >> chromaSubsampY),
                   (height - (y << 1)) >> chromaSubsampY));

        int i = y ? 1 : 0;

        if (use_high_bit_depth) {
          addNoiseToBlock(
              params,
              (uint16_t *)luma + ((y + i) << 1) * lumaStride + (x << 1),
              (uint16_t *)cb +
                  ((y + i) << (1 - chromaSubsampY)) * chromaStride +
                  (x << (1 - chromaSubsampX)),
              (uint16_t *)cr +
                  ((y + i) << (1 - chromaSubsampY)) * chromaStride +
                  (x << (1 - chromaSubsampX)),
              lumaStride, chromaStride, yColBuf + i * 4,
              cbColBuf + i * (2 - chromaSubsampY) * (2 - chromaSubsampX),
              crColBuf + i * (2 - chromaSubsampY) * (2 - chromaSubsampX),
              2, (2 - chromaSubsampX),
              iMin(m_lumaSubblockSizeY >> 1, height / 2 - y) - i, 1,
              bitDepth, chromaSubsampY, chromaSubsampX, mc_identity);
        } else {
          addNoiseToBlock(
              params, luma + ((y + i) << 1) * lumaStride + (x << 1),
              cb + ((y + i) << (1 - chromaSubsampY)) * chromaStride +
                  (x << (1 - chromaSubsampX)),
              cr + ((y + i) << (1 - chromaSubsampY)) * chromaStride +
                  (x << (1 - chromaSubsampX)),
              lumaStride, chromaStride, yColBuf + i * 4,
              cbColBuf + i * (2 - chromaSubsampY) * (2 - chromaSubsampX),
              crColBuf + i * (2 - chromaSubsampY) * (2 - chromaSubsampX),
              2, (2 - chromaSubsampX),
              iMin(m_lumaSubblockSizeY >> 1, height / 2 - y) - i, 1,
              bitDepth, chromaSubsampY, chromaSubsampX, mc_identity);
        }
      }

      if (overlap && y) {
        if (x) {
          horBoundaryOverlap(yLineBuf + (x << 1), lumaStride, yColBuf, 2,
                               yLineBuf + (x << 1), lumaStride, 2, 2);

          horBoundaryOverlap(cbLineBuf + x * (2 >> chromaSubsampX),
                               chromaStride, cbColBuf, 2 >> chromaSubsampX,
                               cbLineBuf + x * (2 >> chromaSubsampX),
                               chromaStride, 2 >> chromaSubsampX,
                               2 >> chromaSubsampY);

          horBoundaryOverlap(crLineBuf + x * (2 >> chromaSubsampX),
                               chromaStride, crColBuf, 2 >> chromaSubsampX,
                               crLineBuf + x * (2 >> chromaSubsampX),
                               chromaStride, 2 >> chromaSubsampX,
                               2 >> chromaSubsampY);
        }

        horBoundaryOverlap(
            yLineBuf + ((x ? x + 1 : 0) << 1), lumaStride,
            lumaGrainBlock + lumaOffsetY * lumaGrainStride +
                lumaOffsetX + (x ? 2 : 0),
            lumaGrainStride, yLineBuf + ((x ? x + 1 : 0) << 1), lumaStride,
            iMin(m_lumaSubblockSizeX - ((x ? 1 : 0) << 1),
                   width - ((x ? x + 1 : 0) << 1)),
            2);

        horBoundaryOverlap(
            cbLineBuf + ((x ? x + 1 : 0) << (1 - chromaSubsampX)),
            chromaStride,
            cbGrainBlock + chromaOffsetY * chromaGrainStride +
                chromaOffsetX + ((x ? 1 : 0) << (1 - chromaSubsampX)),
            chromaGrainStride,
            cbLineBuf + ((x ? x + 1 : 0) << (1 - chromaSubsampX)),
            chromaStride,
            iMin(m_chromaSubblockSizeX -
                       ((x ? 1 : 0) << (1 - chromaSubsampX)),
                   (width - ((x ? x + 1 : 0) << 1)) >> chromaSubsampX),
            2 >> chromaSubsampY);

        horBoundaryOverlap(
            crLineBuf + ((x ? x + 1 : 0) << (1 - chromaSubsampX)),
            chromaStride,
            crGrainBlock + chromaOffsetY * chromaGrainStride +
                chromaOffsetX + ((x ? 1 : 0) << (1 - chromaSubsampX)),
            chromaGrainStride,
            crLineBuf + ((x ? x + 1 : 0) << (1 - chromaSubsampX)),
            chromaStride,
            iMin(m_chromaSubblockSizeX -
                       ((x ? 1 : 0) << (1 - chromaSubsampX)),
                   (width - ((x ? x + 1 : 0) << 1)) >> chromaSubsampX),
            2 >> chromaSubsampY);

        if (use_high_bit_depth) {
          addNoiseToBlock(
              params, (uint16_t *)luma + (y << 1) * lumaStride + (x << 1),
              (uint16_t *)cb + (y << (1 - chromaSubsampY)) * chromaStride +
                  (x << ((1 - chromaSubsampX))),
              (uint16_t *)cr + (y << (1 - chromaSubsampY)) * chromaStride +
                  (x << ((1 - chromaSubsampX))),
              lumaStride, chromaStride, yLineBuf + (x << 1),
              cbLineBuf + (x << (1 - chromaSubsampX)),
              crLineBuf + (x << (1 - chromaSubsampX)), lumaStride,
              chromaStride, 1,
              iMin(m_lumaSubblockSizeX >> 1, width / 2 - x), bitDepth,
              chromaSubsampY, chromaSubsampX, mc_identity);
        } else {
          addNoiseToBlock(
              params, luma + (y << 1) * lumaStride + (x << 1),
              cb + (y << (1 - chromaSubsampY)) * chromaStride +
                  (x << ((1 - chromaSubsampX))),
              cr + (y << (1 - chromaSubsampY)) * chromaStride +
                  (x << ((1 - chromaSubsampX))),
              lumaStride, chromaStride, yLineBuf + (x << 1),
              cbLineBuf + (x << (1 - chromaSubsampX)),
              crLineBuf + (x << (1 - chromaSubsampX)), lumaStride,
              chromaStride, 1,
              iMin(m_lumaSubblockSizeX >> 1, width / 2 - x), bitDepth,
              chromaSubsampY, chromaSubsampX, mc_identity);
        }
      }

      int i = overlap && y ? 1 : 0;
      int j = overlap && x ? 1 : 0;

      if (use_high_bit_depth) {
        addNoiseToBlock(
            params,
            (uint16_t *)luma + ((y + i) << 1) * lumaStride + ((x + j) << 1),
            (uint16_t *)cb +
                ((y + i) << (1 - chromaSubsampY)) * chromaStride +
                ((x + j) << (1 - chromaSubsampX)),
            (uint16_t *)cr +
                ((y + i) << (1 - chromaSubsampY)) * chromaStride +
                ((x + j) << (1 - chromaSubsampX)),
            lumaStride, chromaStride,
            lumaGrainBlock + (lumaOffsetY + (i << 1)) * lumaGrainStride +
                lumaOffsetX + (j << 1),
            cbGrainBlock +
                (chromaOffsetY + (i << (1 - chromaSubsampY))) *
                    chromaGrainStride +
                chromaOffsetX + (j << (1 - chromaSubsampX)),
            crGrainBlock +
                (chromaOffsetY + (i << (1 - chromaSubsampY))) *
                    chromaGrainStride +
                chromaOffsetX + (j << (1 - chromaSubsampX)),
            lumaGrainStride, chromaGrainStride,
            iMin(m_lumaSubblockSizeY >> 1, height / 2 - y) - i,
            iMin(m_lumaSubblockSizeX >> 1, width / 2 - x) - j, bitDepth,
            chromaSubsampY, chromaSubsampX, mc_identity);
      } else {
        addNoiseToBlock(
            params, luma + ((y + i) << 1) * lumaStride + ((x + j) << 1),
            cb + ((y + i) << (1 - chromaSubsampY)) * chromaStride +
                ((x + j) << (1 - chromaSubsampX)),
            cr + ((y + i) << (1 - chromaSubsampY)) * chromaStride +
                ((x + j) << (1 - chromaSubsampX)),
            lumaStride, chromaStride,
            lumaGrainBlock + (lumaOffsetY + (i << 1)) * lumaGrainStride +
                lumaOffsetX + (j << 1),
            cbGrainBlock +
                (chromaOffsetY + (i << (1 - chromaSubsampY))) *
                    chromaGrainStride +
                chromaOffsetX + (j << (1 - chromaSubsampX)),
            crGrainBlock +
                (chromaOffsetY + (i << (1 - chromaSubsampY))) *
                    chromaGrainStride +
                chromaOffsetX + (j << (1 - chromaSubsampX)),
            lumaGrainStride, chromaGrainStride,
            iMin(m_lumaSubblockSizeY >> 1, height / 2 - y) - i,
            iMin(m_lumaSubblockSizeX >> 1, width / 2 - x) - j, bitDepth,
            chromaSubsampY, chromaSubsampX, mc_identity);
      }

      if (overlap) {
        if (x) {
          // Copy overlapped column bufer to line buffer
          copyArea(yColBuf + (m_lumaSubblockSizeY << 1), 2,
                    yLineBuf + (x << 1), lumaStride, 2, 2);

          copyArea(
              cbColBuf + (m_chromaSubblockSizeY << (1 - chromaSubsampX)),
              2 >> chromaSubsampX,
              cbLineBuf + (x << (1 - chromaSubsampX)), chromaStride,
              2 >> chromaSubsampX, 2 >> chromaSubsampY);

          copyArea(
              crColBuf + (m_chromaSubblockSizeY << (1 - chromaSubsampX)),
              2 >> chromaSubsampX,
              crLineBuf + (x << (1 - chromaSubsampX)), chromaStride,
              2 >> chromaSubsampX, 2 >> chromaSubsampY);
        }

        // Copy grain to the line buffer for overlap with a bottom block
        copyArea(
            lumaGrainBlock +
                (lumaOffsetY + m_lumaSubblockSizeY) * lumaGrainStride +
                lumaOffsetX + ((x ? 2 : 0)),
            lumaGrainStride, yLineBuf + ((x ? x + 1 : 0) << 1), lumaStride,
            iMin(m_lumaSubblockSizeX, width - (x << 1)) - (x ? 2 : 0), 2);

        copyArea(cbGrainBlock +
                      (chromaOffsetY + m_chromaSubblockSizeY) *
                          chromaGrainStride +
                      chromaOffsetX + (x ? 2 >> chromaSubsampX : 0),
                  chromaGrainStride,
                  cbLineBuf + ((x ? x + 1 : 0) << (1 - chromaSubsampX)),
                  chromaStride,
                  iMin(m_chromaSubblockSizeX,
                         ((width - (x << 1)) >> chromaSubsampX)) -
                      (x ? 2 >> chromaSubsampX : 0),
                  2 >> chromaSubsampY);

        copyArea(crGrainBlock +
                      (chromaOffsetY + m_chromaSubblockSizeY) *
                          chromaGrainStride +
                      chromaOffsetX + (x ? 2 >> chromaSubsampX : 0),
                  chromaGrainStride,
                  crLineBuf + ((x ? x + 1 : 0) << (1 - chromaSubsampX)),
                  chromaStride,
                  iMin(m_chromaSubblockSizeX,
                         ((width - (x << 1)) >> chromaSubsampX)) -
                      (x ? 2 >> chromaSubsampX : 0),
                  2 >> chromaSubsampY);

        // Copy grain to the column buffer for overlap with the next block to
        // the right

        copyArea(lumaGrainBlock + lumaOffsetY * lumaGrainStride +
                      lumaOffsetX + m_lumaSubblockSizeX,
                  lumaGrainStride, yColBuf, 2, 2,
                  iMin(m_lumaSubblockSizeY + 2, height - (y << 1)));

        copyArea(cbGrainBlock + chromaOffsetY * chromaGrainStride +
                      chromaOffsetX + m_chromaSubblockSizeX,
                  chromaGrainStride, cbColBuf, 2 >> chromaSubsampX,
                  2 >> chromaSubsampX,
                  iMin(m_chromaSubblockSizeY + (2 >> chromaSubsampY),
                         (height - (y << 1)) >> chromaSubsampY));

        copyArea(crGrainBlock + chromaOffsetY * chromaGrainStride +
                      chromaOffsetX + m_chromaSubblockSizeX,
                  chromaGrainStride, crColBuf, 2 >> chromaSubsampX,
                  2 >> chromaSubsampX,
                  iMin(m_chromaSubblockSizeY + (2 >> chromaSubsampY),
                         (height - (y << 1)) >> chromaSubsampY));
      }
    }
  }

  return 0;
}

void AddAV1FilmGrain::addNoiseData (const uint16 *iData, uint16 *oData, int size, int maxSampleValue)
{
  for (int i = 0; i < size; i++) {
    //*oData++ = iClip(*iData++ + (uint16) generateGaussianNoise(m_variance, m_mean), 0, maxSampleValue);
  }
}

void AddAV1FilmGrain::addNoiseData (const imgpel *iData, imgpel *oData, int size, int maxSampleValue)
{
  for (int i = 0; i < size; i++) {
    //*oData++ = iClip(*iData++ + (imgpel) generateGaussianNoise(m_variance, m_mean), 0, maxSampleValue);
  }
}

void AddAV1FilmGrain::addNoiseData (const float *iData, float *oData, int size, double maxSampleValue)
{
  for (int i = 0; i < size; i++) {
    //double noise = generateGaussianNoise(m_variance, m_mean);
    //*oData++ = (float) ((double) *iData++ + noise);
  }
}


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------


void AddAV1FilmGrain::process ( Frame* out, Frame *inp)
{
  out->m_frameNo = inp->m_frameNo;
  out->m_isAvailable = TRUE;
  
  if (inp->equalType(out)) {
    if (inp->m_isFloat == TRUE) {
      for (int c = Y_COMP; c < inp->m_noComponents; c++)
        addNoiseData (inp->m_floatComp[c], out->m_floatComp[c], inp->m_compSize[c], inp->m_maxPelValue[c]);
    }
    else if (inp->m_bitDepth > 8) {
      for (int c = Y_COMP; c < inp->m_noComponents; c++)
        addNoiseData (inp->m_ui16Comp[c], out->m_ui16Comp[c], inp->m_compSize[c], inp->m_maxPelValue[c]);
    }
    else { // 8 bit data
      for (int c = Y_COMP; c < inp->m_noComponents; c++)
        addNoiseData (inp->m_comp[c], out->m_comp[c], inp->m_compSize[c], inp->m_maxPelValue[c]);
    }
  }
  else {
    printf("AddAV1FilmGrain::Output frame buffer of different type than input frame buffer. Check your implementation\n");
  }
}

void AddAV1FilmGrain::process ( Frame *inp)
{
  if (inp->m_isFloat == TRUE) {
    for (int c = Y_COMP; c < inp->m_noComponents; c++)
      addNoiseData (inp->m_floatComp[c], inp->m_floatComp[c], inp->m_compSize[c], inp->m_maxPelValue[c]);
  }
  else if (inp->m_bitDepth > 8) {
    for (int c = Y_COMP; c < inp->m_noComponents; c++)
      addNoiseData (inp->m_ui16Comp[c], inp->m_ui16Comp[c], inp->m_compSize[c], inp->m_maxPelValue[c]);
  }
  else { // 8 bit data
    for (int c = Y_COMP; c < inp->m_noComponents; c++)
      addNoiseData (inp->m_comp[c], inp->m_comp[c], inp->m_compSize[c], inp->m_maxPelValue[c]);
  }
}
} // namespace hdrtoolslib


//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
