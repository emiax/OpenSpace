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

#include <openspace/scene/scene.h>

#include <openspace/openspace.h>
#include <openspace/engine/configurationmanager.h>
#include <openspace/engine/openspaceengine.h>
#include <openspace/engine/wrapper/windowwrapper.h>
#include <openspace/interaction/interactionhandler.h>
#include <openspace/query/query.h>
#include <openspace/rendering/renderengine.h>
#include <openspace/scene/scenegraphnode.h>
#include <openspace/scripting/scriptengine.h>
#include <openspace/scripting/script_helper.h>
#include <openspace/util/time.h>

#include <ghoul/filesystem/filesystem.h>
#include <ghoul/io/texture/texturereader.h>
#include <ghoul/misc/dictionary.h>
#include <ghoul/misc/exception.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/lua/ghoul_lua.h>
#include <ghoul/lua/lua_helper.h>
#include <ghoul/misc/onscopeexit.h>
#include <ghoul/opengl/programobject.h>
#include <ghoul/opengl/texture.h>

#include <chrono>
#include <iostream>
#include <iterator>
#include <numeric>
#include <fstream>
#include <string>
#include <stack>
#include <unordered_map>

#ifdef OPENSPACE_MODULE_ONSCREENGUI_ENABLED
#include <modules/onscreengui/include/gui.h>
#endif

#include "scene_doc.inl"
#include "scene_lua.inl"

namespace {
    const std::string _loggerCat = "Scene";
    const std::string _moduleExtension = ".mod";
    const std::string _commonModuleToken = "${COMMON_MODULE}";

    const std::string KeyCamera = "Camera";
    const std::string KeyFocusObject = "Focus";
    const std::string KeyPositionObject = "Position";
    const std::string KeyViewOffset = "Offset";

    const std::string MainTemplateFilename = "${OPENSPACE_DATA}/web/properties/main.hbs";
    const std::string PropertyOwnerTemplateFilename = "${OPENSPACE_DATA}/web/properties/propertyowner.hbs";
    const std::string PropertyTemplateFilename = "${OPENSPACE_DATA}/web/properties/property.hbs";
    const std::string HandlebarsFilename = "${OPENSPACE_DATA}/web/common/handlebars-v4.0.5.js";
    const std::string JsFilename = "${OPENSPACE_DATA}/web/properties/script.js";
    const std::string BootstrapFilename = "${OPENSPACE_DATA}/web/common/bootstrap.min.css";
    const std::string CssFilename = "${OPENSPACE_DATA}/web/common/style.css";
}

namespace openspace {

Scene::Scene() {}

Scene::~Scene() {}

void Scene::setRoot(std::unique_ptr<SceneGraphNode> root) {
    if (_root) {
        removeNode(_root.get());
    }
    _root = std::move(root);
    _root->setScene(this);
    addNode(_root.get());
}

void Scene::setCamera(std::unique_ptr<Camera> camera) {
    _camera = std::move(camera);
}

Camera* Scene::camera() const {
    return _camera.get();
}

void Scene::addNode(SceneGraphNode* node, Scene::UpdateDependencies updateDeps) {
    // Add the node and all its children.
    node->traversePreOrder([this](SceneGraphNode* n) {
        _topologicallySortedNodes.push_back(n);
        _nodesByName[n->name()] = n;
    });
    
    if (updateDeps) {
        updateDependencies();
    }
}

void Scene::removeNode(SceneGraphNode* node, Scene::UpdateDependencies updateDeps) {
    // Remove the node and all its children.
    node->traversePostOrder([this](SceneGraphNode* n) {
        std::remove(_topologicallySortedNodes.begin(), _topologicallySortedNodes.end(), n);
        _nodesByName.erase(n->name());
    });
    
    if (updateDeps) {
        updateDependencies();
    }
}

void Scene::updateDependencies() {
    sortTopologically();
}

void Scene::sortTopologically() {
    std::copy(_circularNodes.begin(), _circularNodes.end(), std::back_inserter(_topologicallySortedNodes));
    _circularNodes.clear();

    ghoul_assert(_topologicallySortedNodes.size() == _nodesByName.size(), "Number of scene graph nodes is inconsistent");
    
    if (_topologicallySortedNodes.empty())
        return;

    // Only the Root node can have an in-degree of 0
    SceneGraphNode* root = _nodesByName[SceneGraphNode::RootNodeName];
    if (!root) {
        throw Scene::InvalidSceneError("No root node found");
    }



    std::unordered_map<SceneGraphNode*, size_t> inDegrees;
    for (SceneGraphNode* node : _topologicallySortedNodes) {
        size_t inDegree = node->dependencies().size();
        if (node->parent() != nullptr) {
            inDegree++;
            inDegrees[node] = inDegree;
        }
    }

    std::stack<SceneGraphNode*> zeroInDegreeNodes;
    zeroInDegreeNodes.push(root);
    
    std::vector<SceneGraphNode*> nodes;
    nodes.reserve(_topologicallySortedNodes.size());
    while (!zeroInDegreeNodes.empty()) {
        SceneGraphNode* node = zeroInDegreeNodes.top();
        nodes.push_back(node);
        zeroInDegreeNodes.pop();

        for (auto& n : node->dependentNodes()) {
            auto it = inDegrees.find(n);
            it->second -= 1;
            if (it->second == 0) {
                zeroInDegreeNodes.push(n);
                inDegrees.erase(it);
            }
        }
        for (auto& n : node->children()) {
            auto it = inDegrees.find(n);
            it->second -= 1;
            if (it->second == 0) {
                zeroInDegreeNodes.push(n);
                inDegrees.erase(it);
            }
        }
    }
    if (inDegrees.size() > 0) {
        LERROR("The scene contains circular dependencies. " << inDegrees.size() << " nodes will be disabled.");
    }

    for (auto& it : inDegrees) {
        _circularNodes.push_back(it.first);
    }
    
    _topologicallySortedNodes = nodes;
}

void Scene::initialize() {
    for (SceneGraphNode* node : _topologicallySortedNodes) {
        try {
            bool success = node->initialize();
            if (success)
                LDEBUG(node->name() << " initialized successfully!");
            else
                LWARNING(node->name() << " not initialized.");
        }
        catch (const ghoul::RuntimeError& e) {
            LERRORC(_loggerCat + "(" + e.component + ")", e.what());
        }
    }
}

void Scene::update(const UpdateData& data) {
    for (auto& node : _topologicallySortedNodes) {
        try {
            node->update(data);
        }
        catch (const ghoul::RuntimeError& e) {
            LERRORC(e.component, e.what());
        }
    }
}

void Scene::evaluate(Camera* camera) {
    for (auto& node : _topologicallySortedNodes) {
        try {
            node->evaluate(camera);
        }
        catch (const ghoul::RuntimeError& e) {
            LERRORC(e.component, e.what());
        }
    }
}

void Scene::render(const RenderData& data, RendererTasks& tasks) {
    for (auto& node : _topologicallySortedNodes) {
        try {
            node->render(data, tasks);
        }
        catch (const ghoul::RuntimeError& e) {
            LERRORC(e.component, e.what());
        }
    }
}

void Scene::clear() {
    LINFO("Clearing current scene graph");
    _root = nullptr;
}

const std::map<std::string, SceneGraphNode*>& Scene::nodesByName() const {
    return _nodesByName;
}

/*bool Scene::loadSceneInternal(const std::string& sceneDescriptionFilePath) {
    ghoul::Dictionary dictionary;
    
    OsEng.windowWrapper().setSynchronization(false);
    OnExit(
        [](){ OsEng.windowWrapper().setSynchronization(true); }
    );
    
    lua_State* state = ghoul::lua::createNewLuaState();
    OnExit(
           // Delete the Lua state at the end of the scope, no matter what
           [state](){ ghoul::lua::destroyLuaState(state); }
           );
    
    OsEng.scriptEngine().initializeLuaState(state);

    ghoul::lua::loadDictionaryFromFile(
        sceneDescriptionFilePath,
        dictionary,
        state
    );

    // Perform testing against the documentation/specification
    openspace::documentation::testSpecificationAndThrow(
        Scene::Documentation(),
        dictionary,
        "Scene"
    );


    _graph.loadFromFile(sceneDescriptionFilePath);

    // Initialize all nodes
    for (SceneGraphNode* node : _graph.nodes()) {
        try {
            bool success = node->initialize();
            if (success)
                LDEBUG(node->name() << " initialized successfully!");
            else
                LWARNING(node->name() << " not initialized.");
        }
        catch (const ghoul::RuntimeError& e) {
            LERRORC(_loggerCat + "(" + e.component + ")", e.what());
        }
    }

    // update the position of all nodes
    // TODO need to check this; unnecessary? (ab)
    for (SceneGraphNode* node : _graph.nodes()) {
        try {
            node->update({
                glm::dvec3(0),
                glm::dmat3(1),
                1,
                Time::ref().j2000Seconds() });
        }
        catch (const ghoul::RuntimeError& e) {
            LERRORC(e.component, e.message);
        }
    }

    for (auto it = _graph.nodes().rbegin(); it != _graph.nodes().rend(); ++it)
        (*it)->calculateBoundingSphere();


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

    OsEng.enableBarrier();

    return true;
}*/

SceneGraphNode* Scene::root() const {
    return _root.get();
}
    
SceneGraphNode* Scene::sceneGraphNode(const std::string& name) const {
    auto it = _nodesByName.find(name);
    if (it != _nodesByName.end()) {
        return it->second;
    }
    return nullptr;
}

const std::vector<SceneGraphNode*>& Scene::allSceneGraphNodes() const {
    return _topologicallySortedNodes;
}

void Scene::writePropertyDocumentation(const std::string& filename, const std::string& type, const std::string& sceneFilename) {
    LDEBUG("Writing documentation for properties");
    if (type == "text") {
        std::ofstream file;
        file.exceptions(~std::ofstream::goodbit);
        file.open(filename);

        using properties::Property;
        for (SceneGraphNode* node : allSceneGraphNodes()) {
            std::vector<Property*> properties = node->propertiesRecursive();
            if (!properties.empty()) {
                file << node->name() << std::endl;

                for (Property* p : properties) {
                    file << p->fullyQualifiedIdentifier() << ":   " <<
                        p->guiName() << std::endl;
                }

                file << std::endl;
            }
        }
    }
    else if (type == "html") {
        std::ofstream file;
        file.exceptions(~std::ofstream::goodbit);
        file.open(filename);


        std::ifstream handlebarsInput(absPath(HandlebarsFilename));
        std::ifstream jsInput(absPath(JsFilename));

        std::string jsContent;
        std::back_insert_iterator<std::string> jsInserter(jsContent);

        std::copy(std::istreambuf_iterator<char>{handlebarsInput}, std::istreambuf_iterator<char>(), jsInserter);
        std::copy(std::istreambuf_iterator<char>{jsInput}, std::istreambuf_iterator<char>(), jsInserter);

        std::ifstream bootstrapInput(absPath(BootstrapFilename));
        std::ifstream cssInput(absPath(CssFilename));

        std::string cssContent;
        std::back_insert_iterator<std::string> cssInserter(cssContent);

        std::copy(std::istreambuf_iterator<char>{bootstrapInput}, std::istreambuf_iterator<char>(), cssInserter);
        std::copy(std::istreambuf_iterator<char>{cssInput}, std::istreambuf_iterator<char>(), cssInserter);

        std::ifstream mainTemplateInput(absPath(MainTemplateFilename));
        std::string mainTemplateContent{ std::istreambuf_iterator<char>{mainTemplateInput},
            std::istreambuf_iterator<char>{} };

        std::ifstream propertyOwnerTemplateInput(absPath(PropertyOwnerTemplateFilename));
        std::string propertyOwnerTemplateContent{ std::istreambuf_iterator<char>{propertyOwnerTemplateInput},
            std::istreambuf_iterator<char>{} };

        std::ifstream propertyTemplateInput(absPath(PropertyTemplateFilename));
        std::string propertyTemplateContent{ std::istreambuf_iterator<char>{propertyTemplateInput},
            std::istreambuf_iterator<char>{} };

        // Create JSON
        std::function<std::string(properties::PropertyOwner*)> createJson =
            [&createJson](properties::PropertyOwner* owner) -> std::string 
        {
            std::stringstream json;
            json << "{";
            json << "\"name\": \"" << owner->name() << "\",";

            json << "\"properties\": [";
            auto properties = owner->properties();
            for (properties::Property* p : properties) {
                json << "{";
                json << "\"id\": \"" << p->identifier() << "\",";
                json << "\"type\": \"" << p->className() << "\",";
                json << "\"fullyQualifiedId\": \"" << p->fullyQualifiedIdentifier() << "\",";
                json << "\"guiName\": \"" << p->guiName() << "\"";
                json << "}";
                if (p != properties.back()) {
                    json << ",";
                }
            }
            json << "],";

            json << "\"propertyOwners\": [";
            auto propertyOwners = owner->propertySubOwners();
            for (properties::PropertyOwner* o : propertyOwners) {
                json << createJson(o);
                if (o != propertyOwners.back()) {
                    json << ",";
                }
            }
            json << "]";
            json << "}";

            return json.str();
        };


        std::stringstream json;
        json << "[";
        std::vector<SceneGraphNode*> nodes = allSceneGraphNodes();
        if (!nodes.empty()) {
            json << std::accumulate(
                std::next(nodes.begin()),
                nodes.end(),
                createJson(*nodes.begin()),
                [createJson](std::string a, SceneGraphNode* n) {
                    return a + "," + createJson(n);
                }
            );
        }

        json << "]";

        std::string jsonString = "";
        for (const char& c : json.str()) {
            if (c == '\'') {
                jsonString += "\\'";
            } else {
                jsonString += c;
            }
        }

        std::stringstream html;
        html << "<!DOCTYPE html>\n"
            << "<html>\n"
            << "\t<head>\n"
            << "\t\t<script id=\"mainTemplate\" type=\"text/x-handlebars-template\">\n"
            << mainTemplateContent << "\n"
            << "\t\t</script>\n"
            << "\t\t<script id=\"propertyOwnerTemplate\" type=\"text/x-handlebars-template\">\n"
            << propertyOwnerTemplateContent << "\n"
            << "\t\t</script>\n"
            << "\t\t<script id=\"propertyTemplate\" type=\"text/x-handlebars-template\">\n"
            << propertyTemplateContent << "\n"
            << "\t\t</script>\n"
            << "\t<script>\n"
            << "var propertyOwners = JSON.parse('" << jsonString << "');\n"
            << "var version = [" << OPENSPACE_VERSION_MAJOR << ", " << OPENSPACE_VERSION_MINOR << ", " << OPENSPACE_VERSION_PATCH << "];\n"
            << "var sceneFilename = '" << sceneFilename << "';\n"
            << "var generationTime = '" << Time::now().ISO8601() << "';\n"
            << jsContent << "\n"
            << "\t</script>\n"
            << "\t<style type=\"text/css\">\n"
            << cssContent << "\n"
            << "\t</style>\n"
            << "\t\t<title>Documentation</title>\n"
            << "\t</head>\n"
            << "\t<body>\n"
            << "\t<body>\n"
            << "</html>\n";
        file << html.str();
    }
    else
        LERROR("Undefined type '" << type << "' for Property documentation");
}

scripting::LuaLibrary Scene::luaLibrary() {
    return {
        "",
        {
            {
                "setPropertyValue",
                &luascriptfunctions::property_setValue,
                "string, *",
                "Sets all properties identified by the URI (with potential wildcards) in "
                "the first argument. The second argument can be any type, but it has to "
                "match the type that the property (or properties) expect."
            },
            {
                "setPropertyValueRegex",
                &luascriptfunctions::property_setValueRegex,
                "string, *",
                "Sets all properties that pass the regular expression in the first "
                "argument. The second argument can be any type, but it has to match the "
                "type of the properties that matched the regular expression. The regular "
                "expression has to be of the ECMAScript grammar."
            },
            {
                "setPropertyValueSingle",
                &luascriptfunctions::property_setValueSingle,
                "string, *",
                "Sets a property identified by the URI in "
                "the first argument. The second argument can be any type, but it has to "
                "match the type that the property expects.",
            },
            {
                "getPropertyValue",
                &luascriptfunctions::property_getValue,
                "string",
                "Returns the value the property, identified by "
                "the provided URI."
            },
            {
                "loadScene",
                &luascriptfunctions::loadScene,
                "string",
                "Loads the scene found at the file passed as an "
                "argument. If a scene is already loaded, it is unloaded first"
            },
            /*{
                "addSceneGraphNode",
                &luascriptfunctions::addSceneGraphNode,
                "table",
                "Loads the SceneGraphNode described in the table and adds it to the "
                "SceneGraph"
            },
            {
                "removeSceneGraphNode",
                &luascriptfunctions::removeSceneGraphNode,
                "string",
                "Removes the SceneGraphNode identified by name"
            }*/
        }
    };
}

Scene::InvalidSceneError::InvalidSceneError(const std::string& error, const std::string& comp)
    : ghoul::RuntimeError(error, comp)
{}

}  // namespace openspace
