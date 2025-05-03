#pragma once
// Created by Ondrej Ac (xacond00)
#include "aabb.h"
#include "ray.h"
#include "scene.h"

enum Accel_t { None, Bbox, BVH, Octree, Blast };

/*
struct PolyPtr{
	Uint prim_idx;
	Uint mesh_idx;
};*/

// Acceleration structure base interface
class Accel {
  public:
	Accel(const Scene &scene, Accel_t type) : m_scene(scene), m_type(type), m_built(false) {
		if (m_type <= Accel_t::Bbox)
			return;
		// This array is actually going to get modified
		// As opposed to the original vertices/indices
		Uint size = m_scene.poly_cnt();
		m_poly.reserve(size);
		for (Uint i = 0; i < size; i++) {
			m_poly.emplace_back(i);
		}
	}
	//virtual ~Accel() {}
	virtual bool intersect(const Ray &r, HitInfo &rec) const{return false;} 
	virtual bool ray_test(const Ray &r, Float t = InfF) const{return false;}
	virtual void update(){}
	virtual void build(){}

	// Scene getters from poly indices
	Vert3 vert(Uint i) const { return m_scene.get_vert(m_poly[i]); }
	Poly poly(Uint i) const { return m_scene.get_poly(m_poly[i]); }
	Vec3f cent(Uint i) const { return m_scene.get_center(m_poly[i]); }
	
	// Compute bbox from the range of stored indices
	AABB bbox_in(Uint beg, Uint end) const { return bbox_in(Vec2u{beg, end}); }

	// Compute bbox from the range of stored indices
	AABB bbox_in(Vec2u range) const {
		Vec3f pmin(InfF);
		Vec3f pmax(-InfF);
		for (Uint i = range[0]; i < range[1]; i++) {
			auto V = vert(i);
			for (Uint i = 0; i < 3; i++) {
				pmin = min(pmin, V.data[i]);
				pmax = max(pmax, V.data[i]);
			}
		}
		return AABB(pmin, pmax);
	}

	// Getters
	bool built() { return m_built; }
	Accel_t type() const { return m_type; }

  protected:
	const Scene &m_scene;
	std::vector<Uint> m_poly;
	const Accel_t m_type;
	bool m_built;
};