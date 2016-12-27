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

#include "gtest/gtest.h"

#include <openspace/scene/sceneloader.h>
#include <openspace/scene/scenegraphnode.h>
#include <openspace/documentation/documentation.h>
#include <ghoul/misc/dictionaryluaformatter.h>

#include <ghoul/lua/lua_helper.h>
#include <fstream>


//class SceneLoaderTest : public testing::Test {};

TEST(SceneLoaderTest, NonExistingFileTest) {
    const std::string file = absPath("NonExistingFile");

    openspace::SceneLoader loader;
    EXPECT_THROW(loader.loadScene(file), ghoul::FileNotFoundError);
}

TEST(SceneLoaderTest, IllformedFileTest) {
    const std::string file = absPath("${TESTDIR}/SceneLoaderTest/illformed.scene");

    openspace::SceneLoader loader;
    EXPECT_THROW(loader.loadScene(file), ghoul::lua::LuaRuntimeException);
}


TEST(SceneLoaderTest, IllformedFileTestInvalidSceneFolder) {
    const std::string file = absPath("${TESTDIR}/SceneLoaderTest/illformedInvalidScene.scene");

    openspace::SceneLoader loader;
    EXPECT_THROW(loader.loadScene(file), openspace::documentation::SpecificationError);
}

TEST(SceneLoaderTest, IllformedFileTestWrongType) {
    const std::string file = absPath("${TESTDIR}/SceneLoaderTest/illformedWrongType.scene");

    openspace::SceneLoader loader;
    EXPECT_THROW(loader.loadScene(file), openspace::documentation::SpecificationError);
}


TEST(SceneLoaderTest, AbsoluteScenePath) {
    const std::string scenePath = absPath("${TEMPORARY}/tmp.scene");
    std::ofstream sceneFile(scenePath.c_str());

    ghoul::DictionaryLuaFormatter formatter;
    ghoul::Dictionary sceneFileDictionary;

    sceneFileDictionary.setValue<std::string>("ScenePath", absPath("${TESTDIR}/SceneLoaderTest/scene-folder"));
    sceneFileDictionary.setValue<ghoul::Dictionary>("Modules", ghoul::Dictionary());

    sceneFile << "return " << formatter.format(sceneFileDictionary);
    sceneFile.close();

    openspace::SceneLoader loader;
    std::unique_ptr<openspace::Scene> scene = loader.loadScene(scenePath);
    ASSERT_NE(scene, nullptr) << "loadScene returned nullptr";
    std::vector<openspace::SceneGraphNode*> nodes = scene->allSceneGraphNodes();
    EXPECT_EQ(nodes.size(), 1) << "Expected scene to consist of one root node";
}

TEST(SceneLoaderTest, Test00) {
    const std::string file = absPath("${TESTDIR}/SceneLoaderTest/test00.scene");

    openspace::SceneLoader loader;
    std::unique_ptr<openspace::Scene> scene = loader.loadScene(file);

    ASSERT_NE(scene, nullptr) << "loadScene returned nullptr"; 
    std::vector<openspace::SceneGraphNode*> nodes = scene->allSceneGraphNodes();
    EXPECT_EQ(nodes.size(), 1) << "Expected scene to consist of one root node";
}


TEST(SceneLoaderTest, Test00Location) {
    const std::string file = absPath("${TESTDIR}/SceneLoaderTest/test00-location.scene");

    openspace::SceneLoader loader;
    std::unique_ptr<openspace::Scene> scene = loader.loadScene(file);

    ASSERT_NE(scene, nullptr) << "loadScene returned nullptr";
    std::vector<openspace::SceneGraphNode*> nodes = scene->allSceneGraphNodes();
    EXPECT_EQ(nodes.size(), 1) << "Expected scene to consist of one root node";
}


TEST(SceneGraphLoaderTest, Test01) {
    const std::string file = absPath("${TESTDIR}/SceneLoaderTest/test01.scene");

    openspace::SceneLoader loader;
    std::unique_ptr<openspace::Scene> scene = loader.loadScene(file);

    ASSERT_NE(scene, nullptr) << "loadScene returned nullptr";
    std::vector<openspace::SceneGraphNode*> nodes = scene->allSceneGraphNodes();
    EXPECT_EQ(nodes.size(), 2) << "Expected scene to consist of two nodes";

    std::map<std::string, openspace::SceneGraphNode*> nodesByName = scene->nodesByName();
    EXPECT_EQ(nodesByName.size(), 2) << "Expected scene to consist of two nodes";

    EXPECT_EQ(nodesByName["Root"]->name(), "Root");
    EXPECT_EQ(nodesByName["NoDependency"]->name(), "NoDependency");
    EXPECT_EQ(nodesByName["NoDependency"]->parent(), nodesByName["Root"]);
}

TEST(SceneGraphLoaderTest, Test01Location) {
    const std::string file = absPath("${TESTDIR}/SceneLoaderTest/test01-location.scene");

    openspace::SceneLoader loader;
    std::unique_ptr<openspace::Scene> scene = loader.loadScene(file);

    ASSERT_NE(scene, nullptr) << "loadScene returned nullptr";
    std::vector<openspace::SceneGraphNode*> nodes = scene->allSceneGraphNodes();
    EXPECT_EQ(nodes.size(), 2) << "Expected scene to consist of two nodes";
    
    std::map<std::string, openspace::SceneGraphNode*> nodesByName = scene->nodesByName();
    EXPECT_EQ(nodesByName.size(), 2) << "Expected scene to consist of two nodes";

    EXPECT_EQ(nodesByName["Root"]->name(), "Root");
    EXPECT_EQ(nodesByName["NoDependency"]->name(), "NoDependency");
    EXPECT_EQ(nodesByName["NoDependency"]->parent(), nodesByName["Root"]);
}
//
//TEST_F(SceneGraphLoaderTest, Test02) {
//    const std::string file = absPath("${TESTDIR}/SceneGraphLoaderTest/test02.scene");
//
//    std::vector<openspace::SceneGraphNode*> nodes;
//    bool success = openspace::SceneGraphLoader::load(file, nodes);
//
//    ASSERT_TRUE(success) << "Successful loading";
//    ASSERT_TRUE(nodes.size() == 2) << "Correct number of nodes";
//    bool found = false;
//    for (openspace::SceneGraphNode* n : nodes)
//        if (n->name() == "NoDependency")
//            found = true;
//        
//    EXPECT_TRUE(found) << "Correct node loaded";
//}
//
//TEST_F(SceneGraphLoaderTest, Test02Location) {
//    const std::string file = absPath("${TESTDIR}/SceneGraphLoaderTest/test02-location.scene");
//
//    std::vector<openspace::SceneGraphNode*> nodes;
//    bool success = openspace::SceneGraphLoader::load(file, nodes);
//
//    ASSERT_TRUE(success) << "Successful loading";
//    ASSERT_TRUE(nodes.size() == 2) << "Correct number of nodes";
//    bool found = false;
//    for (openspace::SceneGraphNode* n : nodes)
//        if (n->name() == "NoDependency")
//            found = true;
//
//    EXPECT_TRUE(found) << "Correct node loaded";
//}
//
//TEST_F(SceneGraphLoaderTest, Test03) {
//    const std::string file = absPath("${TESTDIR}/SceneGraphLoaderTest/test03.scene");
//
//    std::vector<openspace::SceneGraphNode*> nodes;
//    bool success = openspace::SceneGraphLoader::load(file, nodes);
//
//    ASSERT_TRUE(success) << "Successful loading";
//    ASSERT_TRUE(nodes.size() == 2) << "Correct number of nodes";
//    bool found = false;
//    for (openspace::SceneGraphNode* n : nodes)
//        if (n->name() == "CommonDependency")
//            found = true;
//
//    EXPECT_TRUE(found) << "Correct node loaded";
//}
//
//TEST_F(SceneGraphLoaderTest, Test03Location) {
//    const std::string file = absPath("${TESTDIR}/SceneGraphLoaderTest/test03-location.scene");
//
//    std::vector<openspace::SceneGraphNode*> nodes;
//    bool success = openspace::SceneGraphLoader::load(file, nodes);
//
//    ASSERT_TRUE(success) << "Successful loading";
//    ASSERT_TRUE(nodes.size() == 2) << "Correct number of nodes";
//    bool found = false;
//    for (openspace::SceneGraphNode* n : nodes)
//        if (n->name() == "CommonDependency")
//            found = true;
//
//    EXPECT_TRUE(found) << "Correct node loaded";
//}
//
//TEST_F(SceneGraphLoaderTest, Test04) {
//    const std::string file = absPath("${TESTDIR}/SceneGraphLoaderTest/test04.scene");
//
//    std::vector<openspace::SceneGraphNode*> nodes;
//    bool success = openspace::SceneGraphLoader::load(file, nodes);
//
//    ASSERT_TRUE(success) << "Successful loading";
//    ASSERT_TRUE(nodes.size() == 3) << "Correct number of nodes";
//    bool found = false;
//    for (openspace::SceneGraphNode* n : nodes)
//        if (n->name() == "DirectDependency")
//            found = true;
//
//    EXPECT_TRUE(found) << "Correct node loaded";
//}
//
//TEST_F(SceneGraphLoaderTest, Test04Location) {
//    const std::string file = absPath("${TESTDIR}/SceneGraphLoaderTest/test04-location.scene");
//
//    std::vector<openspace::SceneGraphNode*> nodes;
//    bool success = openspace::SceneGraphLoader::load(file, nodes);
//
//    ASSERT_TRUE(success) << "Successful loading";
//    ASSERT_TRUE(nodes.size() == 3) << "Correct number of nodes";
//    bool found = false;
//    for (openspace::SceneGraphNode* n : nodes)
//        if (n->name() == "DirectDependency")
//            found = true;
//
//    EXPECT_TRUE(found) << "Correct node loaded";
//}
//
//TEST_F(SceneGraphLoaderTest, Test05) {
//    const std::string file = absPath("${TESTDIR}/SceneGraphLoaderTest/test05.scene");
//
//    std::vector<openspace::SceneGraphNode*> nodes;
//    bool success = openspace::SceneGraphLoader::load(file, nodes);
//
//    ASSERT_TRUE(success) << "Successful loading";
//    ASSERT_TRUE(nodes.size() == 4) << "Correct number of nodes";
//    bool found = false;
//    for (openspace::SceneGraphNode* n : nodes)
//        if (n->name() == "MultipleDependencies")
//            found = true;
//
//    EXPECT_TRUE(found) << "Correct node loaded";
//}
//
//TEST_F(SceneGraphLoaderTest, Test05Location) {
//    const std::string file = absPath("${TESTDIR}/SceneGraphLoaderTest/test05-location.scene");
//
//    std::vector<openspace::SceneGraphNode*> nodes;
//    bool success = openspace::SceneGraphLoader::load(file, nodes);
//
//    ASSERT_TRUE(success) << "Successful loading";
//    ASSERT_TRUE(nodes.size() == 4) << "Correct number of nodes";
//    bool found = false;
//    for (openspace::SceneGraphNode* n : nodes)
//        if (n->name() == "MultipleDependencies")
//            found = true;
//
//    EXPECT_TRUE(found) << "Correct node loaded";
//}
//
//TEST_F(SceneGraphLoaderTest, Test06) {
//    const std::string file = absPath("${TESTDIR}/SceneGraphLoaderTest/test06.scene");
//
//    std::vector<openspace::SceneGraphNode*> nodes;
//    bool success = openspace::SceneGraphLoader::load(file, nodes);
//
//    ASSERT_FALSE(success) << "Successful loading";
//    ASSERT_TRUE(nodes.empty()) << "Correct number of nodes";
//}
//
//TEST_F(SceneGraphLoaderTest, Test06Location) {
//    const std::string file = absPath("${TESTDIR}/SceneGraphLoaderTest/test06-location.scene");
//
//    std::vector<openspace::SceneGraphNode*> nodes;
//    bool success = openspace::SceneGraphLoader::load(file, nodes);
//
//    ASSERT_TRUE(success) << "Successful loading";
//    ASSERT_TRUE(nodes.size() == 4) << "Correct number of nodes";
//    bool found = false;
//    for (openspace::SceneGraphNode* n : nodes)
//        if (n->name() == "MultipleDependencies")
//            found = true;
//
//    EXPECT_TRUE(found) << "No scenegraph nodes loaded";
//}
//
////
////
////
////TEST_F(SceneGraphTest, SceneGraphNode) {
////
////    openspace::SceneGraphNode *node =
////        openspace::SceneGraphNode::createFromDictionary(ghoul::Dictionary());
////
////    // Should not have a renderable and position should be 0,0,0,0 (undefined).
////    EXPECT_EQ(nullptr, node->renderable());
////    EXPECT_EQ(openspace::psc(), node->position());
////    
////    delete node;
////    ghoul::Dictionary nodeDictionary;
////    
////    ghoul::Dictionary positionDictionary;
////    ghoul::Dictionary positionPositionArrayDictionary;
////    
////    ghoul::Dictionary renderableDictionary;
////    
////    renderableDictionary.setValue("Type", std::string("RenderablePlanet"));
////    
////    positionPositionArrayDictionary.setValue("1", 1.0);
////    positionPositionArrayDictionary.setValue("2", 1.0);
////    positionPositionArrayDictionary.setValue("3", 1.0);
////    positionPositionArrayDictionary.setValue("4", 1.0);
////    
////    positionDictionary.setValue("Type", std::string("Static"));
////    positionDictionary.setValue("Position", positionPositionArrayDictionary);
////    
////    nodeDictionary.setValue("Position", positionDictionary);
////    nodeDictionary.setValue("Renderable", renderableDictionary);
////    
////    node =
////        openspace::SceneGraphNode::createFromDictionary(nodeDictionary);
////    
////    // This node should have a renderable (probably no good values but an existing one)
////    EXPECT_TRUE(node->renderable());
////    
////    // position should be initialized
////    EXPECT_EQ(openspace::psc(1.0,1.0,1.0,1.0), node->position());
////    
////    delete node;
////}
////
////TEST_F(SceneGraphTest, Loading) {
////    
////    
////    // Should not successfully load a non existing scenegraph
////    EXPECT_FALSE(_scenegraph->loadScene(absPath("${TESTDIR}/ScenegraphTestNonExisting"), absPath("${TESTDIR}")));
////    
////    // Existing scenegraph should load
////    EXPECT_TRUE(_scenegraph->loadScene(absPath("${TESTDIR}/ScenegraphTest"), absPath("${TESTDIR}")));
////    // TODO need to check for correctness
////    
////    // This loading should fail regardless of existing or not since the
////    // scenegraph is already loaded
////    EXPECT_FALSE(_scenegraph->loadScene(absPath("${TESTDIR}/ScenegraphTest"), absPath("${TESTDIR}")));
////}
////
////TEST_F(SceneGraphTest, Reinitializing) {
////    
////    // Existing scenegraph should load
////    EXPECT_TRUE(_scenegraph->loadScene(absPath("${TESTDIR}/ScenegraphTest"), absPath("${TESTDIR}")));
////    
////    _scenegraph->deinitialize();
////    
////    // Existing scenegraph should load
////    EXPECT_TRUE(_scenegraph->loadScene(absPath("${TESTDIR}/ScenegraphTest"), absPath("${TESTDIR}")));
////    // TODO need to check for correctness
////}
//

//
//TEST_F(SceneGraphLoaderTest, IllformedFileTestWrongCommonFolder) {
//    const std::string file = absPath("${TESTDIR}/SceneGraphLoaderTest/illformedWrongCommon.scene");
//
//    std::vector<openspace::SceneGraphNode*> nodes;
//    bool success = openspace::SceneGraphLoader::load(file, nodes);
//
//    EXPECT_FALSE(success) << "Unsuccessful loading";
//    EXPECT_TRUE(nodes.empty()) << "Empty scenegraph nodes list";
//}
//


//TEST_F(SceneLoaderTest, IllformedFileTestNonExistingCommonFolder) {
//    const std::string file = absPath("${TESTDIR}/SceneGraphLoaderTest/illformedNonExistingCommon.scene");
//
//    std::vector<openspace::SceneGraphNode*> nodes;
//    bool success = openspace::SceneGraphLoader::load(file, nodes);
//    EXPECT_FALSE(success) << "Unsuccessful loading";
//   EXPECT_TRUE(nodes.empty()) << "Empty scenegraph nodes list";
//}
