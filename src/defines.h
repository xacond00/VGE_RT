#pragma once
// Created by Ondrej Ac (xacond00)
#include <array>
#include <chrono>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
using Float = float;
using Uint = unsigned int;
using Int = int;
constexpr Float InfF = 1e4;
constexpr Float EpsF = 1e-4;
constexpr Float Eps6F = 1e-6;
constexpr Float PiF = 3.14159265358979323846;
constexpr Float PihF = 0.5 * 3.14159265358979323846;
constexpr Float Pi2F = 2.0 * 3.14159265358979323846;

inline bool within(Float t, Float min, Float max) { return (t >= min) && (t <= max); }
inline bool inside(Float t, Float min, Float max) { return (t > min) && (t < max); }

inline double timer() {
	auto t = std::chrono::high_resolution_clock::now();
	return std::chrono::duration<double>(t.time_since_epoch()).count();
}
inline double timer(double t1) {
	auto t = std::chrono::high_resolution_clock::now();
	return std::chrono::duration<double>(t.time_since_epoch()).count() - t1;
}
inline static constexpr Float todeg(Float a) { return a * Float(180.0 / PiF); }
inline static constexpr Float torad(Float a) { return a * Float(PiF / 180.0f); }

inline uint32_t xorshift32() {
	thread_local static uint32_t x = 0x6f9f;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return x;
}

inline Float randfl() {
	uint32_t x = 0x3f800000 | (xorshift32() & 0x007FFFFF);
	return *(float *)&x - 1.f;
}

inline uint32_t pack_rgb(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
	return r + (g << 8) + (b << 16) + (a << 24);
}

inline uint32_t pack_bgr(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
	return b + (g << 8) + (r << 16) + (a << 24);
}

inline Float fast_sin(const Float &x) {
	Float x2 = x * x;
	return x + x * x2 *
				   (Float(-1.6666656684e-1) +
					x2 * (Float(8.3330251389e-3) + x2 * (Float(-1.9807418727e-4) + x2 * Float(2.6019030676e-6))));
}

inline Float fract(const Float &x) { return x - std::floor(x); }

inline Float mod(const Float &u, const Float &v) { return u - v * floor(u / v); }

inline Float fsin(Float x) {
	x = mod(x - PihF, Pi2F);
	x = fabs(x - PiF) - PihF;
	return fast_sin(x);
}
inline Float fcos(Float x) {
	x = mod(x, Pi2F);
	x = fabs(x - PiF) - PihF;
	return fast_sin(x);
}
