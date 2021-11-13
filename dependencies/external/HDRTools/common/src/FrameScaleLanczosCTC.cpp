/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Apple Inc.
 * <ORGANIZATION> = Apple Inc.
 * <YEAR> = 2020
 *
 * Copyright (c) 2020, Apple Inc.
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
 * \file FrameScaleLanczosCTC.cpp
 *
 * \brief
 *    Rescale using the SHVC specified interpolation methods discussed in m24499
 *    Code taken from SHVCSoftware/trunk/source/App/TAppDownConvert/DownConvert.cpp
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "Global.H"
#include "FrameScaleLanczosCTC.H"
#include <string.h>
#include <cfloat>

namespace hdrtoolslib {

//--
// Constants
//--
static const int crDownScaleFilter[10][16][100] =
{ 
  { // 1x
    {1, 1, 0, 16384}
  },
  { // 3/4x 
    { 14, 3, -6, 80, -161, -121, 961, -2080, 2560, 12208, 4586, -2321, 709, 136, -280, 107, 0 },
    { 14, 3, -5, 18, 41, -412, 930, -957, -667, 10352, 8669, -1667, -304, 713, -430, 96, 2 },
    { 14, 3, -3, 117, -378, 429, 275, -2208, 6683, 11570, 768, -1587, 1027, -309, -49, 46, 0 }
  }, // 2/3x
  {
    { 16, 2, -7, 27, 47, -384, 511, 319, -2071, 3160, 10801, 5633, -1772, -364, 800, -330, -59, 66, 0 },
    { 16, 2, -7,  0, 98, -197, -141, 915, -1139, -768, 7973, 9799,   929, -1789, 764, 163, -324, 98, 3   }
  },
  { // 1/2x
    { 20, 1, -9, 42, 49, -222, -150, 555, 342, -1239, -816, 3759, 7980, 6369, 1085, -1543, -418, 678, 187, -287, -69, 77, 5 }
  }, 
  { // 3/8x
    { 27, 3, -12, 27, 57, -38, -198, -135, 252, 514, 71, -864, -1057, 594, 3598, 5894, 5657, 3081, 186, -1137, -719, 200, 509, 177, -172, -178, -12, 58, 19, 0 },
    { 27, 3, -10, 2, 44, 43, -96, -216, -31, 388, 458, -232, -1085, -729, 1526, 4560, 6138, 4982, 2034, -479, -1146, -396, 395, 442, 33, -212, -125, 29, 51, 6 },
    { 27, 3, -8, 12, 56, 11, -154, -197, 103, 484, 308, -560, -1165, -173, 2556, 5351, 6056, 4095, 1043, -922, -988, -75, 497, 323, -88, -211, -66, 52, 36, 0}
  },
  { // 1/3x
    { 30, 1, -14, 6, 42, 40, -63, -183, -126, 169, 443, 288, -376, -995, -681, 997, 3422, 5209, 5209, 3422, 997, -681, -995, -376, 288, 443, 169, -126, -183, -63, 40, 42, 6}
  },
  { // 1/4x
    { 40, 1, -19, 1, 15, 36, 32, -16, -93, -143, -103, 45, 237, 342, 236, -101, -529, -774, -555, 256, 1529, 2888, 3857, 4071, 3449, 2228, 854, -222, -736, -694, -319, 92, 321, 310, 143, -41, -137, -126, -54, 14, 39, 26, 6}
  },
  { // 3/16x
    { 54, 3, -25, 1, 10, 23, 29, 20, -9, -52, -92, -108, -82, -8, 98, 201, 256, 222, 85, -136, -378, -552, -561, -336, 142, 826, 1606, 2335, 2862, 3073, 2921, 2440, 1735, 953, 244, -274, -541, -567, -415, -177, 52, 207, 257, 215, 117, 8, -73, -107, -97, -59, -15, 16, 29, 25, 13, 2, 0},
    { 54, 3, -20, 0, 7, 19, 28, 25, 3, -37, -80, -107, -96, -37, 61, 170, 246, 245, 142, -56, -300, -508, -581, -440, -44, 580, 1344, 2107, 2717, 3042, 3011, 2632, 1987, 1212, 463, -128, -481, -582, -480, -260, -18, 166, 252, 237, 153, 43, -50, -101, -104, -73, -29, 8, 27, 27, 17, 5, 0},
    { 54, 3, -14, 4, 15, 26, 28, 12, -22, -66, -101, -105, -62, 25, 135, 227, 256, 188, 18, -219, -449, -577, -514, -204, 351, 1082, 1862, 2540, 2971, 3062, 2794, 2224, 1475, 701, 46, -392, -574, -532, -340, -96, 114, 235, 252, 186, 79, -23, -90, -108, -86, -44, -3, 23, 29, 21, 9, 1, 0}
  },
  { // 1/6x
    { 60, 1, -28, 2, 9, 19, 26, 23, 6, -23, -58, -87, -96, -74, -18, 62, 148, 212, 226, 170, 42, -139, -331, -476, -514, -397, -101, 361, 942, 1561, 2123, 2534, 2724, 2660, 2353, 1856, 1253, 642, 111, -272, -477, -512, -413, -237, -44, 114, 207, 227, 185, 106, 20, -50, -89, -95, -74, -40, -7, 16, 26, 23, 14, 5, 0}
  },
  { // 1/8x
    { 80, 1, -38, 0, 2, 6, 12, 17, 19, 17, 10, -4, -22, -42, -60, -70, -71, -57, -29, 11, 59, 108, 147, 169, 166, 132, 66, -25, -132, -240, -329, -382, -379, -307, -160, 62, 348, 678, 1027, 1365, 1661, 1886, 2019, 2046, 1965, 1784, 1520, 1199, 852, 509, 198, -58, -243, -352, -388, -362, -288, -187, -77, 24, 103, 153, 171, 161, 129, 84, 34, -11, -45, -66, -72, -66, -51, -32, -13, 4, 14, 19, 19, 15, 9, 4, 1 }
  }
};

static const int lumaDownScaleFilter[10][16][100] =
{ 
  { // 1x
    {1, 1, 0, 16384}
  },
  { // 3/4x
    { 14, 3, -6, 63, -103, -225, 1017, -1858, 1626, 11966, 5636, -2314, 513, 281, -333, 115, 0 },
    { 14, 3, -5, 8, 73, -431, 833, -627, -1224, 9560, 9560, -1224, -627, 833, -431, 73, 8 },
    { 14, 3, -4, 115, -333, 281, 513, -2314, 5636, 11966, 1626, -1858, 1017, -225, -103, 63, 0 }
  },
  { // 2/3x
    { 16, 2, -7, 12, 80, -366,  337, 577, -1991,  1993, 10418,  6844, -1361, -752, 886, -252, -126, 85, 0 },
    { 16, 2, -7,  0, 85, -126, -252, 886,  -752, -1361,  6844, 10418, 1993, -1991, 577, 337, -366, 80, 12 }
  },
  { // 1/2x
    { 20, 1, -9, 20, 74, -148, -248, 384, 573, -854, -1329, 2370, 7350, 7350, 2370, -1329, -854, 573, 384, -248, -148, 74, 20}
  },
  { // 3/8
    { 27, 3, -12, 9, 54, 20, -140, -206, 68, 465, 355, -478, -1161, -333, 2294, 5174, 6107, 4332, 1281, -833, -1041, -152, 481, 356, -61, -215, -81, 48, 40, 1},
    { 27, 3, -9, 23, 58, -24, -189, -155, 215, 514, 137, -793, -1104, 384, 3341, 5785, 5785, 3341, 384, -1104, -793, 137, 514, 215, -155, -189, -24, 58, 23, 0 },
    { 27, 3, -7, 1, 40, 48, -81, -215, -61, 356, 481, -152, -1041, -833, 1281, 4332, 6107, 5174, 2294, -333, -1161, -478, 355, 465, 68, -206, -140, 20, 54, 9 }
  },
  { // 1/3x
    { 30, 1, -13, 23, 52, 0, -132, -187, 0, 335, 438, 0, -747, -1002, 0, 2194, 4486, 5465, 4486, 2194, 0, -1002, -747, 0, 438, 335, 0, -187, -133, 0, 52, 23, 0}
  },
  { // 1/4x
    { 40, 1, -18, 3, 21, 38, 24, -34, -111, -144, -75, 94, 278, 339, 171, -209, -619, -772, -408, 542, 1879, 3185, 3990, 3990, 3185, 1879, 542, -408, -772, -619, -209, 171, 339, 278, 94, -75, -144, -111, -34, 24, 38, 21, 3}
  },
  { // 3/16x
    { 54, 3, -24, 1, 10, 22, 29, 21, -6, -48, -89, -108, -86, -16, 89, 194, 254, 229, 100, -116, -359, -542, -569, -365, 93, 763, 1541, 2280, 2829, 3069, 2947, 2491, 1799, 1017, 297, -240, -529, -573, -432, -198, 35, 198, 257, 221, 126, 17, -68, -106, -99, -63, -19, 15, 29, 25, 14, 3, 0},
    { 54, 3, -19, 0, 6, 18, 28, 26, 5, -33, -77, -106, -99, -44, 52, 161, 242, 249, 154, -37, -280, -494, -582, -461, -87, 522, 1278, 2048, 2675, 3028, 3028, 2675, 2048, 1278, 522, -87, -461, -582, -494, -280, -37, 154, 249, 242, 161, 52, -44, -99, -106, -77, -33, 5, 26, 28, 18, 6, 0},
    { 54, 3, -13, 3, 14, 25, 29, 15, -19, -63, -99, -106, -68, 17, 126, 221, 257, 198, 35, -198, -432, -573, -529, -240, 297, 1017, 1799, 2491, 2947, 3069, 2829, 2280, 1541, 763, 93, -365, -569, -542, -359, -116, 100, 229, 254, 194, 89, -16, -86, -108, -89, -48, -6, 21, 29, 22, 10, 1, 0}
  },
  { // 1/6x
    { 60, 1, -27, 1, 7, 16, 24, 25, 12, -15, -49, -81, -96, -83, -35, 41, 128, 200, 229, 191, 80, -91, -285, -447, -518, -443, -192, 232, 790, 1408, 1993, 2450, 2700, 2700, 2450, 1993, 1408, 790, 232, -192, -443, -518, -447, -285, -91, 80, 191, 229, 200, 128, 41, -35, -83, -96, -81, -49, -15, 12, 25, 24, 16, 7, 1}
  },
  { // 1/8x
    { 80, 1, -36, 0, 3, 8, 13, 18, 19, 16, 7, -8, -27, -47, -63, -72, -69, -52, -20, 23, 72, 119, 155, 171, 160, 118, 46, -51, -159, -264, -347, -387, -368, -278, -111, 128, 427, 765, 1114, 1444, 1725, 1928, 2036, 2036, 1928, 1725, 1444, 1114, 765, 427, 128, -111, -278, -368, -387, -347, -264, -159, -51, 46, 118, 160, 171, 155, 119, 72, 23, -20, -52, -69, -72, -63, -47, -27, -8, 7, 16, 19, 18, 13, 8, 3, 0}
  }
};
static const int crUpScaleFilter[10][16][100] =
{ 
  { // 1x
    {1, 1, 0, 16384}
  },
  { // 4/3x 
    { 10, 4, -5, 3, -65, 181, -405, 1024, 16276, -888, 366, -162, 54 },
    { 10, 4, -4, 61, -373, 950, -2116, 6118, 13799, -2945, 1283, -549, 156 },
    { 10, 4, -3, 143, -573, 1371, -3098, 11559, 8916, -2776, 1240, -504, 106 },
    { 10, 4, -2, 128, -412, 946, -2221, 15424, 3415, -1275, 572, -216, 23 }
  },
  { // 3/2x
    { 10, 3, -5,  5,  -88,  244,  -546,  1392, 16192, -1152,  478, -211,  70 },
    { 10, 3, -4, 99, -486, 1201, -2685,  8454, 11966, -3108, 1373, -577, 147 },
    { 10, 3, -4, 148, -496, 1146, -2658, 14700,  4740, -1709,  767, -295,  41 }
  },
  { // 2x 
    { 10, 2, -5, 10, -137,  374,  -835,  2168, 15953, -1630,  684, -301, 98 },
    { 10, 2, -4, 153, -575, 1357, -3088, 12744,  7521, -2479, 1111, -444, 84 }
  },
  { // 8/3x
    { 10, 8, -5,  16, -176, 473, -1055, 2780, 15714, -1945, 822, -360, 115 },
    { 10, 8, -4, 139, -458, 1054, -2458, 15085, 4069, -1494, 670, -255, 32 },
    { 10, 8, -4,  95, -476, 1180, -2637, 8221, 12166, -3108, 1371, -577, 149 },
    { 10, 8, -4, 1, -31, 88, -198, 496, 16357, -462, 189, -84, 28 },
    { 10, 8, -3, 156, -565, 1327, -3033, 13290, 6818, -2305, 1034, -410, 72 },
    { 10, 8, -3, 51, -335, 861, -1917, 5424, 14271, -2820, 1222, -526, 153 },
    { 10, 8, -2, 77, -234, 532, -1278, 16141, 1582, -618, 276, -100, 6 },
    { 10, 8, -2, 136, -564, 1357, -3057, 10926, 9601, -2894, 1291, -529, 117 }
  },
  { // 3x
    { 10, 3, -5, 19, -189, 506, -1128, 2989, 15622, -2041, 865, -378, 119     },
    { 10, 3, -4, 119, -378, 865, -2041, 15622, 2989, -1128, 506, -189, 19     },
    { 10, 3, -4, 127, -549, 1330, -2988, 10272, 10272, -2988, 1330, -549, 127 }
  },
  { // 4x
    { 10, 4, -5, 23, -216, 572, -1275, 3415, 15424, -2221, 946, -412, 128 },
    { 10, 4, -4, 54, -162, 366, -888, 16276, 1024, -405, 181, -65, 3 },
    { 10, 4, -4, 156, -549, 1283, -2945, 13799, 6118, -2116, 950, -373, 61},
    { 10, 4, -4, 106, -504, 1240, -2776, 8916, 11559, -3098, 1371, -573, 143 }
  },
  { // 16/3x
    { 10, 16, -5, 27, -235, 621, -1385, 3740, 15260, -2344, 1002, -436, 134 },
    { 10, 16, -5, 0, -15, 43, -98, 244, 16377, -235, 96, -43, 15 },
    { 10, 16, -4, 122, -387, 886, -2088, 15575, 3095, -1165, 522, -196, 20 },
    { 10, 16, -4, 155, -571, 1344, -3065, 13021, 7170, -2394, 1073, -427, 78 },
    { 10, 16, -4, 112, -517, 1267, -2838, 9260, 11245, -3081, 1365, -569, 140 },
    { 10, 16, -4, 46, -315, 814, -1814, 5081, 14491, -2744, 1186, -512, 151 },
    { 10, 16, -4, 4, -82, 228, -510, 1299, 16215, -1088, 451, -199, 66 },
    { 10, 16, -3, 88, -268, 610, -1459, 16053, 1872, -726, 325, -119, 8 },
    { 10, 16, -3, 155, -538, 1254, -2887, 14040, 5770, -2018, 906, -354, 56 },
    { 10, 16, -3, 131, -557, 1345, -3026, 10602, 9939, -2944, 1312, -540, 122 },
    { 10, 16, -3, 67, -392, 992, -2212, 6468, 13549, -2993, 1307, -558, 156 },
    { 10, 16, -3, 13, -156, 423, -945, 2471, 15840, -1792, 755, -331, 106 },
    { 10, 16, -2, 42, -123, 279, -679, 16323, 756, -301, 134, -48, 1 },
    { 10, 16, -2, 144, -478, 1102, -2563, 14898, 4403, -1602, 719, -275, 36 },
    { 10, 16, -2, 146, -576, 1373, -3107, 11866, 8570, -2709, 1211, -491, 101 },
    { 10, 16, -2, 89, -460, 1146, -2560, 7871, 12459, -3102, 1366, -577, 152 }
  },
  { // 6x
    { 10,  6, -5,  29, -242, 638, -1422, 3849, 15203, -2383, 1019, -443, 136 },
    { 10,  6, -5,   1, -42, 119, -266, 668, 16336, -608, 249, -110, 37 },
    { 10,  6, -4,  98, -301, 684, -1630, 15953, 2168, -835, 374, -137, 10},
    { 10,  6, -4, 154, -534, 1244, -2865, 14118, 5655, -1985, 891, -348, 54},
    { 10,  6, -4, 138, -568, 1363, -3074, 11140, 9374, -2857, 1275, -521, 114 },
    { 10,  6, -4,  84, -444, 1111, -2479, 7521, 12744, -3088, 1357, -575, 153 }
  },
  { // 8x
    { 10, 8, -5,  32, -255, 670, -1494, 4069, 15085, -2458, 1054, -458, 139},
    { 10, 8, -5,   6, -100, 276, -618, 1582, 16141, -1278, 532, -234, 77 },
    { 10, 8, -4,  28, -84, 189, -462, 16357, 496, -198, 88, -31, 1 },
    { 10, 8, -4, 115, -360, 822, -1945, 15714, 2780, -1055, 473, -176, 16 },
    { 10, 8, -4, 153, -526, 1222, -2820, 14271, 5424, -1917, 861, -335, 51 },
    { 10, 8, -4, 149, -577, 1371, -3108, 12166, 8221, -2637, 1180, -476, 95 },
    { 10, 8, -4, 117, -529, 1291, -2894, 9601, 10926, -3057, 1357, -564, 136 },
    { 10, 8, -4,  72, -410, 1034, -2305, 6818, 13290, -3033, 1327, -565, 156 }
  }
};

static const int lumaUpScaleFilter[10][16][100] =
{ 
  { // 1x
    {1, 1, 0, 16384}
  },
  { // 4/3x
    { 10, 4, -5,  10, -137, 374, -835, 2168, 15953, -1630, 684, -301, 98 },
    { 10, 4, -4,  84, -444, 1111, -2479, 7521, 12744, -3088, 1357, -575, 153 },
    { 10, 4, -3, 153, -575, 1357, -3088, 12744, 7521, -2479, 1111, -444, 84 },
    { 10, 4, -2,  98, -301, 684, -1630, 15953, 2168, -835, 374, -137, 10 }
  },
  { // 3/2x
    { 10, 3, -5, 19, -189, 506, -1128, 2989, 15622, -2041, 865, -378, 119},
    { 10, 3, -4, 127, -549, 1330, -2988, 10272, 10272, -2988, 1330, -549, 127},
    { 10, 3, -4, 119, -378, 865, -2041, 15622, 2989, -1128, 506, -189, 19 }
  },
  { // 2x 
    {  10, 2, -5,  41, -295, 767, -1709, 4740, 14700, -2658, 1146, -496, 148},
    {  10, 2, -4, 148, -496, 1146, -2658, 14700, 4740, -1709, 767, -295, 41 },
  },
  { // 8/3x
    { 10, 8, -5, 61, -373, 950, -2116, 6118, 13799, -2945, 1283, -549, 156 },
    { 10, 8, -4, 54, -162, 366, -888, 16276, 1024, -405, 181, -65, 3 },
    { 10, 8, -4, 143, -573, 1371, -3098, 11559, 8916, -2776, 1240, -504, 106 },
    { 10, 8, -4, 23, -216, 572, -1275, 3415, 15424, -2221, 946, -412, 128 }, 
    { 10, 8, -3, 128, -412, 946, -2221, 15424, 3415, -1275, 572, -216, 23 },
    { 10, 8, -3, 106, -504, 1240, -2776, 8916, 11559, -3098, 1371, -573, 143 },
    { 10, 8, -3, 3, -65, 181, -405, 1024, 16276, -888, 366, -162, 54 },
    { 10, 8, -2, 156, -549, 1283, -2945, 13799, 6118, -2116, 950, -373, 61 }
  },  
  { // 3x
    { 10, 3, -5, 69, -398, 1006, -2243, 6585, 13464, -3008, 1314, -561, 156 },
    { 10, 3, -4, 0, 0, 0, 0, 16384, 0, 0, 0, 0, 0 },
    { 10, 3, -4, 156, -561, 1314, -3008, 13464, 6585, -2243, 1006, -398, 69 }
  },
  { // 4x
    { 10, 4, -5, 84, -444, 1111, -2479, 7521, 12744, -3088, 1357, -575, 153 },
    { 10, 4, -5, 10, -137, 374, -835, 2168, 15953, -1630, 684, -301, 98 },
    { 10, 4, -4, 98, -301, 684, -1630, 15953, 2168, -835, 374, -137, 10 },
    { 10, 4, -4, 153, -575, 1357, -3088, 12744, 7521, -2479, 1111, -444, 84 }
  },
  { // 16/3x
    { 10, 16, -5, 95, -476, 1180, -2637, 8221, 12166, -3108, 1371, -577, 149 },
    { 10, 16, -5, 32, -255, 670, -1494, 4069, 15085, -2458, 1054, -458, 139 },
    { 10, 16, -5, 1, -31, 88, -198, 496, 16357, -462, 189, -84, 28 },
    { 10, 16, -4, 115, -360, 822, -1945, 15714, 2780, -1055, 473, -176, 16 },
    { 10, 16, -4, 156, -565, 1327, -3033, 13290, 6818, -2305, 1034, -410, 72 },
    { 10, 16, -4, 117, -529, 1291, -2894, 9601, 10926, -3057, 1357, -564, 136 },
    { 10, 16, -4, 51, -335, 861, -1917, 5424, 14271, -2820, 1222, -526, 153 },
    { 10, 16, -4, 6, -100, 276, -618, 1582, 16141, -1278, 532, -234, 77 },
    { 10, 16, -3, 77, -234, 532, -1278, 16141, 1582, -618, 276, -100, 6 },
    { 10, 16, -3, 153, -526, 1222, -2820, 14271, 5424, -1917, 861, -335, 51},
    { 10, 16, -3, 136, -564, 1357, -3057, 10926, 9601, -2894, 1291, -529, 117 },
    { 10, 16, -3, 72, -410, 1034, -2305, 6818, 13290, -3033, 1327, -565, 156},
    { 10, 16, -3, 16, -176, 473, -1055, 2780, 15714, -1945, 822, -360, 115},
    { 10, 16, -2, 28, -84, 189, -462, 16357, 496, -198, 88, -31, 1 },
    { 10, 16, -2, 139, -458, 1054, -2458, 15085, 4069, -1494, 670, -255, 32 },
    { 10, 16, -2, 149, -577, 1371, -3108, 12166, 8221, -2637, 1180, -476, 95 }
  },  
  { // 6x
    { 10, 6, -5,  99, -486, 1201, -2685, 8454, 11966, -3108, 1373, -577, 147 },
    { 10, 6, -5,  41, -295, 767, -1709, 4740, 14700, -2658, 1146, -496, 148 },
    { 10, 6, -5,   5, -88, 244, -546, 1392, 16192, -1152, 478, -211, 70 },
    { 10, 6, -4,  70, -211, 478, -1152, 16192, 1392, -546, 244, -88, 5 },
    { 10, 6, -4, 148, -496, 1146, -2658, 14700, 4740, -1709, 767, -295, 41},
    { 10, 6, -4, 147, -577, 1373, -3108, 11966, 8454, -2685, 1201, -486, 99}
  },
  { // 8x
    { 10, 8, -5, 106, -504, 1240, -2776, 8916, 11559, -3098, 1371, -573, 143 },
    { 10, 8, -5,  61, -373, 950, -2116, 6118, 13799, -2945, 1283, -549, 156 },
    { 10, 8, -5,  23, -216, 572, -1275, 3415, 15424, -2221, 946, -412, 128 },
    { 10, 8, -5,   3, -65, 181, -405, 1024, 16276, -888, 366, -162, 54},
    { 10, 8, -4,  54, -162, 366, -888, 16276, 1024, -405, 181, -65, 3 },
    { 10, 8, -4, 128, -412, 946, -2221, 15424, 3415, -1275, 572, -216, 23},
    { 10, 8, -4, 156, -549, 1283, -2945, 13799, 6118, -2116, 950, -373, 61 },
    { 10, 8, -4, 143, -573, 1371, -3098, 11559, 8916, -2776, 1240, -504, 106 }
  }
};

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------
bool AreSame(double a, double b)
{
    return fabs(a - b) < DBL_EPSILON;
}

int FrameScaleLanczosCTC::SelectFilter(double scaleFactor) {
  int filter = 0;
  
  if ( AreSame(scaleFactor, 8.0 )) 
    filter = 9;
  else if ( AreSame(scaleFactor, 6.0 ))  
    filter = 8;
  else if ( AreSame(scaleFactor, 16.0 / 3.0 ))  
    filter = 7;
  else if ( AreSame(scaleFactor, 4.0 ))  
    filter = 6;
  else if ( AreSame(scaleFactor, 3.0 ))  
    filter = 5;
  else if ( AreSame(scaleFactor, 8.0 / 3.0 ))  
    filter = 4;
  else if ( AreSame(scaleFactor, 2.0 ))  
    filter = 3;
  else if ( AreSame(scaleFactor, 3.0 / 2.0))  
    filter = 2;
  else if ( AreSame(scaleFactor, 4.0 / 3.0))  
    filter = 1;

  return filter;
}

void FrameScaleLanczosCTC::AssignFilters(double factor, 
                   int &filterTaps,
                   std::vector<int> &filterOffset,
                   std::vector<double> &filterCoeffs,
                   std::vector<int> &filterIntCoeff,
                   int oDim, int iDim,
                   bool isChroma, int dimension
                   )
{
  
  // Allocate filter memory
  int index = SelectFilter(factor < 1.0 ? 1.0 / factor : factor );

  if (isChroma && dimension == 1 ) {    
    filterTaps = (factor > 1.0) ?  crDownScaleFilter[ index ][ 0 ][ 0 ] : crUpScaleFilter[ index ][ 0 ][ 0 ];
    
    filterOffset.resize(oDim * filterTaps);
    // We basically allocate filter coefficients for all target positions.
    // This is done once and saves us time from deriving the proper filter for each position.
    filterCoeffs.resize(oDim * filterTaps);
    filterIntCoeff.resize(oDim * filterTaps);
        
    
    PrepareFilterCoefficients((double *) &filterCoeffs[0], 
                              (int *) &filterIntCoeff[0], 
                              (int *) &filterOffset[0], 
                              factor, 
                              filterTaps, 
                              m_offsetX, 
                              oDim, 
                              isChroma, 
                              (factor > 1.0) ?  crDownScaleFilter[ index ] : crUpScaleFilter[ index ]);
  }
  else {
    filterTaps = (factor > 1.0) ?  lumaDownScaleFilter[ index ][ 0 ][ 0 ] : lumaUpScaleFilter[ index ][ 0 ][ 0 ];
    
    filterOffset.resize(oDim * filterTaps);
    // We basically allocate filter coefficients for all target positions.
    // This is done once and saves us time from deriving the proper filter for each position.
    filterCoeffs.resize(oDim * filterTaps);
    filterIntCoeff.resize(oDim * filterTaps);

    // Finally prepare the filter coefficients     
    PrepareFilterCoefficients((double *) &filterCoeffs[0], 
                              (int *) &filterIntCoeff[0], 
                              (int *) &filterOffset[0], 
                              factor, 
                              filterTaps, 
                              m_offsetX, 
                              oDim, 
                              isChroma, 
                              (factor > 1.0) ?  lumaDownScaleFilter[ index ] : lumaUpScaleFilter[ index ]);
  }
}

FrameScaleLanczosCTC::FrameScaleLanczosCTC(int iWidth, int iHeight, int oWidth, int oHeight) {

  m_factorY = (double)(iHeight) / (double) (oHeight);
  m_factorX = (double)(iWidth ) / (double) (oWidth );
  
  m_offsetX = m_factorX > 1.0 ? 1 / m_factorX : m_factorX;
  m_offsetY = m_factorY > 1.0 ? 1 / m_factorY : m_factorX;
  m_chromaOffsetX = m_factorX > 1.0 ? 1 / m_factorX : m_factorX;
  m_chromaOffsetY = m_factorY > 1.0 ? 1 / m_factorY : m_factorX;
  
  AssignFilters(m_factorX, 
                m_filterTapsX,
                m_filterOffsetsX,
                m_filterCoeffsX,
                m_filterIntCoeffsX,
                oWidth, iWidth,
                FALSE, 0
                );

  AssignFilters(m_factorY, 
                m_filterTapsY,
                m_filterOffsetsY,
                m_filterCoeffsY,
                m_filterIntCoeffsY,
                oHeight, iHeight,
                FALSE, 1
                );

  AssignFilters(m_factorX, 
                m_chromaFilterTapsX,
                m_chromaFilterOffsetsX,
                m_chromaFilterCoeffsX,
                m_chromaFilterIntCoeffsX,
                oWidth / 2, iWidth / 2,
                TRUE, 0
                );

  AssignFilters(m_factorY, 
                m_chromaFilterTapsY,
                m_chromaFilterOffsetsY,
                m_chromaFilterCoeffsY,
                m_chromaFilterIntCoeffsY,
                oHeight / 2, iHeight / 2,
                TRUE, 1
                );
}


//-----------------------------------------------------------------------------
// Private Methods
//-----------------------------------------------------------------------------



void FrameScaleLanczosCTC::PrepareFilterCoefficients(double *filterCoeffs, int *iFilterCoeffs, int *filterOffsets, double factor, int filterTaps, double offset, int oSize, bool isChroma, const int currFilter[16][100])
{
  int x;
  int tapIndex, index;
  double coeffSum;
    
  for ( x = 0, index = 0; x < oSize; x++, index += filterTaps ) {
    
    int iPhase    = x  % currFilter[0][1];
    
    //printf("phase %d\n", iPhase);
    coeffSum = 0.0;
    for ( int k = 0; k < filterTaps; k++ ) {
      tapIndex = index + k;
      filterOffsets[tapIndex] = currFilter[iPhase][2] + k;
      filterCoeffs[tapIndex] = (double) currFilter[iPhase][k + 3];
      iFilterCoeffs[tapIndex] = currFilter[iPhase][k + 3];
      coeffSum += filterCoeffs[tapIndex];   
      //printf(" %d ", iFilterCoeffs[tapIndex] );

    }
    //printf("\n");
    for ( tapIndex = index; tapIndex < index + filterTaps; tapIndex++ ) {
      filterCoeffs[ tapIndex ] /= coeffSum;
    }
  }
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void FrameScaleLanczosCTC::process ( Frame* out, const Frame *inp)
{
  if (( out->m_isFloat != inp->m_isFloat ) || (( inp->m_isFloat == 0 ) && ( out->m_bitDepth != inp->m_bitDepth ))) {
    fprintf(stderr, "Error: trying to copy frames of different data types. \n");
    exit(EXIT_FAILURE);
  }
    
  int c;
  
  out->m_frameNo = inp->m_frameNo;
  out->m_isAvailable = TRUE;
  
  for (c = Y_COMP; c <= V_COMP; c++) {
    out->m_minPelValue[c]  = inp->m_minPelValue[c];
    out->m_midPelValue[c]  = inp->m_midPelValue[c];
    out->m_maxPelValue[c]  = inp->m_maxPelValue[c];
  }
  
  // Below code needs to be modified for 4:4:4 and different chroma positions. TBD
  if (out->m_isFloat == TRUE) {    // floating point data. Currently not modified since not used.    
    for (c = Y_COMP; c <= V_COMP; c++) {
      filter( inp->m_floatComp[c], out->m_floatComp[c], inp->m_width[c], inp->m_height[c], out->m_width[c], out->m_height[c], (double) out->m_minPelValue[c], (double) out->m_maxPelValue[c] );
    }
  }
  else if (out->m_bitDepth == 8) {   // 8 bit data
    filter( inp->m_comp[Y_COMP], out->m_comp[Y_COMP], inp->m_width[Y_COMP], inp->m_height[Y_COMP], out->m_width[Y_COMP], out->m_height[Y_COMP], out->m_minPelValue[Y_COMP], out->m_maxPelValue[Y_COMP], FALSE );
    
    for (c = U_COMP; c <= V_COMP; c++) {
      filter( inp->m_comp[c], out->m_comp[c], inp->m_width[c], inp->m_height[c], out->m_width[c], out->m_height[c], out->m_minPelValue[c], out->m_maxPelValue[c], TRUE );
    }
  }
  else { // 16 bit data
    filter( inp->m_ui16Comp[Y_COMP], out->m_ui16Comp[Y_COMP], inp->m_width[Y_COMP], inp->m_height[Y_COMP], out->m_width[Y_COMP], out->m_height[Y_COMP], out->m_minPelValue[Y_COMP], out->m_maxPelValue[Y_COMP], FALSE );
    
    for (c = U_COMP; c <= V_COMP; c++) {
      filter( inp->m_ui16Comp[c], out->m_ui16Comp[c], inp->m_width[c], inp->m_height[c], out->m_width[c], out->m_height[c], out->m_minPelValue[c], out->m_maxPelValue[c], TRUE );
    }
  }
}

} // namespace hdrtoolslib


//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
