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

#include <modules/globebrowsing/tile/pixelregion.h>

#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/assert.h>
#include <iostream>





namespace {
    const std::string _loggerCat = "PixelRegion";
}




namespace openspace {

   

    PixelRegion::PixelRegion()
        : start(0)
        , numPixels(0) 
    {

    }

    PixelRegion::PixelRegion(const PixelRegion& o) 
        : start(o.start)
        , numPixels(o.numPixels) 
    {
    
    }

    PixelRegion::PixelRegion(const PixelCoordinate& pixelStart, const PixelRange& numberOfPixels)
        : start(pixelStart)
        , numPixels(numberOfPixels) 
    {
    
    }




    //////////////////////////////////////////////////////////////////////////////////
    //                                  Mutators                                    //
    //////////////////////////////////////////////////////////////////////////////////

    void PixelRegion::setSide(Side side, int pos) {
        switch (side) {
        case LEFT: setLeft(pos); break;
        case TOP: setTop(pos); break;
        case RIGHT: setRight(pos); break;
        case BOTTOM: setBottom(pos); break;
        }
    }

    void PixelRegion::setLeft(int x) {
        numPixels.x += (start.x - x);
        start.x = x;
    }

    void PixelRegion::setTop(int p) {
        numPixels.y += (start.y - p);
        start.y = p;
    }

    void PixelRegion::setRight(int x) {
        numPixels.x = x - start.x;
    }

    void PixelRegion::setBottom(int y) {
        numPixels.y = y - start.y;
    }


    void PixelRegion::align(Side side, int pos) {
        switch (side) {
        case LEFT: alignLeft(pos); break;
        case TOP: alignTop(pos); break;
        case RIGHT: alignRight(pos); break;
        case BOTTOM: alignBottom(pos); break;
        }
    }

    void PixelRegion::alignLeft(int x) { 
        start.x = x; 
    }
    
    void PixelRegion::alignTop(int y) { 
        start.y = y; 
    }

    void PixelRegion::alignRight(int x) { 
        start.x = x - numPixels.x; 
    }

    void PixelRegion::alignBottom(int y) { 
        start.y = y - numPixels.y; 
    }



    void PixelRegion::scale(const glm::dvec2& s) {
        start = PixelCoordinate(glm::round(s * glm::dvec2(start)));
        numPixels = PixelCoordinate(glm::round(s * glm::dvec2(numPixels)));
    }

    void PixelRegion::scale(double s) {
        scale(glm::dvec2(s));
    }

    void PixelRegion::downscalePow2(int exponent, PixelCoordinate wrt) {
        start += wrt;
        start.x >>= exponent;
        start.y >>= exponent;
        numPixels.x >>= exponent;
        numPixels.y >>= exponent;
        start -= wrt;
    }

    void PixelRegion::upscalePow2(int exponent, PixelCoordinate wrt) {
        start += wrt;
        start.x <<= exponent;
        start.y <<= exponent;
        numPixels.x <<= exponent;
        numPixels.y <<= exponent;
        start -= wrt;
    }



    void PixelRegion::move(Side side, int amount) {
        switch (side) {
        case LEFT: start.x -= amount; break;
        case TOP: start.y -= amount; break;
        case RIGHT: start.x += amount; break;
        case BOTTOM: start.y += amount; break;
        }
    }

    void PixelRegion::pad(const PixelRegion& padding) {
        start += padding.start;
        numPixels += padding.numPixels;
    }

    void PixelRegion::clampTo(const PixelRegion& boundingRegion) {
        start = glm::max(start, boundingRegion.start);
        numPixels = glm::min(end(), boundingRegion.end()) - start;
    }

    void PixelRegion::forceNumPixelToDifferByNearestMultipleOf(unsigned int multiple) {
        ghoul_assert(multiple > 0, "multiple must be 1 or larger");
        int sizeDiff = numPixels.x - numPixels.y;
        if (std::abs(sizeDiff) > 0) {
            if (sizeDiff > 0) {
                numPixels.y += sizeDiff % multiple;
            }
            else {
                numPixels.x += std::abs(sizeDiff) % multiple;
            }
        }
    }

    void PixelRegion::roundUpNumPixelToNearestMultipleOf(unsigned int multiple) {
        ghoul_assert(multiple > 0, "multiple must be 1 or larger");
        numPixels.x += numPixels.x % multiple;
        numPixels.y += numPixels.y % multiple;
    }

    void PixelRegion::roundDownToQuadratic() {
        if (numPixels.x < numPixels.y) {
            numPixels.y = numPixels.x;
        }
        else if (numPixels.x > numPixels.y) {
            numPixels.x = numPixels.y;
        }
    }

    PixelRegion PixelRegion::globalCut(Side side, int p) {
        if (!lineIntersect(side, p)) {
            return PixelRegion({ 0, 0 }, { 0, 0 });
        }

        PixelRegion cutOff(*this);
        int cutSize = 0;
        switch (side) {
        case LEFT:
            setLeft(p);
            cutOff.setRight(p - cutSize);
            break;
        case TOP:
            setTop(p);
            cutOff.setBottom(p - cutSize);
            break;
        case RIGHT:
            setRight(p);
            cutOff.setLeft(p + cutSize);
            break;
        case BOTTOM:
            setBottom(p);
            cutOff.setTop(p + cutSize);
            break;
        }
        return cutOff;
    }

    PixelRegion PixelRegion::localCut(Side side, int p) {
        if (p < 1) {
            return PixelRegion({ 0, 0 }, { 0, 0 });
        }
        else return globalCut(side, edge(side) - edgeDirectionSign(side) * p);
    }


    //////////////////////////////////////////////////////////////////////////////////
    //                                 Accessors                                    //
    //////////////////////////////////////////////////////////////////////////////////

    int PixelRegion::area() const {
        return numPixels.x * numPixels.y;
    }


    int PixelRegion::edge(Side side) const {
        switch (side) {
        case LEFT: return start.x;
        case TOP: return start.y;
        case RIGHT: return start.x + numPixels.x;
        case BOTTOM: return start.y + numPixels.y;
        }
    }

    int PixelRegion::edgeDirectionSign(Side side) const {
        return side < RIGHT ? -1 : 1;
    }

    PixelCoordinate PixelRegion::end() const {
        return start + numPixels;
    }

    //////////////////////////////////////////////////////////////////////////////////
    //                                Comparators                                   //
    //////////////////////////////////////////////////////////////////////////////////


    bool PixelRegion::lineIntersect(Side side, int p) {
        switch (side)
        {
        case openspace::PixelRegion::LEFT:
        case openspace::PixelRegion::RIGHT:
            return start.x <= p && p <= (start.x + numPixels.x);

        case openspace::PixelRegion::TOP:
        case openspace::PixelRegion::BOTTOM:
            return start.y <= p && p <= (start.y + numPixels.y);
        }
    }

    bool PixelRegion::isInside(const PixelRegion& r) const {
        PixelCoordinate e = end();
        PixelCoordinate re = r.end();
        return r.start.x <= start.x && e.x <= re.x
            && r.start.y <= start.y && e.y <= re.y;
    }

    bool PixelRegion::equals(const PixelRegion& r) const {
        return start == r.start && numPixels == r.numPixels;
    }


}  // namespace openspace
