#pragma once

#include "texture_cache.hpp"

#include <mymath/mymath.hpp>

#include <atomic>
#include <functional>
#include <thread>
#include <vector>

struct SDL_Surface;

namespace raycaster {

class camera;
struct level;

namespace detail {
constexpr auto num_threads = 6;
} // namespace detail

class render_pipeline {
public:
    render_pipeline(texture_cache cache);

    void render(level const& lvl, camera const& cam, SDL_Surface& framebuffer);

private:
    texture_cache _texture_cache;

    using rendering_fn = std::function<void(unsigned)>;

    // Purposefully generic name for a mess of a function
    void do_work(unsigned thread_id, level const& lvl, camera const& cam, SDL_Surface& fb);

    struct rendering_context {
        unsigned id;
        std::atomic<bool> can_start{false};
        std::atomic<bool> done{false};
        std::function<void(unsigned)> work;
    };

    std::array<rendering_context, detail::num_threads> _contexts;
    std::array<std::thread, detail::num_threads> _threads;
};

} // namespace raycaster
