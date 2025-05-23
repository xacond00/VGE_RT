#pragma once
// Created by Ondrej Ac (xacond00)
#include "aabb.h"
#include "ray.h"
#include "vec.h"
#include "transform.h"

struct Vert3{
	Vert3(){}
	Vert3(const Vec3f& A, const Vec3f& B, const Vec3f& C) : data{A,B,C}{}
    Vec3f data[3];
	AABB bbox() const {
		return AABB(data, 3).padded();
	}
	Vec3f center() const { return  (data[0] + data[1] + data[2]) * Float(1.0 / 3.0); }

};

struct Poly {
	// 3 points and index
	Poly(Vec3f a, Vec3f b, Vec3f c, Uint idx) : Q(a), U(b - a), V(c - a), N(cross(U, V)), idx(idx) {}
	Poly(const Vert3& v, Uint idx = -1) : Poly(v.data[0], v.data[1], v.data[2], idx){}

	bool intersect(const Ray &r, HitInfo &rec) {
		auto si = bounds_check(r);
		if (within(si.u(), 0.f, 1.f) && within(si.v(), 0.f, 1.f - si.u()) && inside(si.t(), Eps6F, rec.t())) {
			rec = si;
			return true;
		}
		return false;
	}

	bool ray_test(const Ray &r, Float t = InfF) const {
        auto si = bounds_check(r);
		return within(si.u(), 0.f, 1.f) && within(si.v(), 0.f, 1.f - si.u()) && inside(si.t(), Eps6F, t);
    }

	SurfaceInfo surface_info(const HitInfo &rec) const {
		SurfaceInfo si(rec);
		si.N = norm(si.face ? N : -N);
		si.P = Q + si.u() * U + si.v() * V;
		si.frame = ONB(si.N);
        return si;
	}
	Vec3f center() const { return Q + (U + V) * Float(1.0 / 3.0); }
	Float area() { return Float(0.5f) * N.len(); }
	AABB bbox() const {
		Vec3f pts[] = {Q, Q + U, Q + V};
		return AABB(pts, 3).padded();
	}
	// Origin, Vec U, Vec V, Surf Normal
	Vec3f Q, U, V, N;
	Uint idx;

  private:
  // Moller-Trumbore algorithm
	HitInfo bounds_check(const Ray &r)const {
		Vec3f pV = cross(r.D, V);
		float D = dot(U, pV);
		float iD = Float(1) / D;
		Vec3f tV = r.O - Q;
		Vec3f qV = cross(tV, U);
		float u = dot(tV, pV) * iD;
		float v = dot(r.D, qV) * iD;
		float t = dot(V, qV) * iD;
		return HitInfo(t, u, v, idx, D > 0);
	}
};
