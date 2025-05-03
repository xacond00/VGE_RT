#include "acc_bvh.h"

std::pair<Float, Uint> AccelBvh::split_poly(const Vec2u &rng, const AABB &bbox) {
	struct bins {
		AABB box;
		Uint cnt = 0;
	};
	struct layer {
		Float area = 0;
		Uint cnt = 0;
	};

	Uint axis = bbox.longest_axis();
	Float fplane = bbox.center()[axis];
	Float fcost = 1e30f;
	constexpr Uint no_bins = 32;
	// For each axist
	for (Uint a = 0; a < 3; a++) {
		// Create n bins
		bins bin[no_bins];
		// Compute scale for indexing
		Float scale = no_bins / (bbox.pmax[a] - bbox.pmin[a]);

		// Fill bin bboxes with all polygons in rng
		for (Uint i = rng[0]; i < rng[1]; i++) {
			auto box = vert(i).bbox();
			Int id = (box.center()[a] - bbox.pmin[a]) * scale;
			id = std::max(std::min(id, Int(no_bins - 1)), 0);
			bin[id].cnt++;
			bin[id].box.expand(box);
		}

		layer L[no_bins - 1], R[no_bins - 1];
		bins lbin, rbin;

		// Test all possible bin combinations
		for (Int i = 0; i < no_bins - 1; i++) {
			lbin.box.expand(bin[i].box);
			rbin.box.expand(bin[no_bins - 1 - i].box);
			L[i].area = lbin.box.area();
			R[no_bins - 2 - i].area = rbin.box.area();
			L[i].cnt = lbin.cnt += bin[i].cnt;
			R[no_bins - 2 - i].cnt = rbin.cnt += bin[no_bins - 1 - i].cnt;
		}

		Float step = (bbox.pmax[a] - bbox.pmin[a]) / no_bins;
		Float plane = bbox.pmin[a];
		// Find the best split combination
		for (Uint i = 0; i < no_bins - 1; i++) {
			plane += step;
			Float cost = L[i].area * L[i].cnt + R[i].area * R[i].cnt;
			if (cost < fcost) {
				axis = a;
				fplane = plane;
				fcost = cost;
			}
		}
	}
	Uint split = sort_poly(rng, axis, fplane);
	return {fcost, split};
}
void AccelBvh::split_bvh(Uint node, Float &build_cost) {
	const AABB &bbox = m_bvh[node].bbox;
	Uint be = m_bvh[node].rng[0];
	Uint en = m_bvh[node].rng[1];
	Uint size = en - be;
	float pcost = bbox.area() * size;

	if (size > m_node_size) {
		auto [cost, mi] = split_poly(m_bvh[node].rng, bbox);
		if (mi > be && mi < en && cost < pcost) {
			m_bvh[node].rng = {m_bvh.size() + 1, m_bvh.size()};
			Vec2u left(be, mi);
			Vec2u right(mi, en);
			m_bvh.emplace_back(bbox_in(left), left);
			m_bvh.emplace_back(bbox_in(right), right);
			split_bvh(m_bvh[node].rng[0], build_cost);
			split_bvh(m_bvh[node].rng[1], build_cost);
		} else
			{
				build_cost += pcost;
				//println(cost,pcost, m_bvh[node].rng[1] - m_bvh[node].rng[0]);
			}
	} else
		build_cost += pcost;
}
