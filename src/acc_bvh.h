#pragma once
// Created by Ondrej Ac (xacond00)
#include "accel.h"
// BVH tree
// Uses binned BVH building and fast updates
class AccelBvh : public Accel {

	// 32B BVH node
	struct Node {
		Node() {}
		Node(AABB &&bbox, Vec2u &&rng) : bbox(std::move(bbox)), rng(std::move(rng)) {}
		Node(const AABB &bbox, const Vec2u &rng) : bbox((bbox)), rng((rng)) {}
		AABB bbox;
		// When left < right, it points to triangle indices
		// When left > right, it points to next nodes
		// When left == right, it is empty
		bool leaf() const { return rng[0] < rng[1]; }
		bool parent() const { return rng[0] > rng[1]; }
		bool empty() const { return rng[0] == rng[1]; }
		Vec2u rng;
	};

  public:
	AccelBvh(const Scene &scene, Uint node_size = 4) : Accel(scene, Accel_t::BVH), m_node_size(node_size) {
		build();
	}

	// virtual ~AccelBvh() {}

	bool intersect(const Ray &r, HitInfo &rec) const override {
#if 0
		constexpr Uint stack_size = 64;
		struct Stack {
			Uint idx;
			Float t;
		};
		Stack stack[stack_size];
		Uint sptr = 0;

		stack[sptr++] = {0, rec.t()}; //, rec.t()};
		if (!m_bvh[0].bbox.ray_test(r, stack[0].t))
			return false;
		while (sptr) {
			Stack st = stack[--sptr];
			if (st.t > rec.t())
				continue;
			const Node &node = m_bvh[st.idx];
			if (node.parent()) {
				float t1 = rec.t(), t2 = rec.t();
				bool h1 = m_bvh[node.rng[1]].bbox.ray_dist(r, t1);
				bool h2 = m_bvh[node.rng[0]].bbox.ray_dist(r, t2);
				if (h1 && h2) {
					bool lt = t1 > t2;
					if (lt) {
						std::swap(t1, t2);
					}
					stack[sptr++] = {node.rng[lt], t2};
					stack[sptr++] = {node.rng[!lt], t1};
				} else if (h1)
					stack[sptr++] = {node.rng[1], t1};
				else if (h2)
					stack[sptr++] = {node.rng[0], t2};
			} else {
				for (uint i = node.rng[0]; i < node.rng[1]; i++) {
					poly(i).intersect(r, rec);
				}
			}
		}
#else
		constexpr Uint stack_size = 128;
		Uint stack[stack_size];
		Uint sptr = 0;
		stack[sptr++] = 0;
		while (sptr) {
			const Node &node = m_bvh[stack[--sptr]];
			if (node.bbox.ray_test(r, rec.t())) {
				if (node.parent()) {
					stack[sptr++] = node.rng[0];
					stack[sptr++] = node.rng[1];
				} else {
					for (uint i = node.rng[0]; i < node.rng[1]; i++) {
						poly(i).intersect(r, rec);
					}
				}
			}
		}

#endif
		return rec.idx != -1;
	}

	bool ray_test(const Ray &r, Float t = InfF) const override {
		constexpr Uint stack_size = 1024;
		Uint stack[stack_size];
		Uint sptr = 0;
		stack[sptr++] = 0;
		while (sptr) {
			const Node &node = m_bvh[stack[--sptr]];
			if (node.bbox.ray_test(r, t)) {
				if (node.parent()) {
					stack[sptr++] = node.rng[0];
					stack[sptr++] = node.rng[1];
				} else {
					for (uint i = node.rng[0]; i < node.rng[1]; i++) {
						if (poly(i).ray_test(r, t)) {
							return true;
						}
					}
				}
			}
		}
		return false;
	}

	// Partially sorts poly indices in rng (beg, end)
	// Returns split index where they meet
	Uint sort_poly(const Vec2u &rng, Uint axis, Float plane) {
		Int i = rng[0];
		Int j = rng[1] - 1;
		while (i <= j) {
			if (vert(i).bbox().center()[axis] < plane)
				i++;
			else
				std::swap(m_poly[i], m_poly[j--]);
		}
		return i;
	}
	// Split polygons according to binned SAH metric
	// Returns cost and split index
	std::pair<Float, Uint> split_poly(const Vec2u &rng, const AABB &bbox);

	// Split bvh node
	// Supposes the node already exists
	void split_bvh(Uint node, Float &build_cost);

	void update_bvh() {
		double t1 = timer();
		float cost = 0;
		for (int i = m_bvh.size() - 1; i >= 0; i--) {
			auto &node = m_bvh[i];
			if (node.leaf()) {
				node.bbox = bbox_in(node.rng);
				cost += node.bbox.area() * (node.rng[1] - node.rng[0]);
			} else {
				node.bbox = m_bvh[node.rng[0]].bbox + m_bvh[node.rng[1]].bbox;
			}
		}
		// print("Cost", cost, m_build_cost);
		if (cost > 1.2 * m_build_cost)
			build();
		else
			m_update_cost = cost;
	}

	void update() override {
		update_bvh();
		m_built = true;
	};

	void build() override {
		double build_timer = timer();

		m_bvh.clear();
		m_bvh.reserve(1024);
		m_bvh.emplace_back(m_scene.m_bbox, Vec2u(0, m_poly.size()));
		Float cost = 0;
		split_bvh(0, cost);
		m_build_cost = cost;
		m_update_cost = cost;
		m_built = true;

		m_build_time = timer(build_timer);
	}

	// Getters
	size_t nodes_cnt()  const override { return m_bvh.size(); }
	double build_time() const override { return m_build_time; }

	std::vector<Node> m_bvh;
	Uint m_node_size = 4;
	Float m_update_cost = 0;
	Float m_build_cost = 0;
	double m_build_time = 0;
	// This points to the actual indices of triangles
	// Need to use m_scene
};