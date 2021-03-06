/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2015                                                                    *
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

#ifndef __SHENBRICKSELECTOR_H__
#define __SHENBRICKSELECTOR_H__

#include <vector>
#include <modules/multiresvolume/rendering/brickselector.h>
#include <modules/multiresvolume/rendering/tsp.h>
#include <modules/multiresvolume/rendering/brickcover.h>

namespace openspace {


class ShenBrickSelector : public BrickSelector {
public:
    ShenBrickSelector(TSP* tsp, float spatialTolerance, float temporalTolerance);
    ~ShenBrickSelector();
    void setSpatialTolerance(float spatialTolerance);
    void setTemporalTolerance(float temporalTolerance);
    void selectBricks(int timestep,
                      std::vector<int>& bricks);
private:
    TSP* _tsp;
    float _spatialTolerance;
    float _temporalTolerance;

    void traverseOT(int timestep,
                    unsigned int brickIndex,
                    BrickCover coveredBricks,
                    std::vector<int>& bricks);

    void traverseBST(int timestep,
                     unsigned int brickIndex,
                     unsigned int bstRootBrickIndex,
                     int timeSpanStart,
                     int timeSpanEnd,
                     BrickCover coveredBricks,
                     std::vector<int>& bricks);

    void selectBricks(int timestep,
                      unsigned int brickIndex,
                      unsigned int bstRootBrickIndex,
                      int timeSpanStart,
                      int timeSpanEnd,
                      BrickCover coveredBricks,
                      std::vector<int>& bricks);

    int linearCoords(int x, int y, int z);

    void selectCover(BrickCover coveredBricks, unsigned int brickIndex, std::vector<int>& bricks);
};

} // namespace openspace

#endif // __SHENBRICKSELECTOR_H__
