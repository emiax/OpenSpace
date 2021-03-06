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

#include <modules/base/ephemeris/spiceephemeris.h>

#include <openspace/util/spicemanager.h>
#include <openspace/util/time.h>

#include <ghoul/filesystem/filesystem.h>

#include <openspace/documentation/verifier.h>

namespace {
    const std::string KeyBody = "Body";
    const std::string KeyObserver = "Observer";
    const std::string KeyKernels = "Kernels";

    const std::string ReferenceFrame = "GALACTIC";
}

namespace openspace {
    
Documentation SpiceEphemeris::Documentation() {
    using namespace openspace::documentation;

    return {
        "Spice Translation",
        "base_translation_spicetranslation",
        {
            {
                "Type",
                new StringEqualVerifier("SpiceEphemeris"),
                "",
                Optional::No
            },
            {
                KeyBody,
                new StringAnnotationVerifier("A valid SPICE NAIF name or identifier"),
                "This is the SPICE NAIF name for the body whose translation is to be "
                "computed by the SpiceTranslation. It can either be a fully qualified "
                "name (such as 'EARTH') or a NAIF integer id code (such as '399').",
                Optional::No
            },
            {
                KeyObserver,
                new StringAnnotationVerifier("A valid SPICE NAIF name or identifier"),
                "This is the SPICE NAIF name for the parent of the body whose "
                "translation is to be computed by the SpiceTranslation. It can either be "
                "a fully qualified name (such as 'SOLAR SYSTEM BARYCENTER') or a NAIF "
                "integer id code (such as '0').",
                Optional::No
            },
            {
                KeyKernels,
                new OrVerifier(
                    new TableVerifier({
                        { "*", new StringVerifier }
                    }),
                    new StringVerifier
                ),
                "A single kernel or list of kernels that this SpiceTranslation depends "
                "on. All provided kernels will be loaded before any other operation is "
                "performed.",
                Optional::Yes
            }
        },
        Exhaustive::Yes
    };
}

SpiceEphemeris::SpiceEphemeris(const ghoul::Dictionary& dictionary)
    : _target("target", "Target", "")
    , _origin("origin", "Origin", "")
    , _kernelsLoadedSuccessfully(true)
{
    documentation::testSpecificationAndThrow(
        Documentation(),
        dictionary,
        "SpiceEphemeris"
    );

    _target = dictionary.value<std::string>(KeyBody);
    _origin = dictionary.value<std::string>(KeyObserver);

    auto loadKernel = [](const std::string& kernel) {
        if (!FileSys.fileExists(kernel)) {
            throw SpiceManager::SpiceException("Kernel '" + kernel + "' does not exist");
        }

        try {
            SpiceManager::ref().loadKernel(kernel);
        }
        catch (const SpiceManager::SpiceException& exception) {
            LERRORC("SpiceEphemeris", exception.message);
        }
    };

    if (dictionary.hasKey(KeyKernels)) {
        // Due to the specification, we can be sure it is either a Dictionary or a string
        if (dictionary.hasValue<std::string>(KeyKernels)) {
            std::string kernel = dictionary.value<std::string>(KeyKernels);
            loadKernel(kernel);
        }
        else {
            ghoul::Dictionary kernels = dictionary.value<ghoul::Dictionary>(KeyKernels);
            for (size_t i = 1; i <= kernels.size(); ++i) {
                std::string kernel = kernels.value<std::string>(std::to_string(i));
                loadKernel(kernel);
            }
        }
    }
}
    
glm::dvec3 SpiceEphemeris::position() const {
    return _position;
}

void SpiceEphemeris::update(const UpdateData& data) {
    double lightTime = 0.0;
    _position = SpiceManager::ref().targetPosition(
        _target, _origin, ReferenceFrame, {}, data.time, lightTime
    ) * glm::pow(10.0, 3.0);
}

} // namespace openspace
