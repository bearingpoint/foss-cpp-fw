/*
 * json.h
 *
 * Created on: Sep 26, 2024
 * Author: bogdan ionita <bogdan.ionita@bearingpoint.com>
 */

#ifdef ENABLE_PERF_PROFILING

#include "section.h"
#include "frameCapture.h"

#include <nlohmann/json.hpp>

#include <vector>
#include <memory>

namespace perf {
namespace json {

nlohmann::json callFrame(perf::sectionData const& s);
nlohmann::json callTree(std::vector<std::shared_ptr<perf::sectionData>> t);
nlohmann::json topHits(std::vector<perf::sectionData> data);

nlohmann::json sequenceCapture(std::vector<perf::FrameCapture::frameData> data);

} // namespace json
} // namespace perf

#endif // ENABLE_PERF_PROFILING
