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

#include <openspace/scripting/scriptscheduler.h>

#include <openspace/engine/openspaceengine.h>

#include <openspace/scripting/scriptengine.h>
#include <openspace/util/spicemanager.h> // parse time
#include <ghoul/logging/logmanager.h>
#include <ghoul/filesystem/filesystem>


namespace openspace {
namespace scripting {

namespace {
    const std::string _loggerCat = "ScriptScheduler";

    const std::string KEY_TIME = "Time";
    const std::string KEY_FORWARD_SCRIPT = "ReversibleLuaScript.Forward";
    const std::string KEY_BACKWARD_SCRIPT = "ReversibleLuaScript.Backward";
}


ScheduledScript::ScheduledScript(const ghoul::Dictionary& dict) 
    : ScheduledScript() // default init first
{
    std::string timeStr;
    if (dict.getValue(KEY_TIME, timeStr)) {
        time = SpiceManager::ref().ephemerisTimeFromDate(timeStr);

        if (!dict.getValue(KEY_FORWARD_SCRIPT, script.forwardScript)) {
            LERROR("Unable to read " << KEY_FORWARD_SCRIPT);
        }
        if (!dict.getValue(KEY_BACKWARD_SCRIPT, script.backwardScript)) {
            LERROR("Unable to read " << KEY_BACKWARD_SCRIPT);
        }
    }
    else {
        LERROR("Unable to read " << KEY_TIME);
    }
}

bool ScheduledScript::CompareByTime(const ScheduledScript& s1, const ScheduledScript& s2){
    return s1.time < s2.time;
}



void ScriptScheduler::loadScripts(const std::string& filepath, lua_State* L) {
    ghoul::Dictionary timedScriptsDict;
    try {
        ghoul::lua::loadDictionaryFromFile(absPath(filepath), timedScriptsDict, L);
    }
    catch (const ghoul::RuntimeError& e) {
        LERROR(e.what());
        return;
    }
    loadScripts(timedScriptsDict);
}

void ScriptScheduler::loadScripts(const ghoul::Dictionary& dict) {
    for (size_t i = 0; i < dict.size(); ++i) {
        std::string id = std::to_string(i + 1);
        const ghoul::Dictionary& timedScriptDict = dict.value<ghoul::Dictionary>(id);
        _scheduledScripts.push_back(ScheduledScript(timedScriptDict));
    }

    // Sort scripts by time
    std::stable_sort(_scheduledScripts.begin(), _scheduledScripts.end(), &ScheduledScript::CompareByTime);

    // Ensure _currentIndex and _currentTime is accurate after new scripts was added
    double lastTime = _currentTime;
    rewind();
    progressTo(lastTime);
}

void ScriptScheduler::rewind() {
    _currentIndex = 0;
    _currentTime = -DBL_MAX;
}


void ScriptScheduler::clearSchedule() {
    rewind();
    _scheduledScripts.clear();
}

std::queue<std::string> ScriptScheduler::progressTo(double newTime) {
    std::queue<std::string> triggeredScripts;
    if (newTime > _currentTime) {
        while(_currentIndex < _scheduledScripts.size() && _scheduledScripts[_currentIndex].time <= newTime){
            triggeredScripts.push(_scheduledScripts[_currentIndex].script.forwardScript);
            _currentIndex++;
        }
    }
    else {
        while (0 < _currentIndex && _scheduledScripts[_currentIndex - 1].time > newTime) {
            triggeredScripts.push(_scheduledScripts[_currentIndex - 1].script.backwardScript);
            _currentIndex--;
        }
    }

    _currentTime = newTime;
    return triggeredScripts;
}

std::queue<std::string> ScriptScheduler::progressTo(const std::string& timeStr) {
    return std::move(progressTo(SpiceManager::ref().ephemerisTimeFromDate(timeStr)));
}

double ScriptScheduler::currentTime() const { 
    return _currentTime; 
};

const std::vector<ScheduledScript>& ScriptScheduler::allScripts() const { 
    return _scheduledScripts; 
};



/////////////////////////////////////////////////////////////////////
//                     Lua library functions                       //
/////////////////////////////////////////////////////////////////////

namespace luascriptfunctions {
    int loadTimedScripts(lua_State* L) {
        using ghoul::lua::luaTypeToString;
        int nArguments = lua_gettop(L);
        if (nArguments != 1)
            return luaL_error(L, "Expected %i arguments, got %i", 1, nArguments);

        std::string missionFileName = luaL_checkstring(L, -1);
        if (missionFileName.empty()) {
            return luaL_error(L, "filepath string is empty");
        }
        
        OsEng.scriptScheduler().loadScripts(missionFileName, L);
    }

    int clear(lua_State* L) {
        using ghoul::lua::luaTypeToString;
        int nArguments = lua_gettop(L);
        if (nArguments != 0)
            return luaL_error(L, "Expected %i arguments, got %i", 0, nArguments);

        OsEng.scriptScheduler().clearSchedule();
    }
} // namespace luascriptfunction



LuaLibrary ScriptScheduler::luaLibrary() {    
    return {
        "scriptScheduler",
        {
            {
                "load",
                &luascriptfunctions::loadTimedScripts,
                "string",
                "Load timed scripts from file"
            },
            {
                "clear",
                &luascriptfunctions::clear,
                "",
                "clears all scheduled scripts"
            },
        }
    };
}

} // namespace scripting

} // namespace openspace
