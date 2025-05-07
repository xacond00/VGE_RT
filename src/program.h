#pragma once
// Created by Ondrej Ac (xacond00)

#include "renderer.h"
#include "window.h"
class Program {
  public:
	// TODO: scene selector

	Program()
		: renderer(Scene("../sponza.obj", 0.01),
				   Camera(600, 600, 90, Transform(Vec3f(3, 5, -0.5), Vec3f(0, PihF, 0), 1)), Accel_t::BIH) {
		// Program() : renderer(Scene("../bunny.obj", 1.2f), Camera(600,600,90, Transform(Vec3f(0.059,0.235,0.232),
		// Vec3f(0,0.388,-0.292), 1)), Accel_t::BIH) { Program() : renderer(Scene("../armadillo.obj", 1.2f),
		// Camera(600,600,90, Transform(Vec3f(-1.117,1.7,-2.557), Vec3f(-0.148,3.475,-0.292), 1)), Accel_t::BIH) {
		printf("Polygons: %u\n", renderer.m_scene.poly_cnt());
		printf("Nodes:    %lu\n", renderer.m_acc->nodes_cnt());

		SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_GAMEPAD);
		IMGUI_CHECKVERSION();
		std::string view_title = std::string("View (") + renderer.m_acc->type_name() + ")";
		m_view = Window(view_title.c_str(), 600, 600, SDL_PIXELFORMAT_ARGB8888);
		m_menu = Window("Menu", 480, 720, m_imgui_menu);
	}
	~Program() {
		m_view.free();
		m_menu.free();
		SDL_Quit();
	}

	bool run() {
		m_running = true;
		while (m_running) {
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_EVENT_QUIT) {
					m_running = false;
					break;
				}
				m_view.scan_event(event);
				m_menu.scan_event(event);
			}
			if (m_view.shown()) {
				auto [pixels, height, pitch] = m_view.get_surf();
				m_iteration++;

				m_time = timer();
				renderer.set_output((Uint *)pixels, pitch);
				renderer.render();
				// renderer.m_acc->update();
				m_time = timer(m_time);
				m_view.set_surf();
			}
			m_running = m_view.valid() || m_menu.valid();
			m_view.render();
			m_menu.render();
			// SDL_Delay(10);
		}
		return true;
	}

  private:
	std::function<void(void)> m_imgui_menu = [this]() {
		using namespace ImGui;
		SetNextWindowPos({0, 0});
		SetNextWindowSize({(float)m_menu.width(), (float)m_menu.height()});
		Begin("Menu", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
		if (Button(renderer.m_pause ? "Unpause" : "Pause")) {
			renderer.m_pause = !renderer.m_pause;
		}
		auto &T = renderer.m_cam.T;
		if (SliderFloat3("Cam P", T.P.ptr(), -10, 10, "%.3f")) {
			renderer.m_reset = true;
		}
		if (SliderFloat3("Cam A", T.A.ptr(), -Pi2F, Pi2F, "%.3f")) {
			renderer.m_reset = true;
			T.update_Tr();
		}

		Text("Polygons:     %u", renderer.m_scene.poly_cnt());
		Text("Accel. nodes: %lu", renderer.m_acc->nodes_cnt());

		// pass times
		if (m_iteration > 0) {
			m_time_avg = (m_time_avg * (m_iteration - 1) + m_time) / m_iteration;
			m_time_min = std::min(m_time_min, m_time);
			m_time_max = std::max(m_time_max, m_time);
			Text("time: min       average    max");
			Text("      %.3f   %.3f    %.3f   [ms]", m_time_min * 1000, m_time_avg * 1000, m_time_max * 1000);
		}

		End();
	};

	struct Data {
		float tmp = 0.f;
	} data;
	Renderer renderer;
	Window m_view;
	Window m_menu;
	SDL_Event event;
	bool m_running;
	double m_time = 0;
	size_t m_iteration = 0;
	double m_time_avg = 0;
	double m_time_min = InfF;
	double m_time_max = 0;
};