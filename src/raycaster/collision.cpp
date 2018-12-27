#include "collision.hpp"

#include "intersection.hpp"
#include "level.hpp"

#include <mymath/mymath.hpp>

#include <vector>

using namespace mymath;

namespace {
constexpr auto PI_OVER_2 = static_cast<float>(M_PI / 2.f);
} // namespace

namespace raycaster {

render_candidates find_collision(level const& lvl, point2f const& origin,
    float direction, float reference_direction, float max_distance)
{
    auto const march_vector = vector2f{direction, max_distance};
    auto const ray_line = line2f{origin, origin + march_vector};

    render_candidates candidates{direction, {}};
    auto& hits = candidates.hits;

    for (auto const& wall : lvl.walls) {
        point2f cross_point{0.f, 0.f};
        float t = 0.f;
        if (find_intersection(ray_line, wall.data, cross_point, t)) {
            auto const exact_line = line2f{origin, cross_point};
            // HACK! For walls, we want the texture to repeat across the length,
            // but the `t` we get normalizes across the line. So correct for
            // that here.
            t *= wall.data.length();
            t -= std::floor(t);
            hits.push_back(
                ray_hit{exact_line.length(), cross_point, wall.texture, t});
        }
    }

    for (auto const& sprite : lvl.sprites) {
        auto const sprite_plane = line2f{
            sprite.data + vector2f{reference_direction + PI_OVER_2, 0.5f},
            sprite.data + vector2f{reference_direction - PI_OVER_2, 0.5f}};
        point2f cross_point{0.f, 0.f};
        float t = 0.f;
        if (find_intersection(ray_line, sprite_plane, cross_point, t)) {
            auto const exact_line = line2f{origin, cross_point};
            hits.push_back(
                ray_hit{exact_line.length(), cross_point, sprite.texture, t});
        }
    }

    std::sort(hits.begin(), hits.end());
    return candidates;
}

} // namespace raycaster
