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

#include <modules/server/include/topics/shortcuttopic.h>

#include <modules/server/include/connection.h>
#include <modules/server/include/jsonconverters.h>
#include <modules/volume/transferfunctionhandler.h>
#include <openspace/engine/globals.h>
#include <openspace/engine/virtualpropertymanager.h>
#include <openspace/engine/windowdelegate.h>
#include <openspace/interaction/navigationhandler.h>
#include <openspace/network/parallelpeer.h>
#include <openspace/query/query.h>
#include <openspace/rendering/luaconsole.h>
#include <openspace/rendering/renderengine.h>
#include <openspace/rendering/screenspacerenderable.h>
#include <openspace/scene/scene.h>
#include <ghoul/logging/logmanager.h>

using nlohmann::json;

namespace {
const char* _loggerCat = "ShortcutTopic";
const char* CommandKey = "command";
const char* ApplyCommand = "apply";
const char* SubscribeCommand = "subscribe";
}

namespace openspace {

void ShortcutTopic::handleJson(const nlohmann::json& json) {
    std::string command = json.at(CommandKey).get<std::string>();

    nlohmann::json response;
    if (command == ApplyCommand) {
        // global::shortcutmanager apply command
    } else if (command == SubscribeCommand) {
        _isSubscribing = true;
        response = listShortcuts();
        // Set up subscription to shortcut manager
        // Set up callback to send data to client
        // Send current data
    }
    _connection->sendJson(response);
}

bool ShortcutTopic::isDone() const {
    return !_isSubscribing;
}

json ShortcutTopic::listShortcuts() const {
    json payload {};
    // Extend payload with shortcuts from shortcutmanager.
    return wrappedPayload(payload);
}

} // namespace openspace
