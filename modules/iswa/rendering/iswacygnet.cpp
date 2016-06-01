/*****************************************************************************************
*                                                                                       *
* OpenSpace                                                                             *
*                                                                                       *
* Copyright (c) 2014-2015                                                               *
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
#include <modules/iswa/rendering/iswacygnet.h>
#include <openspace/engine/openspaceengine.h>
#include <openspace/rendering/renderengine.h>
#include <openspace/scene/scene.h>
#include <openspace/scene/scenegraphnode.h>
#include <openspace/util/time.h>
#include <openspace/util/transformationmanager.h>

namespace {
    const std::string _loggerCat = "IswaCygnet";
}

namespace openspace{

IswaCygnet::IswaCygnet(const ghoul::Dictionary& dictionary)
    : Renderable(dictionary)
    , _delete("delete", "Delete")
    ,_alpha("alpha", "Alpha", 0.9f, 0.0f, 1.0f)
    , _shader(nullptr)
    ,_type(IswaManager::CygnetType::NoType)
    ,_groupEvent()
    ,_group(nullptr)
    ,_textureDirty(false)
{
    _data = std::make_shared<Metadata>();

    // dict.getValue can only set strings in _data directly
    float renderableId;
    float updateTime;
    glm::vec3 min, max;
    glm::vec4 spatialScale;

    dictionary.getValue("Id", renderableId);
    dictionary.getValue("UpdateTime", updateTime);
    dictionary.getValue("SpatialScale", spatialScale);
    dictionary.getValue("GridMin", min);
    dictionary.getValue("GridMax", max);
    dictionary.getValue("Frame",_data->frame);
    dictionary.getValue("CoordinateType", _data->coordinateType);
    
    _data->id = (int) renderableId;
    _data->updateTime = (int) updateTime;
    _data->spatialScale = spatialScale;
    _data->gridMin = min;
    _data->gridMax = max;


    glm::vec3 scale;
    glm::vec3 offset;

    scale = glm::vec3(
        (max.x - min.x),
        (max.y - min.y),
        (max.z - min.z)
    );

    offset = glm::vec3(
        (min.x + (std::abs(min.x)+std::abs(max.x))/2.0f),
        (min.y + (std::abs(min.y)+std::abs(max.y))/2.0f),
        (min.z + (std::abs(min.z)+std::abs(max.z))/2.0f)
    );

    _data->scale = scale;
    _data->offset = offset;

    // std::cout << std::to_string(min) << std::endl;
    // std::cout << std::to_string(max) << std::endl;
    // std::cout << std::to_string(_data->scale) << std::endl;
    // std::cout << std::to_string(_data->offset) << std::endl;

    addProperty(_alpha);
    addProperty(_delete);

    // if(dictionary.hasValue<float>("Group")){
    dictionary.getValue("Group", _data->groupName);
    // }
    // _data->groupId = groupId;
    // std::cout << _data->id << std::endl;
    // std::cout << _data->frame << std::endl;
    // std::cout << std::to_string(_data->offset) << std::endl;
    // std::cout << std::to_string(_data->scale) << std::endl;
    // std::cout << std::to_string(_data->max) << std::endl;
    // std::cout << std::to_string(_data->min) << std::endl;
    // std::cout << std::to_string(_data->spatialScale) << std::endl;
    // OsEng.gui()._iswa.registerProperty(&_enabled);
}

IswaCygnet::~IswaCygnet(){}
bool IswaCygnet::initialize(){
    _textures.push_back(nullptr);
    
    if(!_data->groupName.empty()){
        initializeGroup();
    }else{
        OsEng.gui()._iswa.registerProperty(&_alpha);
        OsEng.gui()._iswa.registerProperty(&_delete);

        _delete.onChange([this](){
            deinitialize();
            OsEng.scriptEngine().queueScript("openspace.removeSceneGraphNode('" + name() + "')");
        });
    }
    
    initializeTime();
    createGeometry();
    createShader();
    updateTexture();

	return true;
}

bool IswaCygnet::deinitialize(){
     if(!_data->groupName.empty())
        _groupEvent->unsubscribe(name());
    //     IswaManager::ref().unregisterFromGroup(_data->groupName, this);


    unregisterProperties();
    destroyGeometry();
    destroyShader();

    return true;
}

bool IswaCygnet::isReady() const{
    bool ready = true;
    if (!_shader)
        ready &= false;
    return ready;
}

void IswaCygnet::render(const RenderData& data){
    if(!readyToRender()) return;
    
    psc position = data.position;
    glm::mat4 transform = glm::mat4(1.0);

    glm::mat4 rot = glm::mat4(1.0);
    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){
            transform[i][j] = static_cast<float>(_stateMatrix[i][j]);
        }
    }

    position += transform*glm::vec4(_data->spatialScale.x*_data->offset, _data->spatialScale.w);
    
    // Activate shader
    _shader->activate();
    glEnable(GL_ALPHA_TEST);
    glDisable(GL_CULL_FACE);


    _shader->setUniform("ViewProjection", data.camera.viewProjectionMatrix());
    _shader->setUniform("ModelTransform", transform);

    setPscUniforms(*_shader.get(), data.camera, position);

    setUniformAndTextures();
    renderGeometry();

    glEnable(GL_CULL_FACE);
    _shader->deactivate();
}

void IswaCygnet::update(const UpdateData& data){
    _openSpaceTime = Time::ref().currentTime();
    _realTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    _stateMatrix = TransformationManager::ref().frameTransformationMatrix(_data->frame, "GALACTIC", _openSpaceTime);

    bool timeToUpdate = (fabs(_openSpaceTime-_lastUpdateOpenSpaceTime) >= _data->updateTime &&
                        (_realTime.count()-_lastUpdateRealTime.count()) > _minRealTimeUpdateInterval);

    if( _data->updateTime != 0 && (Time::ref().timeJumped() || timeToUpdate )){
        updateTexture();

        _lastUpdateRealTime = _realTime;
        _lastUpdateOpenSpaceTime = _openSpaceTime;
    }

    if(_futureObject.valid() && DownloadManager::futureReady(_futureObject)) {
        _textureDirty = true;
    }
    
    if(_textureDirty) {
        loadTexture();
        _textureDirty = false;
    }

    if(!_transferFunctions.empty())
        for(auto tf : _transferFunctions)
            tf->update();
}


bool IswaCygnet::destroyShader(){
    RenderEngine& renderEngine = OsEng.renderEngine();
    if (_shader) {
        renderEngine.removeRenderProgram(_shader);
        _shader = nullptr;
    }
	return true;
}

void IswaCygnet::registerProperties(){
    OsEng.gui()._iswa.registerProperty(&_enabled);
    // OsEng.gui()._iswa.registerProperty(&_delete);
}

void IswaCygnet::unregisterProperties(){
    OsEng.gui()._iswa.unregisterProperties(name());
}

void IswaCygnet::initializeTime(){
    _openSpaceTime = Time::ref().currentTime();
    _lastUpdateOpenSpaceTime = _openSpaceTime;

    _realTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    _lastUpdateRealTime = _realTime;

    _minRealTimeUpdateInterval = 100;
}

bool IswaCygnet::createShader(){
    if (_shader == nullptr) {
        // Plane Program
        RenderEngine& renderEngine = OsEng.renderEngine();
        _shader = renderEngine.buildRenderProgram(_programName,
            _vsPath,
            _fsPath
            );
        if (!_shader) return false;
    }
    return true;
}

void IswaCygnet::initializeGroup(){
    _groupEvent = IswaManager::ref().groupEvent(_data->groupName, _type);
    _group = IswaManager::ref().registerToGroup(_data->groupName, _type);

    //Subscribe to enable propert and delete
    _groupEvent->subscribe(name(), "enabledChanged", [&](const ghoul::Dictionary& dict){
        LDEBUG(name() + " Event enabledChanged");
        _enabled.setValue(dict.value<bool>("enabled"));
    });

    _groupEvent->subscribe(name(), "alphaChanged", [&](const ghoul::Dictionary& dict){
        LDEBUG(name() + " Event alphaChanged");
        _alpha.setValue(dict.value<float>("alpha"));
    });

    _groupEvent->subscribe(name(), "clearGroup", [&](ghoul::Dictionary dict){
        LDEBUG(name() + " Event clearGroup");
        OsEng.scriptEngine().queueScript("openspace.removeSceneGraphNode('" + name() + "')");
    });
}

}//namespace openspac