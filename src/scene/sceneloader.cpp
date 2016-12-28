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



#include <openspace/engine/openspaceengine.h>
#include <openspace/scene/sceneloader.h>
#include <openspace/scene/scene.h>
#include <openspace/scene/scenegraphnode.h>
#include <openspace/documentation/verifier.h>

#include <ghoul/filesystem/filesystem.h>
#include <ghoul/filesystem/file.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/onscopeexit.h>


#include <memory>

namespace {
    const std::string _loggerCat = "SceneLoader";
    const std::string KeyPathScene = "ScenePath";
    const std::string KeyModules = "Modules";
    const std::string ModuleExtension = ".mod";
    const std::string KeyPathModule = "ModulePath";

    const std::string RootNodeName = "Root";
    const std::string KeyName = "Name";
    const std::string KeyParentName = "Parent";
    const std::string KeyDependencies = "Dependencies";
    const std::string KeyCamera = "Camera";
    const std::string KeyCameraFocus = "Focus";
    const std::string KeyCameraPosition = "Position";
    const std::string KeyCameraRotation = "Rotation";

}


struct ModuleInformation {
    ghoul::Dictionary dictionary;
    std::string moduleFile;
    std::string modulePath;
    std::string moduleName;
};


namespace openspace {

/*
Documentation SceneLoader::Documentation() {
    using namespace openspace::documentation;

    return {
        "Scene",
        "scene",
        {
        {
            KeyPathScene,
            new StringAnnotationVerifier("A relative or absolute path"),
            "The path to the directory where referenced modules are located",
            Optional::Yes
        },
        {
            KeyCamera,
            new TableVerifier({
                {
                    KeyCameraFocus,
                    new StringAnnotationVerifier("A name of a scene graph node"),
                    "The scene graph node to which the camera position is relatively described",
                    Optional::No
                },
                {
                    KeyCameraPosition,
                    new Vector3Verifier<double>(),
                    "The camera position",
                    Optional::No
                }
            }),
            "Information about the camera",
            Optional::No
        },
        {
            KeyModules,
            new StringListVerifier("Module directories"),
            "List of modules to include",
            Optional::No
        }
        }
    };

}*/


std::unique_ptr<Scene> SceneLoader::loadScene(const std::string& path) {
    // Set up lua state.
    lua_State* state = ghoul::lua::createNewLuaState();
    OnExit(
        // Delete the Lua state at the end of the scope, no matter what
        [state]() {ghoul::lua::destroyLuaState(state); }
    );
    OsEng.scriptEngine().initializeLuaState(state);

    std::string absScenePath = absoluteScenePath(path);
    ghoul::filesystem::File sceneFile(absScenePath);
    std::string sceneDirectory = sceneFile.directoryName();

    ghoul::Dictionary sceneDictionary = loadSceneDictionary(absScenePath, state);
    documentation::testSpecificationAndThrow(Scene::Documentation(), sceneDictionary, "Scene");

    std::string relativeSceneDirectory = ".";
    sceneDictionary.getValue<std::string>(KeyPathScene, relativeSceneDirectory);
    std::string modulesPath = FileSys.absPath(sceneDirectory + FileSys.PathSeparator + relativeSceneDirectory);

    ghoul::Dictionary moduleDictionary;
    sceneDictionary.getValue(KeyModules, moduleDictionary);

    // Above we generated a ghoul::Dictionary from the scene file; now we run the scene
    // file again to load any variables defined inside into the state that is passed to
    // the modules. This allows us to specify global variables that can then be used
    // inside the modules to toggle settings
    ghoul::lua::runScriptFile(state, absScenePath);
    std::vector<std::string> keys = moduleDictionary.keys();
    ghoul::filesystem::Directory oldDirectory = FileSys.currentDirectory();

    std::vector<std::unique_ptr<SceneLoader::LoadedNode>> allNodes;

    for (const std::string& key : keys) {
        std::string fullModuleName = moduleDictionary.value<std::string>(key);
        std::replace(fullModuleName.begin(), fullModuleName.end(), '/', FileSys.PathSeparator);
        std::string modulePath = FileSys.pathByAppendingComponent(modulesPath, fullModuleName);

        std::vector<std::unique_ptr<SceneLoader::LoadedNode>> nodes = loadDirectory(modulePath, state);
        std::move(nodes.begin(), nodes.end(), std::back_inserter(allNodes));
    }

    FileSys.setCurrentDirectory(oldDirectory);
    
    std::unique_ptr<Scene> scene = std::make_unique<Scene>();

    std::unique_ptr<SceneGraphNode> rootNode = std::make_unique<SceneGraphNode>();
    rootNode->setName(SceneGraphNode::RootNodeName);
    scene->setRoot(std::move(rootNode));

    addLoadedNodes(*scene, allNodes);

    ghoul::Dictionary cameraDictionary;
    sceneDictionary.getValue(KeyCamera, cameraDictionary);
    std::unique_ptr<LoadedCamera> loadedCamera = loadCamera(cameraDictionary);

    auto& nodeMap = scene->nodesByName();
    auto it = nodeMap.find(loadedCamera->parent);
    if (it != nodeMap.end()) {
        loadedCamera->camera->setParent(it->second);
    }

    scene->setCamera(std::move(loadedCamera->camera));

    return std::move(scene);

    // Reset the InteractionManager to Orbital/default mode
    // TODO: Decide if it belongs in the scene and/or how it gets reloaded
    //OsEng.interactionHandler().setInteractionMode("Orbital");

    // After loading the scene, the keyboard bindings have been set
    /*
    const std::string KeyboardShortcutsType =
        ConfigurationManager::KeyKeyboardShortcuts + "." +
        ConfigurationManager::PartType;

    const std::string KeyboardShortcutsFile =
        ConfigurationManager::KeyKeyboardShortcuts + "." +
        ConfigurationManager::PartFile;


            std::string type;
            std::string file;
            bool hasType = OsEng.configurationManager().getValue(
                KeyboardShortcutsType, type
                );

            bool hasFile = OsEng.configurationManager().getValue(
                KeyboardShortcutsFile, file
                );

            if (hasType && hasFile) {
                OsEng.interactionHandler().writeKeyboardDocumentation(type, file);
            }

            LINFO("Loaded " << _sceneGraphToLoad);
            _sceneGraphToLoad = "";
        }
        catch (const ghoul::RuntimeError& e) {
            LERROR(e.what());
            _sceneGraphToLoad = "";
            return;
        }
    }
    */

    /*
    
    _camera = std::make_unique<Camera>();
    OsEng.renderEngine().setCamera(_camera.get());
    OsEng.interactionHandler().setCamera(_camera.get());

    // Read the camera dictionary and set the camera state
    ghoul::Dictionary cameraDictionary;
    if (dictionary.getValue(KeyCamera, cameraDictionary)) {
        OsEng.interactionHandler().setCameraStateFromDictionary(cameraDictionary);
    }

    // If a PropertyDocumentationFile was specified, generate it now
    const std::string KeyPropertyDocumentationType =
        ConfigurationManager::KeyPropertyDocumentation + '.' +
        ConfigurationManager::PartType;

    const std::string KeyPropertyDocumentationFile =
        ConfigurationManager::KeyPropertyDocumentation + '.' +
        ConfigurationManager::PartFile;

    const bool hasType = OsEng.configurationManager().hasKey(KeyPropertyDocumentationType);
    const bool hasFile = OsEng.configurationManager().hasKey(KeyPropertyDocumentationFile);
    if (hasType && hasFile) {
        std::string propertyDocumentationType;
        OsEng.configurationManager().getValue(KeyPropertyDocumentationType, propertyDocumentationType);
        std::string propertyDocumentationFile;
        OsEng.configurationManager().getValue(KeyPropertyDocumentationFile, propertyDocumentationFile);

        propertyDocumentationFile = absPath(propertyDocumentationFile);
        writePropertyDocumentation(propertyDocumentationFile, propertyDocumentationType, sceneDescriptionFilePath);
    }


    OsEng.runPostInitializationScripts(sceneDescriptionFilePath);

    */
}

std::unique_ptr<SceneLoader::LoadedCamera> SceneLoader::loadCamera(const ghoul::Dictionary& cameraDict) {
    std::string focus;
    glm::vec3 cameraPosition;
    glm::vec4 cameraRotation;


    bool readSuccessful = true;
    readSuccessful &= cameraDict.getValue(KeyCameraFocus, focus);
    readSuccessful &= cameraDict.getValue(KeyCameraPosition, cameraPosition);
    readSuccessful &= cameraDict.getValue(KeyCameraRotation, cameraRotation);

    std::unique_ptr<Camera> camera = std::make_unique<Camera>();

    camera->setPositionVec3(cameraPosition);
    camera->setRotation(glm::dquat(
        cameraRotation.x, cameraRotation.y, cameraRotation.z, cameraRotation.w));

    std::unique_ptr<LoadedCamera> loadedCamera = std::make_unique<LoadedCamera>(focus, std::move(camera));
    
    if (!readSuccessful) {
        throw Scene::InvalidSceneError(
            "Position, Rotation and Focus need to be defined for camera dictionary.");
    }
    
    //setFocusNode(node);
    return std::move(loadedCamera);
}


std::vector<std::unique_ptr<SceneLoader::LoadedNode>> SceneLoader::loadDirectory(
    const std::string& path,
    lua_State* luaState)
{
    std::string::size_type pos = path.find_last_of(FileSys.PathSeparator);
    if (pos == std::string::npos) {
        LERROR("Error parsing directory name '" << path << "'");
        return std::vector<std::unique_ptr<SceneLoader::LoadedNode>>();
    }
    std::string moduleName = path.substr(pos + 1);
    std::string moduleFile = FileSys.pathByAppendingComponent(path, moduleName) + ModuleExtension;

    if (FileSys.fileExists(moduleFile)) {
        // Todo: get rid of changing the working directory (global state is bad)
        FileSys.setCurrentDirectory(ghoul::filesystem::Directory(path));
        
        // We have a module file, so it is a direct include.
        return loadModule(moduleFile, luaState);
    } else {
        std::vector<std::unique_ptr<SceneLoader::LoadedNode>> allLoadedNodes;
        // If we do not have a module file, we have to include all subdirectories
        using ghoul::filesystem::Directory;
        using std::string;
        std::vector<string> directories = Directory(path).readDirectories();

        for (const string& s : directories) {
            std::string submodulePath = FileSys.pathByAppendingComponent(path, s);
            std::vector<std::unique_ptr<SceneLoader::LoadedNode>> loadedNodes = loadDirectory(submodulePath, luaState);
            std::move(loadedNodes.begin(), loadedNodes.end(), std::back_inserter(allLoadedNodes));
        }
        return std::move(allLoadedNodes);
    }
}


std::unique_ptr<SceneLoader::LoadedNode> SceneLoader::loadNode(const ghoul::Dictionary& dictionary) {
    std::vector<std::string> dependencies;

    std::string nodeName = dictionary.value<std::string>(KeyName);
    std::string parentName = dictionary.value<std::string>(KeyParentName);
    std::unique_ptr<SceneGraphNode> node = SceneGraphNode::createFromDictionary(dictionary);

    if (dictionary.hasKey(SceneGraphNode::KeyDependencies)) {
        if (!dictionary.hasValue<ghoul::Dictionary>(SceneGraphNode::KeyDependencies)) {
            LERROR("Dependencies did not have the corrent type");
        }
        ghoul::Dictionary nodeDependencies;
        dictionary.getValue(SceneGraphNode::KeyDependencies, nodeDependencies);

        std::vector<std::string> keys = nodeDependencies.keys();
        for (const std::string& key : keys) {
            std::string value = nodeDependencies.value<std::string>(key);
            dependencies.push_back(value);
        }
    }
    return std::make_unique<SceneLoader::LoadedNode>(nodeName, parentName, dependencies, std::move(node));
}


std::vector<std::unique_ptr<SceneLoader::LoadedNode>> SceneLoader::loadModule(const std::string& path, lua_State* luaState) {
    ghoul::Dictionary moduleDictionary;
    try {
        ghoul::lua::loadDictionaryFromFile(path, moduleDictionary, luaState);
    } catch (ghoul::lua::LuaRuntimeException e) {
        LERRORC(e.component, e.message);
        return std::vector<std::unique_ptr<SceneLoader::LoadedNode>>();
    }

    std::vector<std::unique_ptr<SceneLoader::LoadedNode>> loadedNodes;
    std::vector<std::string> keys = moduleDictionary.keys();
    for (const std::string& key : keys) {
        ghoul::Dictionary nodeDictionary;
        if (!moduleDictionary.getValue(key, nodeDictionary)) {
            LERROR("Node dictionary did not have the corrent type");
        }
        loadedNodes.push_back(loadNode(nodeDictionary));
    }
    return loadedNodes;
};


std::string SceneLoader::absoluteScenePath(const std::string& path) {
    std::string absScenePath = absPath(path);

    /*
    // See if scene file exists
    using RawPath = ghoul::filesystem::FileSystem::RawPath;
    if (!FileSys.fileExists(absScenePath, RawPath::Yes)) {
        LERROR("Could not load scene file '" << absScenePath << "'. " <<
            "File not found");
        return false;
    }
    LINFO("Loading scene from file '" << absScenePath << "'");

    // The scene path could either be an absolute or relative path to the description
    // paths directory
    std::string relativeCandidate = modulesPath +
        ghoul::filesystem::FileSystem::PathSeparator + sceneDirectory;
    std::string absoluteCandidate = abscPath(sceneDirectory);

    if (FileSys.directoryExists(relativeCandidate))
        sceneDirectory = relativeCandidate;
    else if (FileSys.directoryExists(absoluteCandidate))
        sceneDirectory = absoluteCandidate;
    else {
        LERROR("The '" << KeyPathScene << "' pointed to a "
            "path '" << sceneDirectory << "' that did not exist");
        return false;
    }*/

    return absScenePath;

}

ghoul::Dictionary SceneLoader::loadSceneDictionary(const std::string& path, lua_State* state) {
    // Load dictionary
    ghoul::Dictionary sceneDictionary;
    if (!FileSys.fileExists(path)) {
        throw ghoul::FileNotFoundError(path);
    }
    ghoul::lua::loadDictionaryFromFile(path, sceneDictionary, state);
    return sceneDictionary;
}


void SceneLoader::addLoadedNodes(Scene& scene, const std::vector<std::unique_ptr<SceneLoader::LoadedNode>>& loadedNodes) {
    std::map<std::string, SceneGraphNode*> existingNodes = scene.nodesByName();
    std::map<std::string, SceneGraphNode*> addedNodes;
    std::vector<SceneGraphNode*> attachedBranches;

    // Extend the map from name to nodes with all the new contents.
    for (auto& loadedNode : loadedNodes) {
        std::string name = loadedNode->name;
        if (existingNodes.count(name) > 0) {
            throw Scene::InvalidSceneError("Node with name '" + name + "' already exists in scene");
        }
        if (addedNodes.count(name) > 0) {
            throw Scene::InvalidSceneError("Duplicate node names '" + name + "' among loaded nodes");
        }

        SceneGraphNode* node = loadedNode->node.get();
        addedNodes[name] = node;

        if (existingNodes.count(loadedNode->parent) == 1) {
            attachedBranches.push_back(node);
        }
    }
    
    auto findNode = [&existingNodes, &addedNodes](const std::string name) {
        std::map<std::string, SceneGraphNode*>::iterator it;
        if ((it = existingNodes.find(name)) != existingNodes.end()) {
            return it->second;
        }
        if ((it = addedNodes.find(name)) != addedNodes.end()) {
            return it->second;
        }
        return static_cast<SceneGraphNode*>(nullptr);
    };

    // Attach nodes to each other and set up dependencies.
    for (auto& loadedNode : loadedNodes) {
        std::string parentName = loadedNode->parent;
        std::vector<std::string> dependencyNames = loadedNode->dependencies;

        SceneGraphNode* parent = findNode(parentName);

        if (!parent) {
            throw Scene::InvalidSceneError("Could not find parent '" + parentName + "' for '" + loadedNode->name + "'");
        }
        
        std::vector<SceneGraphNode*> dependencies;
        for (const auto& depName : dependencyNames) {
            SceneGraphNode* dep = findNode(depName);
            if (!dep) {
                throw Scene::InvalidSceneError("Could not find dependency '" + depName + "' for '" + loadedNode->name + "'");
            }
            dependencies.push_back(dep);
        }

        SceneGraphNode* child = loadedNode->node.get();

        parent->attachChild(std::move(loadedNode->node), SceneGraphNode::UpdateScene::No);
        child->setDependencies(dependencies, SceneGraphNode::UpdateScene::No);
    }

    
    // Add the nodes to the scene and update dependencies.
    for (auto& node : attachedBranches) {
        scene.addNode(node, Scene::UpdateDependencies::No);
    }

    for (auto& p : addedNodes) {
        SceneGraphNode* node = p.second;
        if (!node->scene()) {
            LWARNING("Node '" << node->name() << "' is not connected to the root and will not be added to the scene");
        }
    }

    try {
        scene.updateDependencies();
    } catch (Scene::InvalidSceneError e) {
        for (SceneGraphNode* attachedBranch : attachedBranches) {
            // Reset the scene to the previous state.
            scene.removeNode(attachedBranch, Scene::UpdateDependencies::No);
            attachedBranch->parent()->detachChild(*attachedBranch, SceneGraphNode::UpdateScene::No);
        }
        throw e;
    }
}




}