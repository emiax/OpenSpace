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

#ifndef __RENDERABLERINGS_H__
#define __RENDERABLERINGS_H__

#include <openspace/rendering/renderable.h>

#include <openspace/properties/stringproperty.h>
#include <openspace/properties/vectorproperty.h>
#include <openspace/util/updatestructures.h>

namespace ghoul {
    namespace filesystem {
        class File;
    }
    namespace opengl {
        class ProgramObject;
        class Texture;
    }
}

namespace openspace {
class RenderableRings : public Renderable {
public:
    RenderableRings(const ghoul::Dictionary& dictionary);

    bool initialize() override;
    bool deinitialize() override;

    bool isReady() const override;

    void render(const RenderData& data) override;
    void update(const UpdateData& data) override;

private:
    void loadTexture();
    void createPlane();

    properties::StringProperty _texturePath;
    properties::Vec2Property _size;
    properties::Vec2Property _offset;
    properties::FloatProperty _nightFactor;
    properties::FloatProperty _transparency;

    std::unique_ptr<ghoul::opengl::ProgramObject> _shader;
    std::unique_ptr<ghoul::opengl::Texture> _texture;
    std::unique_ptr<ghoul::filesystem::File> _textureFile;

    bool _textureIsDirty;
    GLuint _quad;
    GLuint _vertexPositionBuffer;
    bool _planeIsDirty;
    
    std::string _frame;
    glm::mat3 _orientation;
    glm::mat3 _state;
    
    std::string _body;
    glm::vec3 _sunPosition;
    bool _hasSunPosition;
};

} // namespace openspace

#endif // __RENDERABLERINGS_H__
