#pragma once

#include <vector>

#include "collision.hpp"

namespace raycaster {

class camera;
class level;

/// All the hits for all rays which, together, gives enough information to fill
/// the framebuffer with pixels.
using candidate_buffer = std::vector<render_candidates>;

/// Stage 1: cast a bunch of rays and find their intersections
candidate_buffer cast_rays(int num_rays, level const& lvl, camera const& cam);

/// Stage 2: rasterize (see raycaster_app)

} // namespace raycaster
