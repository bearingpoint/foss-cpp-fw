#ifndef __TESSELATE_VEC2_H__
#define __TESSELATE_VEC2_H__

#include <boglfw/utils/earcut.hpp>
#include <glm/vec2.hpp>

namespace mapbox {
namespace util {

template <>
struct nth<0, glm::vec2> {
    inline static auto get(const glm::vec2 &t) {
        return t.x;
    };
};
template <>
struct nth<1, glm::vec2> {
    inline static auto get(const glm::vec2 &t) {
        return t.y;
    };
};

} // namespace util
} // namespace mapbox

#endif // __TESSELATE_VEC2_H__
