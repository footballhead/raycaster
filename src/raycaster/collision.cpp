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

collision_result find_collision(level const& lvl, point2f const& origin,
    float direction, float reference_direction, float max_distance)
{
    auto const no_result
        = collision_result{-1.f, {-1.f, -1.f}, 0u, 0.f, direction};

    auto const march_vector = vector2f{direction, max_distance};
    auto const ray_line = line2f{origin, origin + march_vector};

    std::vector<collision_result> intersections;

    for (auto const& wall : lvl.walls) {
        point2f cross_point{0.f, 0.f};
        float t = 0.f;
        if (find_intersection(ray_line, wall.data, cross_point, t)) {
            auto const exact_line = line2f{origin, cross_point};
            intersections.push_back(collision_result{
                exact_line.length(), cross_point, wall.texture, t, direction});
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
            intersections.push_back(collision_result{exact_line.length(),
                cross_point, sprite.texture, t, direction});
        }
    }

    if (intersections.empty()) {
        return no_result;
    }

    auto closest_I = intersections.cbegin();
    for (auto I = intersections.cbegin(); I < intersections.cend(); I++) {
        auto const& collision = *I;
        auto const& closest_so_far = *closest_I;
        if (collision.distance < closest_so_far.distance) {
            closest_I = I;
        }
    }

    return *closest_I;
}

} // namespace raycaster
