/*
 * Copyright (C) 2003 Maxim Stepin ( maxst@hiend3d.com )
 *
 * Copyright (C) 2010 Cameron Zemek ( grom@zeminvaders.net)
 * Copyright (C) 2011 Francois Gannaz <mytskine@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef __HQX_COMMON_H_
#define __HQX_COMMON_H_

#include <stdlib.h>
#include <stdint.h>

#define MASK_2     0x0000FF00
#define MASK_13    0x00FF00FF
#define MASK_RGB   0x00FFFFFF
#define MASK_ALPHA 0xFF000000

#define trV   6

/*
#define Ymask 0x00FF0000
#define Umask 0x0000FF00
#define Vmask 0x000000FF
#define trY   0x00300000
#define trU   0x00000700
#define trV   0x00000006

// RGB to YUV lookup table
extern uint32_t RGBtoYUV[16777216];

static inline uint32_t rgb_to_yuv(uint32_t c)
{
    // Mask against MASK_RGB to discard the alpha channel
    return RGBtoYUV[MASK_RGB & c];
}

// Test if there is difference in color
static inline int yuv_diff(uint32_t yuv1, uint32_t yuv2) {
    return (( abs((yuv1 & Ymask) - (yuv2 & Ymask)) > trY ) ||
            ( abs((yuv1 & Umask) - (yuv2 & Umask)) > trU ) ||
            ( abs((yuv1 & Vmask) - (yuv2 & Vmask)) > trV ) );
}
*/

static inline int Diff(int16_t c1, int16_t c2)
{
    return abs(c1 - c2) > trV;
}

/* Interpolate functions */
/*
static inline uint32_t Interpolate_2(uint32_t c1, int w1, uint32_t c2, int w2, int s)
{
    if (c1 == c2)
    {
        return c1;
    }

    return
        (((((c1 & MASK_ALPHA) >> 24) * w1 + ((c2 & MASK_ALPHA) >> 24) * w2) << (24-s)) & MASK_ALPHA) +
        ((((c1 & MASK_2) * w1 + (c2 & MASK_2) * w2) >> s) & MASK_2)	+
        ((((c1 & MASK_13) * w1 + (c2 & MASK_13) * w2) >> s) & MASK_13);
}

static inline uint32_t Interpolate_3(uint32_t c1, int w1, uint32_t c2, int w2, uint32_t c3, int w3, int s)
{
    return
        (((((c1 & MASK_ALPHA) >> 24) * w1 + ((c2 & MASK_ALPHA) >> 24) * w2 + ((c3 & MASK_ALPHA) >> 24) * w3) << (24-s)) & MASK_ALPHA) +
        ((((c1 & MASK_2) * w1 + (c2 & MASK_2) * w2 + (c3 & MASK_2) * w3) >> s) & MASK_2) +
        ((((c1 & MASK_13) * w1 + (c2 & MASK_13) * w2 + (c3 & MASK_13) * w3) >> s) & MASK_13);
}
*/

static inline void Interp1(int16_t * pc, int16_t c1, int16_t c2)
{
    //*pc = (c1*3+c2) / 4;
	int32_t value = c1;
	value *= 3;
	value += c2;
	value /= 4;
    *pc = (int16_t) value;
}

static inline void Interp2(int16_t * pc, int16_t c1, int16_t c2, int16_t c3)
{
    //*pc = (c1*2+c2+c3) / 4;
	int32_t value = c1;
	value *= 2;
	value += c2;
	value += c3;
	value /= 4;
    *pc = (int16_t) value;
}

static inline void Interp3(int16_t * pc, int16_t c1, int16_t c2)
{
    //*pc = (c1*7+c2) / 8;
	int32_t value = c1;
	value *= 7;
	value += c2;
	value /= 8;
    *pc = (int16_t) value;
}

static inline void Interp4(int16_t * pc, int16_t c1, int16_t c2, int16_t c3)
{
    //*pc = (c1*2+(c2+c3)*7) / 16;
	int32_t value = c1;
	value *= 2;
	int32_t value2 = c2 + c3;
	value2 *= 7;
	value += value2;
	value /= 16;
    *pc = (int16_t) value;
}

static inline void Interp5(int16_t * pc, int16_t c1, int16_t c2)
{
    //*pc = (c1+c2) / 2;
	int32_t value = c1;
	value += c2;
	value /= 2;
    *pc = (int16_t) value;
}

static inline void Interp6(int16_t * pc, int16_t c1, int16_t c2, int16_t c3)
{
    //*pc = (c1*5+c2*2+c3) / 8;
	int32_t value = c1;
	value *= 5;
	int32_t value2 = c2;
	value2 *= 2;
	value += value2;
	value += c3;
	value /= 8;
    *pc = (int16_t) value;
}

static inline void Interp7(int16_t * pc, int16_t c1, int16_t c2, int16_t c3)
{
    //*pc = (c1*6+c2+c3) / 8;
	int32_t value = c1;
	value *= 6;
	value += c2;
	value += c3;
	value /= 8;
    *pc = (int16_t) value;
}

static inline void Interp8(int16_t * pc, int16_t c1, int16_t c2)
{
    //*pc = (c1*5+c2*3) / 8;
	int32_t value = c1;
	value *= 5;
	int32_t value2 = c2;
	value2 *= 3;
	value += value2;
	value /= 8;
    *pc = (int16_t) value;
}

static inline void Interp9(int16_t * pc, int16_t c1, int16_t c2, int16_t c3)
{
    //*pc = (c1*2+(c2+c3)*3) / 8;
	int32_t value = c1;
	value *= 2;
	int32_t value2 = c2;
	value2 += c3;
	value2 *= 3;
	value += value2;
	value /= 8;
    *pc = (int16_t) value;
}

static inline void Interp10(int16_t * pc, int16_t c1, int16_t c2, int16_t c3)
{
    //*pc = (c1*14+c2+c3) / 16;
	int32_t value = c1;
	value *= 14;
	value += c2;
	value += c3;
	value /= 16;
    *pc = (int16_t) value;
}

#endif
