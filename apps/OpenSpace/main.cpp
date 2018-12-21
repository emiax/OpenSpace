/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014-2018                                                               *
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


#include <ghoul/opengl/ghoul_gl.h>

#define __gl_h_
#include <ghoul/opengl/texture.h>

#include <GLFW/glfw3.h>
#include <memory>
#include <iostream>
#include <openspace/engine/openspaceengine.h>
#include <openspace/engine/globals.h>
#include <openspace/engine/windowdelegate.h>
#include <openspace/documentation/documentation.h>
#include <ghoul/ghoul.h>
#include <ghoul/filesystem/filesystem.h>
#include <ghoul/filesystem/file.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/logging/consolelog.h>


#include <openspace/engine/configuration.h>
#include <ghoul/lua/ghoul_lua.h>

#include <glm/gtc/matrix_transform.hpp>
#include <atomic>
#include <chrono>

#include <openspace/rendering/helper.h>
#include <ghoul/opengl/textureunit.h>


//#ifdef WIN32
//#include <Windows.h>
//#endif // WIN32

constexpr const char* _loggerCat = "main";
GLFWwindow* guiWindow;
GLFWwindow* renderContext;

glm::ivec2 resolution = glm::ivec2(1280, 720);

int main(int argc, char** argv) {

    ghoul::opengl::TextureUnit::setCurrentContextFunction([]() {
        return static_cast<void*>(glfwGetCurrentContext());
    });

    ghoul::initialize();

    if (!glfwInit()) {
        std::cout << "Error when initializing glfw.";
        return 0;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    openspace::WindowDelegate& windowDelegate = openspace::global::windowDelegate;
    windowDelegate.terminate = []() { return; };
    windowDelegate.setBarrier = [](bool enabled) {
        return;
    };
    windowDelegate.setSynchronization = [](bool enabled) {
        return;
    };
    windowDelegate.clearAllWindows = [](const glm::vec4& clearColor) {
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    };
    windowDelegate.windowHasResized = []() {
        return false;
    };
    windowDelegate.averageDeltaTime = []() { return 1.0 / 60.0; };
    windowDelegate.deltaTime = []() { return 1.0 / 60.0; };
    windowDelegate.applicationTime = []() { return 0.0; };
    windowDelegate.mousePosition = []() {
        return glm::vec2(0, 0);
    };
    windowDelegate.mouseButtons = [](int maxNumber) {
        return uint32_t(0);
    };
    windowDelegate.currentWindowSize = []() {
        return resolution;
    };
    windowDelegate.currentSubwindowSize = []() {
        return resolution;
    };
    windowDelegate.currentWindowResolution = []() {
        return resolution;
    };
    windowDelegate.currentDrawBufferResolution = []() {
        return resolution;
    };
    windowDelegate.currentViewportSize = []() {
        return resolution;
    };
    windowDelegate.dpiScaling = []() {
        return glm::vec2(1, 1);
    };
    windowDelegate.currentNumberOfAaSamples = []() {
        return 1;
    };
    windowDelegate.isRegularRendering = []() {
        return true;
    };
    windowDelegate.hasGuiWindow = []() {
        return false;
    };
    windowDelegate.isGuiWindow = []() {
        return false;
    };
    windowDelegate.isMaster = []() { return true; };
    windowDelegate.isUsingSwapGroups = []() {
        return false;
    };
    windowDelegate.isSwapGroupMaster = []() {
        return false;
    };
    windowDelegate.viewProjectionMatrix = []() {
        return glm::perspective(45.f, 16.f / 9.f, 0.1f, 1000.f);
    };
    windowDelegate.modelMatrix = []() {
        return glm::mat4(1.f);
    };
    windowDelegate.setNearFarClippingPlane = [](float nearPlane, float farPlane) {
    };
    windowDelegate.setEyeSeparationDistance = [](float distance) {
    };
    windowDelegate.viewportPixelCoordinates = []() {
        return glm::ivec4(0, resolution.x, 0, resolution.y);
    };
    windowDelegate.isExternalControlConnected = []() {
        return false;
    };
    windowDelegate.sendMessageToExternalControl = [](const std::vector<char>& message) {
        return;
    };
    windowDelegate.isSimpleRendering = []() {
        return true;
    };
    windowDelegate.isFisheyeRendering = []() {
        return false;
    };
    windowDelegate.takeScreenshot = [](bool applyWarping) {
        return;
    };
    windowDelegate.swapBuffer = []() {
        GLFWwindow* w = glfwGetCurrentContext();
        glfwSwapBuffers(w);
    };
    windowDelegate.nWindows = []() {
        return 1;
    };
    windowDelegate.currentWindowId = []() {
        return 0;
    };
    windowDelegate.openGLProcedureAddress = [](const char* func) {
        return glfwGetProcAddress(func);
    };

    guiWindow = glfwCreateWindow(resolution.x, resolution.y, "OpenSpace UI", NULL, NULL);
    if (!guiWindow)
    {
        std::cout << "Error when creating window.";
        glfwTerminate();
        exit(EXIT_FAILURE);
    }


    {
        using namespace ghoul::logging;
        LogManager::initialize(LogLevel::Debug, LogManager::ImmediateFlush::Yes);
        LogMgr.addLog(std::make_unique<ConsoleLog>());
    }


    // Register the path of the executable,
    // to make it possible to find other files in the same directory.
    FileSys.registerPathToken(
        "${BIN}",
        ghoul::filesystem::File(absPath(argv[0])).directoryName(),
        ghoul::filesystem::FileSystem::Override::Yes
    );

    try {
        using namespace openspace;


        LDEBUG("Finding configuration");
        std::string configurationFilePath = configuration::findConfiguration();
        configurationFilePath = absPath(configurationFilePath);

        if (!FileSys.fileExists(configurationFilePath)) {
            LFATALC("main", "Could not find configuration: " + configurationFilePath);
            exit(EXIT_FAILURE);
        }
        LINFO(fmt::format("Configuration Path: '{}'", configurationFilePath));

        // Loading configuration from disk
        LDEBUG("Loading configuration from disk");
        global::configuration = configuration::loadConfigurationFromFile(
            configurationFilePath
        );

        // If the user requested a commandline-based configuation script that should
        // overwrite some of the values, this is the time to do it

        parseLuaState(global::configuration);
    }
    catch (const openspace::documentation::SpecificationError& e) {
        LFATALC("main", "Loading of configuration file failed");
        for (const openspace::documentation::TestResult::Offense& o : e.result.offenses) {
            LERRORC(o.offender, ghoul::to_string(o.reason));
        }
        for (const openspace::documentation::TestResult::Warning& w : e.result.warnings) {
            LWARNINGC(w.offender, ghoul::to_string(w.reason));
        }
        exit(EXIT_FAILURE);
    }
    catch (const ghoul::RuntimeError& e) {
        // Write out all of the information about the exception and flush the logs
        LFATALC(e.component, e.message);
        if (ghoul::logging::LogManager::isInitialized()) {
            LogMgr.flushLogs();
        }
        return EXIT_FAILURE;
    }

  
    glfwMakeContextCurrent(nullptr);

    // Uncomment to hide render context: glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    renderContext = glfwCreateWindow(
        resolution.x, resolution.y, "OpenSpace Rendering", NULL, guiWindow
    );

    glfwMakeContextCurrent(renderContext);
    glbinding::initialize(glfwGetProcAddress);
   
    ghoul::opengl::Texture renderTexture(glm::uvec3(resolution.x, resolution.y, 1));
    renderTexture.bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, resolution.x, resolution.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    GLuint renderBuffer;
    glGenFramebuffers(1, &renderBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, renderBuffer);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        renderTexture,
        0
    );

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer not complete";
        return 0;
    }

    // Setup texture and fbo for gui window.
    ghoul::opengl::Texture guiWindowTexture(glm::uvec3(resolution.x, resolution.y, 1));
    guiWindowTexture.bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, resolution.x, resolution.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    GLuint guiWindowFbo;
    glGenFramebuffers(1, &guiWindowFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, guiWindowFbo);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        guiWindowTexture,
        0
    );
    // Done

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    std::atomic_bool keepRunning = true;

    glfwMakeContextCurrent(guiWindow);
    glbinding::useCurrentContext();

    std::thread renderThread([&windowDelegate, &keepRunning, &renderTexture, &renderBuffer, &guiWindowFbo] () {
        glfwMakeContextCurrent(renderContext);
        glbinding::useCurrentContext();


        // Bind renderbuffer in order to initialize renderengine with the right buffer.
        glBindFramebuffer(GL_FRAMEBUFFER, renderBuffer);

        openspace::global::openSpaceEngine.registerPathTokens();
        openspace::global::openSpaceEngine.initialize();
        openspace::global::openSpaceEngine.initializeGL();
        while (keepRunning) {
            std::cout << "A";

            openspace::global::openSpaceEngine.preSynchronization();
            openspace::global::openSpaceEngine.postSynchronizationPreDraw();

            glBindFramebuffer(GL_FRAMEBUFFER, renderBuffer);

            glClearColor(0.f, 0.f, 0.f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            openspace::global::openSpaceEngine.render(
                glm::mat4(1.f),
                windowDelegate.modelMatrix(),
                windowDelegate.viewProjectionMatrix()
            );
            openspace::global::openSpaceEngine.drawOverlays();
            openspace::global::openSpaceEngine.postDraw();
            glFinish();

            /* Render from framebuffer to default one.*/
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClearColor(0.f, 0.f, 0.f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDisable(GL_CULL_FACE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_DEPTH_TEST);

            openspace::rendering::helper.renderBox(
                glm::vec2(0.5), glm::vec2(1.0), glm::vec4(1.f),
                renderTexture, openspace::rendering::Helper::Anchor::Center);
            glEnable(GL_DEPTH_TEST);


            /* Render from framebuffer to shared texture */

            glBindFramebuffer(GL_FRAMEBUFFER, guiWindowFbo);
            glClearColor(0.f, 0.f, 0.f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDisable(GL_CULL_FACE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_DEPTH_TEST);

            openspace::rendering::helper.renderBox(
                glm::vec2(0.5), glm::vec2(1.0), glm::vec4(1.f),
                renderTexture, openspace::rendering::Helper::Anchor::Center);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            glFinish();
            glEnable(GL_DEPTH_TEST);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glfwSwapBuffers(renderContext);


        }

        openspace::global::openSpaceEngine.deinitializeGL();
        openspace::global::openSpaceEngine.deinitialize();
    });




    openspace::rendering::Helper guiHelper;
    guiHelper.initialize();

    while (!glfwWindowShouldClose(guiWindow) && !glfwWindowShouldClose(renderContext)) {
        std::cout << "B";
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);

        guiHelper.renderBox(
            glm::vec2(0.5), glm::vec2(1.0), glm::vec4(1.f),
            guiWindowTexture, openspace::rendering::Helper::Anchor::Center);
        glEnable(GL_DEPTH_TEST);

        glFinish();
        glfwSwapBuffers(guiWindow);
        glfwPollEvents();
    }

    glfwMakeContextCurrent(guiWindow);
    glbinding::useCurrentContext();

    guiHelper.deinitialize();

    glfwMakeContextCurrent(nullptr);


    keepRunning = false;

    renderThread.join();
    


    return 0;
}

