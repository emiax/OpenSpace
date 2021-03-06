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

#include <openspace/documentation/core_registration.h>

#include <openspace/documentation/documentationengine.h>
#include <openspace/engine/configurationmanager.h>
#include <openspace/mission/mission.h>
#include <openspace/rendering/renderable.h>
#include <openspace/rendering/screenspacerenderable.h>
#include <openspace/scene/ephemeris.h>
#include <openspace/scene/rotation.h>
#include <openspace/scene/scene.h>
#include <openspace/scene/scenegraphnode.h>
#include <openspace/scene/scale.h>
#include <openspace/util/timerange.h>


namespace openspace {
namespace documentation {

void registerCoreClasses(documentation::DocumentationEngine& engine) {
    engine.addDocumentation(ConfigurationManager::Documentation());
    engine.addDocumentation(Ephemeris::Documentation());
    engine.addDocumentation(Mission::Documentation());
    engine.addDocumentation(Renderable::Documentation());
    engine.addDocumentation(Rotation::Documentation());
    engine.addDocumentation(Scale::Documentation());
    engine.addDocumentation(Scene::Documentation());
    engine.addDocumentation(SceneGraphNode::Documentation());
    engine.addDocumentation(ScreenSpaceRenderable::Documentation());
    engine.addDocumentation(TimeRange::Documentation());
}

} // namespace documentation
} // namespace openspace
