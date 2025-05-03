#pragma once
// Created by Ondrej Ac (xacond00)
#include "acc_bbox.h"
#include "acc_bvh.h"
#include "acc_none.h"
#include "accel.h"
#include "camera.h"
#include "scene.h"
#include "rng.h"
#include <future>

struct OutputFmt {
	Uint *data;
	Uint pitch;
};

class Renderer {
  public:
	// TODO: Implement each pointer cast
	// Dynamically dispatches the correct acceleration structure.
	// To avoid virtual call on each ray intersection.
	Renderer() {}
	Renderer(Scene &&scn, Camera &&cam, Accel_t acc_t) : m_scene(std::move(scn)), m_cam(std::move(cam)) {
		set_accelerator(acc_t);
	}
	void render() {
		if (auto acc = dynamic_cast<AccelNone *>(m_acc)) {
			render_internal(*acc);
		} else if (auto acc = dynamic_cast<AccelBbox *>(m_acc)) {
			render_internal(*acc);
		} else if (auto acc = dynamic_cast<AccelBvh *>(m_acc)) {
			render_internal(*acc);
		} else if (auto acc = dynamic_cast<AccelNone *>(m_acc)) {
			render_internal(*acc);
		} else {
			std::cout << "Invalid acceleration structure !";
		}
	}

	template <class Acc>
	void render_internal(const Acc &acc) {
		auto &film = m_cam.film;
		auto dims = m_cam.film_size();
		if (m_reset)
			film.reset();

		const unsigned num_threads = std::thread::hardware_concurrency();
		std::vector<std::thread> threads;

		auto render_chunk = [&](Uint start_row, Uint end_row) { 
            RNG rng = RNG(std::rand() + 1);
			for (Uint i = start_row; i < end_row; ++i) {
				for (Uint j = 0; j < dims[0]; ++j) {
					Vec2u xy0(j, i);
					Vec2f xy(j + rng.rafl(), i + rng.rafl());
					Ray r = m_cam.sample_ray(xy);
					Vec3f col = sample(acc, rng, r);
					film.put(xy0, col); 
					if (out.data) {
						out.data[i * dims[0] + j] = vec2bgr(film.read(xy0)); 
					}
				}
			}
		};

		Uint rows_per_thread = dims[1] / num_threads;
		Uint remaining_rows = dims[1] % num_threads;

		Uint start_row = 0;
		for (Uint t = 0; t < num_threads; ++t) {
			Uint end_row = start_row + rows_per_thread + (t < remaining_rows ? 1 : 0);
			threads.emplace_back(render_chunk, start_row, end_row);
			start_row = end_row;
		}

		for (auto &t : threads) {
			t.join();
		}

		m_reset = false;
	}
	void set_output(Uint *data, Uint pitch) { out = {data, pitch}; }
	void set_accelerator(Accel_t type) {
		if (type < Accel_t::LAST && m_acc) {
			delete m_acc;
			m_acc = nullptr;
		}
		switch (type) {
			case Accel_t::None:
				m_acc = new AccelNone(m_scene);
				break;
			case Accel_t::Bbox:
				m_acc = new AccelBbox(m_scene);
				break;
			case Accel_t::BVH:
				m_acc = new AccelBvh(m_scene);
				break;
			// case None:m_acc = new AccelNone(m_scene);
			default:
				break;
		};
	}

	template <class Acc>
	Vec3f sample(const Acc &acc, RNG &rng, Ray r) const {
		HitInfo rec;
		Vec3f result(0);
		Vec3f weight(1);
		Uint depth = m_depth;
		while (depth > 0) {
			bool hit = acc.intersect(r, rec);
			if (!hit) { // Sky
				//return Vec3f(0.2, 0.2, 0.2);
				float val = std::pow(max(r.D.shrink(), Vec2f(0)).len2(), 16);
				result = result + weight * lerp(Vec3f(0.5, 0.8, 1.0), 5.f * Vec3f(1, 0.7, 0.2), Vec3f(val));
				break;
			} else {
				SurfaceInfo si = m_scene.surface_info(rec);
				//return Vec3f(dot(si.N, -r.D));
				r.D = si.frame.world(rng.sample_cos_distribution());
                r.iD = rcp(r.D);
				r.O = si.P + si.N * EpsF;
				// si.N; // Replace with random reflection
				//  emission would go here ... result += weight * emiss
				weight = weight * 0.7f;
				rec.idx = -1;
				rec.tuv.x() = InfF;
			}
			depth--;
		}
		return result;
	}
	Scene m_scene;
	Camera m_cam;
	Accel *m_acc = nullptr;
	OutputFmt out;
	Uint m_depth = 5;
	Uint m_spp = 1;
	bool m_reset = true;
};