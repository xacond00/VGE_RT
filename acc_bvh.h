#pragma once
// Created by Ondrej Ac (xacond00)
#include "accel.h"
// BVH tree
// Uses binned BVH building and fast updates
class AccelBvh : public Accel {

	// 32B BVH node
	struct Node{
		AABB bbox;
		// When left < right, it points to triangle indices
		// When left > right, it points to next nodes
		// When left == right, it is empty
		Uint left, right;
	};

  public:
	AccelBvh(const Scene &scene) : Accel(scene, Accel_t::None) {
		Uint size = m_scene.poly_cnt();
		// This array is actually going to get modified
		// As opposed to the original vertices/indices
        m_poly_idx.reserve(size);
        for(Uint i = 0; i < size; i++){
            m_poly_idx.emplace_back(i);
        }
	}
	~AccelBvh() override {}
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
	// This points to the actual indices of triangles
	// Need to use m_scene
    std::vector<Uint> m_poly_idx;
};