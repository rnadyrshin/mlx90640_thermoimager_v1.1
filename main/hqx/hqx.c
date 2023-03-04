/*
 * Copyright (C) 2010 Cameron Zemek ( grom@zeminvaders.net)
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

//#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include "hqx.h"


void hqx_scale(uint8_t scaleBy, uint16_t width, uint16_t height, int16_t *pInput, int16_t *pOutput)
{
    if (scaleBy != 2 && scaleBy != 3 && scaleBy != 4)
    	return;

    switch (scaleBy)
    {
    case 2:
        hq2x_32(pInput, pOutput, width, height);
        break;
    case 3:
        hq3x_32(pInput, pOutput, width, height);
        break;
    case 4:
    default:
        hq4x_32(pInput, pOutput, width, height);
        break;
    }
/*
    // Copy destData into image
    ilTexImage(width * scaleBy, height * scaleBy, 0, 4, IL_BGRA, IL_UNSIGNED_BYTE, destData);

    // Free image data
    free(srcData);
    free(destData);

    // Save image
    ilConvertImage(IL_BGRA, IL_UNSIGNED_BYTE); // No alpha channel
    ilHint(IL_COMPRESSION_HINT, IL_USE_COMPRESSION);
    ilEnable(IL_FILE_OVERWRITE);
    ILboolean saved = ilSaveImage(szFilenameOut);
    */
}
