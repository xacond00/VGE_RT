#pragma once
// Created by Ondrej Ac (xacond00)
#include "accel.h"
// Per mesh bbox test
class AccelBbox : public Accel {
  public:
	AccelBbox(const Scene &scene) : Accel(scene, Accel_t::Bbox) {}
	//virtual ~AccelBbox()override{}
	bool intersect(const Ray &r, HitInfo &rec) const override {
		bool hit = false;
        // If hit scene bbox
		if (m_scene.hit_bbox(r)) {
            // Loop over all meshes
			for (const auto &mesh : m_scene.m_mesh) {
                // If hit mesh bbox
				if (mesh.hit_bbox(r, rec.t())) {
                    // Test all polys in mesh
					for (Uint i = mesh.beg(); i < mesh.end(); i++)
						hit |= m_scene.intersect(i, r, rec);
				}
			}
		}
		return hit;
	}
	bool ray_test(const Ray &r, Float t = InfF) const override {
		if (m_scene.hit_bbox(r)) {
			for (const auto &mesh : m_scene.m_mesh) {
				if (mesh.hit_bbox(r, t)) {
					for (Uint i = mesh.beg(); i < mesh.end(); i++) {
						if (m_scene.ray_test(i, r, t))
							return true;
					}
				}
			}
		}
		return false;
	}
	void update() override { m_built = true; };
	void build() override { m_built = true; }

	size_t nodes_cnt()  const override { return m_scene.m_mesh.size(); }
	double build_time() const override { return 0; }
};