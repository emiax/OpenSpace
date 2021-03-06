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

#include <openspace/util/openspacemodule.h>

#include <ghoul/filesystem/filesystem>
#include <ghoul/logging/logmanager.h>

#include <algorithm>

namespace {
    const std::string _loggerCat = "OpenSpaceModule";
    const std::string ModuleBaseToken = "MODULE_";
}

namespace openspace {

OpenSpaceModule::OpenSpaceModule(std::string name) {
    ghoul_assert(!name.empty(), "Name must not be empty");
    
    setName(name);
}

void OpenSpaceModule::initialize() {
    std::string upperName = name();
    std::transform(upperName.begin(), upperName.end(), upperName.begin(), toupper);
    
    std::string moduleToken =
        ghoul::filesystem::FileSystem::TokenOpeningBraces +
        ModuleBaseToken +
        upperName +
        ghoul::filesystem::FileSystem::TokenClosingBraces;

    std::string path = modulePath();
    LDEBUG("Registering module path: " << moduleToken << ": " << path);
    FileSys.registerPathToken(moduleToken, path);
    
    internalInitialize();
}

void OpenSpaceModule::deinitialize() {
    internalDeinitialize();
}

std::vector<Documentation> OpenSpaceModule::documentations() const {
    return {};
}

std::string OpenSpaceModule::modulePath() const {
    std::string moduleName = name();
    std::transform(moduleName.begin(), moduleName.end(), moduleName.begin(), tolower);

    if (FileSys.directoryExists("${MODULES}/" + moduleName))
        return absPath("${MODULES}/" + moduleName);

#ifdef EXTERNAL_MODULES_PATHS

#endif
    throw ghoul::RuntimeError(
        "Could not resolve path for module '" + name() + "'",
        "OpenSpaceModule"
    );
}

void OpenSpaceModule::internalInitialize() {}
void OpenSpaceModule::internalDeinitialize() {}

} // namespace openspace
