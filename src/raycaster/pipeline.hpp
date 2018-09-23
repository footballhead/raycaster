#pragma once

#include <vector>

namespace raycaster {

class camera;
class collision_result;
class level;

/// Stage 1: cast a bunch of rays and find their intersections
std::vector<std::vector<collision_result>> cast_rays(
    int num_rays, level const& lvl, camera const& cam);

} // namespace raycaster
