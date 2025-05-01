#pragma once

#include "window.h"
class Program {
  public:
	Program() {
		SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_GAMEPAD);
		IMGUI_CHECKVERSION();
		m_view = Window("View", 1280, 720, SDL_PIXELFORMAT_ARGB8888);
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
			auto [pixels, height, pitch] = m_view.get_surf();
			for (uint32_t y = 0; y < height; y++) {
				for (uint32_t x = 0; x < pitch; x += 4) {
					pixels[y * pitch + x] = 0;		 // b
					pixels[y * pitch + x + 1] = 255; // g
					pixels[y * pitch + x + 2] = 255; // r
					pixels[y * pitch + x + 3] = 255; // a
				}
			}
			m_view.set_surf();
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
		SliderFloat("Float", &data.tmp, 0, 100);
		End();
	};

	struct Data {
		float tmp = 0.f;
	} data;

	Window m_view;
	Window m_menu;
	SDL_Event event;
	bool m_running;
};