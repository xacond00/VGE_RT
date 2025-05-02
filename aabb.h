#pragma once
// Created by Ondrej Ac (xacond00)
#include "ray.h"
#include "vec.h"
struct AABB{
    AABB() : pmin(InfF), pmax(-InfF){}

    // Returns if hit was valid and intersection distance in t
    bool intersect(const Ray& r, Float &t)const{
        auto [mint, maxt] = bounds_check(r);
        if (mint < maxt && maxt > 0 && mint < t) {
			t = mint;
			return true;
		}
		return false;
    }

    // Returns that ray intersects
    bool ray_test(const Ray& r)const{
        auto [mint, maxt] = bounds_check(r);
		return mint < maxt && maxt > 0;
    }
    // Checks if edge was hit (for debugging)
    bool hit_edge(const Ray& r)const{
        auto [mint, maxt] = bounds_check(r);
        return mint < maxt && maxt > 0 && (maxt - mint) < EpsF;
    }

    void expand(const Vec3f& v){
        pmin = min(pmin, v);
        pmax = max(pmax, v);
    }

    void join(const AABB& other){
        pmin = min(pmin, other.pmin);
        pmax = max(pmax, other.pmax);
    }

    Vec3f center()const{
        return (pmin + pmax) * 0.5f;
    }

    Float area()const{
        Vec3f S = pmax - pmin;
		return dot(S, S.lrotate());
    }

    Vec3f pmin;
    Vec3f pmax;

    private:
    inline std::pair<Float, Float> bounds_check(const Ray& r) const {
		Vec3f t1 = (pmin - r.O) * r.iD;
		Vec3f t2 = (pmax - r.O) * r.iD;
		Vec3f tmin = min(t1, t2);
		Vec3f tmax = max(t1, t2);
        return {tmin.max(), tmax.min()};
	}
};