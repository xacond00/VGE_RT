#pragma once
// Created by Ondrej Ac (xacond00)
#include "aabb.h"
#include "ray.h"
#include "vec.h"

struct Mesh {
	bool hit_bbox(const Ray &r, Float t = InfF) const { return m_bbox.ray_test(r, t); }
	bool intersect(const Ray &r, Float t = InfF) const { return m_bbox.ray_test(r, t); }
	Uint beg() const { return m_off; }
	Uint end() const { return m_off + m_cnt; }
	Vec2u range() const { return {beg(), end()}; }
	std::string m_name;
	Uint m_off = 0;
	Uint m_cnt = 0;
	AABB m_bbox;
};