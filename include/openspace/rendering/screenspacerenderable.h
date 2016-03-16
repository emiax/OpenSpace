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
 
#ifndef __SCREENSPACERENDERABLE_H__
#define __SCREENSPACERENDERABLE_H__
#include <ghoul/opengl/programobject.h>
#include <openspace/engine/wrapper/windowwrapper.h>
#include <openspace/properties/propertyowner.h>
#include <openspace/properties/vectorproperty.h>
#include <openspace/properties/scalarproperty.h>
#include <openspace/properties/stringproperty.h>

#ifdef WIN32
#define _USE_MATH_DEFINES
#include <math.h>
#endif

namespace openspace {

class ScreenSpaceRenderable : public properties::PropertyOwner {
public:
	ScreenSpaceRenderable();
	~ScreenSpaceRenderable();

	virtual void render() = 0;
	virtual bool initialize() = 0;
	virtual bool deinitialize() = 0;
	virtual void update() = 0;
	virtual bool isReady() const = 0;
	bool isEnabled() const;
	void move(glm::vec2 v){
		if(_useFlatScreen.value()){
			glm::vec2 pos = _euclideanPosition.value();
			pos += v;
			_euclideanPosition.set(pos);
		}else{
			glm::vec2 pos = _sphericalPosition.value();
			pos += v;
			_sphericalPosition.set(pos);
		}
	};

protected:
	void createPlane();
	glm::vec2 toEuclidean(glm::vec2 polar, float radius);
	glm::vec2 toSpherical(glm::vec2 euclidean);

	properties::BoolProperty _enabled;
	properties::BoolProperty _useFlatScreen;
	properties::Vec2Property _euclideanPosition;
	properties::Vec2Property _sphericalPosition;
	properties::FloatProperty _depth;
	properties::FloatProperty _scale;

	GLuint _quad;
	GLuint _vertexPositionBuffer;
	const std::string _rendererPath;
	ghoul::Dictionary _rendererData;
	std::unique_ptr<ghoul::opengl::ProgramObject> _shader;

	float _radius;
	bool _useEuclideanCoordinates;
	const float _planeDepth = -2.0;
	glm::vec2 _originalViewportSize;
	
};
}  // namespace openspace
#endif  // __SCREENSPACERENDERABLE_H__