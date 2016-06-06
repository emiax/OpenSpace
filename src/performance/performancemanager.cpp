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

#include <openspace/performance/performancemanager.h>

#include <openspace/scene/scenegraphnode.h>
#include <openspace/performance/performancelayout.h>

#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/sharedmemory.h>

namespace {
    const std::string _loggerCat = "PerformanceManager";
}

namespace openspace {
namespace performance {

const std::string PerformanceManager::PerformanceMeasurementSharedData =
    "OpenSpacePerformanceMeasurementSharedData";

PerformanceManager::PerformanceManager()
    : _performanceMemory(nullptr)
{
}

PerformanceManager::~PerformanceManager() {
    if (ghoul::SharedMemory::exists(PerformanceMeasurementSharedData))
        ghoul::SharedMemory::remove(PerformanceMeasurementSharedData);
}

bool PerformanceManager::isMeasuringPerformance() const {
    return _doPerformanceMeasurements;
}

void PerformanceManager::storeIndividualPerformanceMeasurement(std::string identifier, long long us) {

}

void PerformanceManager::storeScenePerformanceMeasurements(
                                           const std::vector<SceneGraphNode*>& sceneNodes)
{
    using namespace performance;

    int nNodes = static_cast<int>(sceneNodes.size());
    if (!_performanceMemory) {
        // Compute the total size
        const int totalSize = sizeof(PerformanceLayout);
        LINFO("Create shared memory of " << totalSize << " bytes");

        try {
            ghoul::SharedMemory::remove(PerformanceMeasurementSharedData);
        }
        catch (const ghoul::SharedMemory::SharedMemoryError& e) {
            LINFOC(e.component, e.what());
        }

        ghoul::SharedMemory::create(PerformanceMeasurementSharedData, totalSize);
        _performanceMemory = new ghoul::SharedMemory(PerformanceMeasurementSharedData);
        void* ptr = _performanceMemory->memory();

        // Using the placement-new to create a PerformanceLayout in the shared memory
        PerformanceLayout* layout = new (ptr) PerformanceLayout(nNodes);

        for (int i = 0; i < nNodes; ++i) {
            SceneGraphNode* node = sceneNodes[i];

            memset(layout->sceneGraphEntries[i].name, 0, PerformanceLayout::LengthName);
#ifdef _MSC_VER
            strcpy_s(layout->sceneGraphEntries[i].name, node->name().length() + 1, node->name().c_str());
#else
            strcpy(layout->sceneGraphEntries[i].name, node->name().c_str());
#endif

            layout->sceneGraphEntries[i].currentRenderTime = 0;
            layout->sceneGraphEntries[i].currentUpdateRenderable = 0;
            layout->sceneGraphEntries[i].currentUpdateEphemeris = 0;
        }
    }

    void* ptr = _performanceMemory->memory();
    PerformanceLayout* layout = reinterpret_cast<PerformanceLayout*>(ptr);
    _performanceMemory->acquireLock();
    for (int i = 0; i < nNodes; ++i) {
        SceneGraphNode* node = sceneNodes[i];
        SceneGraphNode::PerformanceRecord r = node->performanceRecord();
        PerformanceLayout::SceneGraphPerformanceLayout& entry = layout->sceneGraphEntries[i];

        entry.renderTime[entry.currentRenderTime] = r.renderTime / 1000.f;
        entry.updateEphemeris[entry.currentUpdateEphemeris] = r.updateTimeEphemeris / 1000.f;
        entry.updateRenderable[entry.currentUpdateRenderable] = r.updateTimeRenderable / 1000.f;

        entry.currentRenderTime = (entry.currentRenderTime + 1) % PerformanceLayout::NumberValues;
        entry.currentUpdateEphemeris = (entry.currentUpdateEphemeris + 1) % PerformanceLayout::NumberValues;
        entry.currentUpdateRenderable = (entry.currentUpdateRenderable + 1) % PerformanceLayout::NumberValues;
    }
    _performanceMemory->releaseLock();
}

} // namespace performance
} // namespace openspace