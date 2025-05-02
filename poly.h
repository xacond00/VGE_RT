#pragma once
// Created by Ondrej Ac (xacond00)
#include "aabb.h"
#include "ray.h"
#include "vec.h"
struct Poly {
	// 3 points and index
	Poly(Vec3f a, Vec3f b, Vec3f c, Uint idx = -1) : Q(a), U(b - a), V(c - a), N(cross(U, V)) {}

	bool intersect(const Ray &r, HitInfo &hit) {
		auto si = bounds_check(r);
		if (within(si.u(), 0.f, 1.f) && within(si.v(), 0.f, 1.f - si.u()) && inside(si.t(), Eps6F, hit.t())) {
			hit = si;
			return true;
		}
		return false;
	}

	bool ray_test(const Ray &r, Float t = InfF) const {
        auto si = bounds_check(r);
		return within(si.u(), 0.f, 1.f) && within(si.v(), 0.f, 1.f - si.u()) && inside(si.t(), Eps6F, t);
    }

	SurfaceInfo surface_info(const HitInfo &hit) const {
		SurfaceInfo si(hit);
		si.N = si.face ? N : -N;
		si.P = Q + si.u() * U + si.v() * V;
        return si;
	}
	Vec3f center() const { return Q + (U + V) * Float(1.0 / 3.0); }
	Float area() { return Float(0.5f) * N.len(); }
	AABB bbox() const {
		std::array<Vec3f, 3u> pts{Q, Q + U, Q + V};
		AABB box(pts);
		return box.padded();
	}
	// Origin, Vec U, Vec V, Surf Normal
	Vec3f Q, U, V, N;
	Uint idx;

  private:
	HitInfo bounds_check(const Ray &r)const {
		Vec3f pV = cross(r.D, V);
		float D = dot(U, pV);
		float iD = 1.f / D;
		Vec3f tV = r.O - Q;
		Vec3f qV = cross(tV, U);
		float u = dot(tV, pV) * iD;
		float v = dot(r.D, qV) * iD;
		float t = dot(V, qV) * iD;
		return HitInfo(t, u, v, idx, D > 0);
	}
};