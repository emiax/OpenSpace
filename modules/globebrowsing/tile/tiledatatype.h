/*****************************************************************************************
*                                                                                       *
* OpenSpace                                                                             *
*                                                                                       *
* Copyright (c) 2014-2016                                                               *
*                                                                                       *
* Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
* software and associated documentation files (the "Software"), to deal in the Software *
* without restriction, including without limitation the rights to use, copy, modify,    *
* merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
* permit persons to whom the Software is furnished to do so, subject to the following   *
* conditions:                                                                           *
*                                                                                       *
* The above copyright notice and this permission notice shall be included in all copies *
* or substantial portions of the Software.                                              *
*                                                                                       *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
* INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
* PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
* CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
* OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
****************************************************************************************/

#ifndef __TILE_DATA_TYPE_H__
#define __TILE_DATA_TYPE_H__

#include <ghoul/opengl/texture.h>

#include <modules/globebrowsing/geometry/geodetic2.h>
#include <modules/globebrowsing/tile/tileioresult.h>

#include "gdal_priv.h"


namespace openspace {
   
    struct TileDataType {

        static GLuint getOpenGLDataType(GDALDataType gdalType);

        static GDALDataType getGdalDataType(GLuint glType);

        static TextureFormat getTextureFormat(int rasterCount, GDALDataType gdalType);

        static size_t getMaximumValue(GDALDataType gdalType);

        static size_t numberOfBytes(GDALDataType gdalType);

        static float interpretFloat(GDALDataType gdalType, const char* src);

    };

} // namespace openspace





#endif  // __TILE_DATA_TYPE_H__