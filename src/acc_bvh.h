#pragma once
// Created by Ondrej Ac (xacond00)
#include "accel.h"
// BVH tree
// Uses binned BVH building and fast updates
class AccelBvh : public Accel {

	// 32B BVH node
	struct Node {
		Node(){}
		Node(AABB &&bbox, Vec2u && range) : bbox(std::move(bbox)), range(std::move(range)){}
		Node(const AABB &bbox, const Vec2u & range) : bbox((bbox)), range((range)){}
		AABB bbox;
		// When left < right, it points to triangle indices
		// When left > right, it points to next nodes
		// When left == right, it is empty
		Vec2u range;
	};

  public:
	AccelBvh(const Scene &scene, Uint node_size = 4) : Accel(scene, Accel_t::BVH), m_node_size(node_size) {}

	void bvh_builder() {}

	~AccelBvh() override {}
	bool intersect(const Ray &r, HitInfo &rec) const override {
		bool hit = false;
		// If hit scene bbox
		if (m_scene.hit_bbox(r)) {
			// Test all polys in scene
			for (Uint i = 0; i < m_scene.poly_cnt(); i++) {
				hit |= m_scene.intersect(i, r, rec);
			}
		}
		return hit;
	}
	bool ray_test(const Ray &r, Float t = InfF) const override {
		if (m_scene.hit_bbox(r)) {
			for (Uint i = 0; i < m_scene.poly_cnt(); i++) {
				if (m_scene.ray_test(i, r, t))
					return true;
			}
		}
		return false;
	}
	// Partially sorts poly indices in range (beg, end)
	// Returns split index where they meet
	Uint sort_poly(const Vec2u& range, Uint axis, Float plane) {
		Int i = range[0];
		Int j = range[1] - 1;
		while (i <= j) {
			if (cent(i)[axis] < plane)
				i++;
			else
				std::swap(m_poly_idx[i], m_poly_idx[j--]);
		}
		return i;
	}
	// Split polygons according to binned SAH metric
	// Returns cost and split index
	std::pair<Float, Uint> split_poly(const Vec2u &range, const AABB &bbox);

	// Split bvh node
	// Supposes the node already exists
	void split_bvh(Uint node, Float &build_cost);

	void update() override { m_built = true; };

	void build() override { 
		m_bvh.reserve(1024);
		m_bvh.emplace_back(m_scene.m_bbox, Vec2u(0, poly_cnt()));
		Float cost = 0;
		split_bvh(0, cost);
		m_build_cost = cost;
		m_built = true; 
	}

	std::vector<Node> m_bvh;
	Uint m_node_size = 4;
	Float m_build_cost = 0;
	// This points to the actual indices of triangles
	// Need to use m_scene
};