#pragma once
// Created by Ondrej Ac (xacond00)

#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include "renderer.h"
#include "window.h"

class Program {
  public:
	Program() : renderer() {
		fetch_scene_files("../");

		// init renderer without scene
		renderer.set_accelerator(m_curr_accel_type);
		renderer.m_cam = Camera(600, 600, 90, Transform(Vec3f(0, 0, 0), Vec3f(0, PihF, 0), 1));

		// load the default scene
		load_scene(m_selected_scene_idx);

		// SDL + ImGui init
		SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_GAMEPAD);
		IMGUI_CHECKVERSION();

		std::string view_title = std::string("View (") + renderer.m_acc->type_name() + ")";
		m_view = std::make_unique<Window>(view_title.c_str(), 600, 600, SDL_PIXELFORMAT_ARGB8888);
		m_menu = std::make_unique<Window>("Menu", 480, 720, m_imgui_menu);
	}

	~Program() {
		if (m_view)
			m_view->free();
		if (m_menu)
			m_menu->free();
		SDL_Quit();
	}

	bool run() {
		double start_t = timer();
		m_running = true;
		while (m_running) {
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_EVENT_QUIT) {
					m_running = false;
					break;
				}
				m_view->scan_event(event);
				m_menu->scan_event(event);
				if (event.type == SDL_EVENT_KEY_DOWN) {
					m_keys[event.key.scancode] = true;
				} else if (event.type == SDL_EVENT_KEY_UP) {
					m_keys[event.key.scancode] = false;
				}
			}
			if (m_view->keyboard_focus() &&
				(m_keys[SDL_SCANCODE_W] || m_keys[SDL_SCANCODE_S] || m_keys[SDL_SCANCODE_A] || m_keys[SDL_SCANCODE_D] ||
				 m_keys[SDL_SCANCODE_SPACE] || m_keys[SDL_SCANCODE_LCTRL])) {
				auto vel = renderer.m_cam.T.vec(Vec3f(m_keys[SDL_SCANCODE_D] - Float(m_keys[SDL_SCANCODE_A]), 0,
													  m_keys[SDL_SCANCODE_S] - Float(m_keys[SDL_SCANCODE_W]))) +
						   Vec3f(0, m_keys[SDL_SCANCODE_SPACE] - Float(m_keys[SDL_SCANCODE_LCTRL]), 0);
				renderer.m_cam.T.P += Vec3f(m_dt * m_speed) * vel;

				renderer.m_reset = true;
			}
			m_time = timer();
			if ((!renderer.m_pause || renderer.m_reset) && m_view->valid() && m_view->shown() && !m_view->minimized()) {
				auto [pixels, height, pitch] = m_view->get_surf();
				m_iteration++;
				renderer.set_output((Uint *)pixels, pitch);
				renderer.render();
				m_view->set_surf();
			}
			m_time = timer(m_time);
			m_running = m_view->valid() || m_menu->valid();
			m_view->render();
			m_menu->render();
		}
		m_dt = timer(start_t);
		return true;
	}

  private:
	void fetch_scene_files(const std::string &path, const std::string &ext = ".obj") {
		m_scene_paths.clear();
		m_scene_labels.clear();

		for (auto &p : std::filesystem::directory_iterator(path)) {
			if (p.path().extension() == ext) {
				m_scene_paths.push_back(p.path().string());
				m_scene_labels.push_back(p.path().filename().string());
			}
		}

		if (m_scene_paths.empty())
			throw std::runtime_error("No " + ext + " files found in " + path);
	}

	// swap in a new Scene and rebuild accel
	void load_scene(int scene_idx) {
		// set default camera and scale
		Float scale = 1.0f;
		if (m_scene_labels[scene_idx] == "armadillo.obj") {
			renderer.m_cam = Camera(600, 600, 80, Transform(Vec3f(-1.117, 1.7, -2.557), Vec3f(-0.148, 3.5, -0.292), 1));
			scale = 1.2f;
		} else if (m_scene_labels[scene_idx] == "bunny.obj") {
			renderer.m_cam = Camera(600, 600, 90, Transform(Vec3f(0.059, 0.235, 0.232), Vec3f(0, 0.388, -0.292), 1));
			scale = 1.2f;
		} else if (m_scene_labels[scene_idx] == "sponza.obj") {
			renderer.m_cam = Camera(600, 600, 90, Transform(Vec3f(3, 5, -0.5f), Vec3f(0, PihF, 0), 1));
			scale = 0.01f;
		} else if (m_scene_labels[scene_idx] == "dragon.obj") {
			renderer.m_cam = Camera(600, 600, 80, Transform(Vec3f(5.8, 0.2, -3), Vec3f(0, -4, 0), 1));
			scale = 0.05f;
		} /* else if {
			more configs...
		} */

		const auto &path = m_scene_paths[scene_idx];
		renderer.m_reset = true;
		renderer.m_scene = Scene(path, scale);
		// reinit accelerator to properly load the polys, etc... could be done better
		renderer.set_accelerator(renderer.m_acc->type());

		// reinit stats
		m_curr_accel_build_time = renderer.m_acc->build_time();
		reset_stats();
		m_curr_poly_cnt = renderer.m_scene.poly_cnt();
		m_curr_accel_nodes_cnt = renderer.m_acc->nodes_cnt();
	}

	// ImGui menu callback
	std::function<void(void)> m_imgui_menu = [this]() {
		using namespace ImGui;
		SetNextWindowPos({0, 0});
		SetNextWindowSize({(float)m_menu->width(), (float)m_menu->height()});
		Begin("Menu", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
		// Pause button
		if (Button(renderer.m_pause ? "Unpause" : "Pause")) {
			if (renderer.m_pause)
				reset_stats();
			renderer.m_pause = !renderer.m_pause;
		}
		// scene selector
		std::vector<const char *> scene_labels_cstr;
		scene_labels_cstr.reserve(m_scene_labels.size());
		for (const auto &label : m_scene_labels)
			scene_labels_cstr.push_back(label.c_str());
		if (Combo("Scene", &m_selected_scene_idx, scene_labels_cstr.data(), scene_labels_cstr.size()))
			load_scene(m_selected_scene_idx);

		// camera
		auto &T = renderer.m_cam.T;
		DragFloat("Movement speed", &m_speed, 1, 0, 10);
		if (SliderFloat3("Cam Pos", T.P.ptr(), -10, 10, "%.3f"))
			renderer.m_reset = true;
		if (SliderFloat3("Cam Ang", T.A.ptr(), -Pi2F, Pi2F, "%.3f")) {
			renderer.m_reset = true;
			T.update_Tr();
		}

		// scene stats
		Text("Polygons:     %u", m_curr_poly_cnt);
		Text("Accel. nodes: %lu", m_curr_accel_nodes_cnt);
		Text("Accel. build: %.6f [ms]", m_curr_accel_build_time * 1000);

		// timing stats
		if (m_iteration > 0) {
			m_time_avg += m_time; //(m_time_avg * (m_iteration - 1) + m_time) / m_iteration;
			m_time_min = std::min(m_time_min, m_time);
			m_time_max = std::max(m_time_max, m_time);
			Text("\nrender: min       avg       max [ms]");
			Text("        %.3f   %.3f   %.3f", m_time_min * 1000, 1000.0f / ImGui::GetIO().Framerate,
				 m_time_max * 1000);
		}

		End();
	};
	void reset_stats() {
		m_time_avg = 0.0;
		m_time_min = InfF;
		m_time_max = 0.0;
	}
	// members
	std::vector<std::string> m_scene_paths;
	std::vector<std::string> m_scene_labels;
	int m_selected_scene_idx = 0;
	Uint m_curr_poly_cnt = 0;
	Accel_t m_curr_accel_type = Accel_t::BVH; // TODO: accelerator selector
	double m_curr_accel_build_time = 0.0;
	size_t m_curr_accel_nodes_cnt = 0;

	Renderer renderer;
	std::unique_ptr<Window> m_view;
	std::unique_ptr<Window> m_menu;
	SDL_Event event;
	bool m_running = false;
	double m_time = 0.0;
	double m_dt = 0.1;
	float m_speed = 1;
	size_t m_iteration = 0;
	double m_time_avg = 0.0;
	double m_time_min = InfF;
	double m_time_max = 0.0;
	std::vector<bool> m_keys = std::vector<bool>(512, false);
};
