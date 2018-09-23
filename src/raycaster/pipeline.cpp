#include "pipeline.hpp"

#include "camera.hpp"
#include "collision.hpp"
#include "level.hpp"

namespace {
constexpr float PI_OVER_2 = M_PI / 2.f;
}

namespace raycaster {

// First "stage" of the pipeline:
std::vector<collision_result> cast_rays(
    int num_rays, level const& lvl, camera const& cam)
{
    std::vector<collision_result> collision_buffer;
    collision_buffer.reserve(num_rays);

    // Fire a ray for each column on the screen and save the result.
    for (int i = 0; i < num_rays; ++i) {
        auto const width_percent = i / static_cast<float>(num_rays);
        auto const proj_point_interp
            = linear_interpolate(cam.get_projection_plane(), width_percent);

        auto const diff = proj_point_interp - cam.get_position();
        auto const camera_ray_radians = atan2(diff.y, diff.x);

        collision_buffer.push_back(find_collision(lvl, proj_point_interp,
            camera_ray_radians, cam.get_rotation(), cam.get_far()));

        auto& last_result = collision_buffer.back();
        if (last_result.distance < 0) {
            continue;
        }

        // Fix fish eye distortion by changing distance from euclidean to
        // projected on the projection plane using basic trig.
        auto const local_ray_radians = cam.get_rotation() - camera_ray_radians;
        last_result.distance *= sin(PI_OVER_2 - std::abs(local_ray_radians));
    }

    return collision_buffer;
}

} // namespace raycaster
