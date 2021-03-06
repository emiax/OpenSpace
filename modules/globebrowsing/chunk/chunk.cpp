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


#include <ghoul/misc/assert.h>

#include <openspace/engine/openspaceengine.h>

#include <modules/globebrowsing/chunk/chunk.h>
#include <modules/globebrowsing/chunk/chunkedlodglobe.h>
#include <modules/globebrowsing/tile/layeredtextures.h>
#include <modules/globebrowsing/tile/tileioresult.h>

#include <algorithm>

namespace {
    const std::string _loggerCat = "Chunk";
}

namespace openspace {

    const float Chunk::DEFAULT_HEIGHT = 0.0f;

    Chunk::Chunk(ChunkedLodGlobe* owner, const ChunkIndex& chunkIndex, bool initVisible)
        : _owner(owner)
        , _surfacePatch(chunkIndex)
        , _index(chunkIndex)
        , _isVisible(initVisible) 
    {

    }

    const GeodeticPatch& Chunk::surfacePatch() const {
        return _surfacePatch;
    }

    ChunkedLodGlobe* const Chunk::owner() const {
        return _owner;
    }

    const ChunkIndex Chunk::index() const {
        return _index;
    }

    bool Chunk::isVisible() const {
        return _isVisible;
    }

    void Chunk::setIndex(const ChunkIndex& index) {
        _index = index;
        _surfacePatch = GeodeticPatch(index);
    }

    void Chunk::setOwner(ChunkedLodGlobe* newOwner) {
        _owner = newOwner;
    }

    Chunk::Status Chunk::update(const RenderData& data) {
        auto savedCamera = _owner->getSavedCamera();
        const Camera& camRef = savedCamera != nullptr ? *savedCamera : data.camera;
        RenderData myRenderData = { camRef, data.position, data.doPerformanceMeasurement };


        _isVisible = true;
        if (_owner->testIfCullable(*this, myRenderData)) {
            _isVisible = false;
            return Status::WANT_MERGE;
        }

        int desiredLevel = _owner->getDesiredLevel(*this, myRenderData);

        if (desiredLevel < _index.level) return Status::WANT_MERGE;
        else if (_index.level < desiredLevel) return Status::WANT_SPLIT;
        else return Status::DO_NOTHING;
    }

    Chunk::BoundingHeights Chunk::getBoundingHeights() const {
        BoundingHeights boundingHeights;
        boundingHeights.max = 0;
        boundingHeights.min = 0;
        boundingHeights.available = false;

        // In the future, this should be abstracted away and more easily queryable.
        // One must also handle how to sample pick one out of multiplte heightmaps
        auto tileProviderManager = owner()->getTileProviderManager();
        
        
        auto heightMapProviders = tileProviderManager->getTileProviderGroup(LayeredTextures::HeightMaps).getActiveTileProviders();
       
        
        size_t HEIGHT_CHANNEL = 0;
        const TileProviderGroup& heightmaps = tileProviderManager->getTileProviderGroup(LayeredTextures::HeightMaps);
        std::vector<TileAndTransform> tiles = TileSelector::getTilesSortedByHighestResolution(heightmaps, _index);
        bool lastHadMissingData = true;
        for (auto tile : tiles) {
            bool goodTile = tile.tile.status == Tile::Status::OK;
            bool hasPreprocessData = tile.tile.preprocessData != nullptr;

            if (goodTile && hasPreprocessData) {
                auto preprocessData = tile.tile.preprocessData;

                if (!boundingHeights.available) {
                    if (preprocessData->hasMissingData[HEIGHT_CHANNEL]) {
                        boundingHeights.min = std::min(DEFAULT_HEIGHT, preprocessData->minValues[HEIGHT_CHANNEL]);
                        boundingHeights.max = std::max(DEFAULT_HEIGHT, preprocessData->maxValues[HEIGHT_CHANNEL]);
                    }
                    else {
                        boundingHeights.min = preprocessData->minValues[HEIGHT_CHANNEL];
                        boundingHeights.max = preprocessData->maxValues[HEIGHT_CHANNEL];
                    }
                    boundingHeights.available = true;
                }
                else {
                    boundingHeights.min = std::min(boundingHeights.min, preprocessData->minValues[HEIGHT_CHANNEL]);
                    boundingHeights.max = std::max(boundingHeights.max, preprocessData->maxValues[HEIGHT_CHANNEL]);
                }
                lastHadMissingData = preprocessData->hasMissingData[HEIGHT_CHANNEL];
            }

            // Allow for early termination
            if (!lastHadMissingData) {
                break;
            }
        }
        
        return boundingHeights;
    }

    std::vector<glm::dvec4> Chunk::getBoundingPolyhedronCorners() const {
        const Ellipsoid& ellipsoid = owner()->ellipsoid();
        const GeodeticPatch& patch = surfacePatch();

        BoundingHeights boundingHeight = getBoundingHeights();

        // assume worst case
        double patchCenterRadius = ellipsoid.maximumRadius();

        double maxCenterRadius = patchCenterRadius + boundingHeight.max;
        Geodetic2 halfSize = patch.halfSize();

        // As the patch is curved, the maximum height offsets at the corners must be long 
        // enough to cover large enough to cover a boundingHeight.max at the center of the 
        // patch.
        // Approximating scaleToCoverCenter by assuming the latitude and longitude angles
        // of "halfSize" are equal to the angles they create from the center of the
        // globe to the patch corners. This is true for the longitude direction when
        // the ellipsoid can be approximated as a sphere and for the latitude for patches
        // close to the equator. Close to the pole this will lead to a bigger than needed
        // value for scaleToCoverCenter. However, this is a simple calculation and a good
        // Approximation.
        double y1 = tan(halfSize.lat);
        double y2 = tan(halfSize.lon);
        double scaleToCoverCenter = sqrt(1 + pow(y1, 2) + pow(y2, 2));
        
        double maxCornerHeight = maxCenterRadius * scaleToCoverCenter - patchCenterRadius;

        bool chunkIsNorthOfEquator = patch.isNorthern();

        // The minimum height offset, however, we can simply 
        double minCornerHeight = boundingHeight.min;
        std::vector<glm::dvec4> corners(8);
        
        Scalar latCloseToEquator = patch.edgeLatitudeNearestEquator();
        Geodetic3 p1Geodetic = { { latCloseToEquator, patch.minLon() }, maxCornerHeight };
        Geodetic3 p2Geodetic = { { latCloseToEquator, patch.maxLon() }, maxCornerHeight };
        
        glm::vec3 p1 = ellipsoid.cartesianPosition(p1Geodetic);
        glm::vec3 p2 = ellipsoid.cartesianPosition(p2Geodetic);
        glm::vec3 p = 0.5f * (p1 + p2);
        Geodetic2 pGeodetic = ellipsoid.cartesianToGeodetic2(p);
        Scalar latDiff = latCloseToEquator - pGeodetic.lat;

        for (size_t i = 0; i < 8; i++) {
            Quad q = (Quad)(i % 4);
            double cornerHeight = i < 4 ? minCornerHeight : maxCornerHeight;
            Geodetic3 cornerGeodetic = { patch.getCorner(q), cornerHeight };
            
            bool cornerIsNorthern = !((i / 2) % 2);
            bool cornerCloseToEquator = chunkIsNorthOfEquator ^ cornerIsNorthern;
            if (cornerCloseToEquator) {
                cornerGeodetic.geodetic2.lat += latDiff;
            }

            corners[i] = dvec4(ellipsoid.cartesianPosition(cornerGeodetic), 1);
        }
        return corners;
    }


} // namespace openspace
