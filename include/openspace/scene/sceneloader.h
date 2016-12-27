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

#ifndef __OPENSPACE_CORE___SCENELOADER___H__
#define __OPENSPACE_CORE___SCENELOADER___H__

#include <memory>
#include <string>

#include <ghoul/misc/dictionary.h>
#include <ghoul/lua/ghoul_lua.h>

#include <openspace/scene/scenegraphnode.h>

namespace openspace {

class Scene;

class SceneLoader {
public:
    struct LoadedNode {
        LoadedNode(
            const std::string& nodeName,
            const std::string& parentName,
            const std::vector<std::string>& deps,
            std::unique_ptr<SceneGraphNode> n)
        {
            name = nodeName;
            parent = parentName;
            dependencies = deps;
            node = std::move(n);
        }

        std::string name;
        std::string parent;
        std::vector<std::string> dependencies;
        std::unique_ptr<SceneGraphNode> node;
    };

    SceneLoader() = default;
    ~SceneLoader() = default;
    
    // storage -> scene
    std::unique_ptr<Scene> loadScene(const std::string& path);
    
    // dictionary -> loaded
    std::unique_ptr<SceneLoader::LoadedNode> loadNode(const ghoul::Dictionary& dictionary, lua_State* luaState);
    
    // storage -> loaded
    std::vector<std::unique_ptr<SceneLoader::LoadedNode>> loadModule(const std::string& path, lua_State* luaState);
    std::vector<std::unique_ptr<SceneLoader::LoadedNode>> loadDirectory(const std::string& path, lua_State* luaState);

    // loaded -> scene
    std::unique_ptr<Scene> createSceneFromLoadedNodes(const std::vector<std::unique_ptr<SceneLoader::LoadedNode>>& nodes);
    void addLoadedNodes(Scene& scene, const std::vector<std::unique_ptr<SceneLoader::LoadedNode>>& nodes);

    //static openspace::Documentation Documentation();
private:
    ghoul::Dictionary loadSceneDictionary(const std::string& path, lua_State* state);
    std::string absoluteScenePath(const std::string& path);
};

}

#endif  // __OPENSPACE_CORE___SCENELOADER___H__
