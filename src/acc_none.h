#pragma once
// Created by Ondrej Ac (xacond00)
#include "accel.h"
// Zero acceleration except for scene bbox test
class AccelNone : public Accel {
  public:
	AccelNone(const Scene &scene) : Accel(scene, Accel_t::None) {
	}
	//virtual ~AccelNone() {}
	bool intersect(const Ray &r, HitInfo &rec) const override {
		bool hit = false;
        // If hit scene bbox
		if (m_scene.hit_bbox(r)) {
			// Test all polys in scene
			for (uint i = 0; i < m_scene.poly_cnt(); i++) {
				hit |= m_scene.intersect(i, r, rec);
			}
		}
		return hit;
	}
	bool ray_test(const Ray &r, Float t = InfF) const override {
		if (m_scene.hit_bbox(r)) {
			for (uint i = 0; i < m_scene.poly_cnt(); i++) {
				if (m_scene.ray_test(i, r, t))
					return true;
			}
		}
		return false;
	}
	void update() override { m_built = true; };
	void build() override { m_built = true; }

	size_t nodes_cnt()  const override { return m_scene.poly_cnt(); }
	double build_time() const override { return 0; }
};