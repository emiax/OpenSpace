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

#include <modules/globebrowsing/tile/tilediskcache.h>

#include <modules/globebrowsing/tile/tileioresult.h>

#include <ghoul/logging/logmanager.h>

#include <sstream>
#include <fstream>
#include <algorithm>

namespace {
    const std::string _loggerCat = "TileDiskCache";
}


namespace openspace {
    const std::string TileDiskCache::CACHE_ROOT = "tilecache";


    TileDiskCache::TileDiskCache(const std::string& name)
        : _name(name)
    {
        if (!FileSystem::isInitialized()) {
            FileSystem::initialize();
        }

        std::string pathToCacheDir = FileSys.pathByAppendingComponent(CACHE_ROOT, name);
        Directory cacheDir(pathToCacheDir, Directory::RawPath::No);
        if (!FileSys.directoryExists(cacheDir)) {
            FileSys.createDirectory(pathToCacheDir, FileSystem::Recursive::Yes);
        }
        _cacheDir = cacheDir;
    }

    bool TileDiskCache::has(const ChunkIndex& chunkIndex) const {
        File metaFile = getMetaDataFile(chunkIndex);
        return FileSys.fileExists(metaFile);
    }


    std::shared_ptr<TileIOResult> TileDiskCache::get(const ChunkIndex& chunkIndex) {
        File metaDataFile = getMetaDataFile(chunkIndex);
        File dataFile = getDataFile(chunkIndex);
        if (FileSys.fileExists(metaDataFile) && FileSys.fileExists(dataFile)) {
            // read meta
            std::ifstream ifsMeta;
            ifsMeta.open(metaDataFile.path(), std::ifstream::in);
            TileIOResult res = TileIOResult::deserializeMetaData(ifsMeta);
            ifsMeta.close();

            // read data
            std::ifstream ifsData;
            ifsData.open(dataFile.path(), std::ifstream::binary);
            char * buffer = new char[res.nBytesImageData];
            ifsData.read(buffer, res.nBytesImageData);
            res.imageData = buffer;

            return std::make_shared<TileIOResult>(res);
        }
        return nullptr;
    }

    bool TileDiskCache::put(const ChunkIndex& chunkIndex, std::shared_ptr<TileIOResult> tileIOResult) {
        File metaDataFile = getMetaDataFile(chunkIndex);
        if (!FileSys.fileExists(metaDataFile)) {
            std::ofstream ofsMeta;
            ofsMeta.open(metaDataFile.path());
            tileIOResult->serializeMetaData(ofsMeta);
            ofsMeta.close();

            std::ofstream ofsData;
            File dataFile = getDataFile(chunkIndex);
            ofsData.open(dataFile.path(), std::ofstream::binary);
            char * data = (char*)tileIOResult->imageData;
            ofsData.write(data, tileIOResult->nBytesImageData);
            ofsData.close();
            return true;
        }
        return false;
    }

    std::string TileDiskCache::getFilePath(const ChunkIndex& chunkIndex) const {
        std::stringstream ss;
        ss << chunkIndex.level;
        ss << "_" << chunkIndex.x;
        ss << "_" << chunkIndex.y;
        std::string filePath = FileSys.pathByAppendingComponent(_cacheDir.path(), ss.str());
        return filePath;
    }

    File TileDiskCache::getMetaDataFile(const ChunkIndex& chunkIndex) const {
        return File(getFilePath(chunkIndex) + ".meta");
    }

    File TileDiskCache::getDataFile(const ChunkIndex& chunkIndex) const {
        return File(getFilePath(chunkIndex) + ".data");
    }



}  // namespace openspace
