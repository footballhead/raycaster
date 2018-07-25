#pragma once

#include <cstdint>

struct color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

constexpr color hueToRgb(float hue)
{
	auto sixth = 1.f/6.f;

	while (hue > 1.f) {
		hue -= 1.f;
	}

	if (hue < sixth) {
		return {
			255,
			static_cast<uint8_t>(hue / sixth * 255),
			0
		};
	} else if (hue < 2*sixth) {
		return {
			static_cast<uint8_t>(255 - ((hue-sixth) / sixth) * 255),
			255,
			0
		};
	} else if (hue < 3*sixth) {
		return {
			0,
			255,
			static_cast<uint8_t>((hue-2*sixth) / sixth * 255),
		};
	} else if (hue < 4*sixth) {
		return {
			0,
			static_cast<uint8_t>(255 - ((hue-3*sixth) / sixth) * 255),
			255
		};
	} else if (hue < 5*sixth) {
		return {
			static_cast<uint8_t>((hue-4*sixth) / sixth * 255),
			0,
			255
		};
	} else {
		return {
			255,
			0,
			static_cast<uint8_t>(255 - ((hue-5*sixth) / sixth) * 255),
		};
	}
}
