#pragma once
// Created by Ondrej Ac (xacond00)
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <functional>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
// #include <sstream>
#include <string>

class Window {
  public:
	Window() {}

	// Creates window
	Window(const char *title, uint32_t width, uint32_t height, SDL_PixelFormat format = SDL_PixelFormat(0))
		: Window(title, width, height, format, nullptr) {}

	Window(const char *title, uint32_t width, uint32_t height, const std::function<void()> &imgui_menu)
		: Window(title, width, height, SDL_PixelFormat(0), imgui_menu) {}

	Window(const char *title, uint32_t width, uint32_t height, SDL_PixelFormat format,
		   const std::function<void()> &imgui_menu);

	// Returns true if window was resized !
	bool scan_event(SDL_Event &e);

	void render();

	void free() {
		if (imgui()) {
			ImGui::SetCurrentContext(m_imgui_ctx);
			ImGui_ImplSDLRenderer3_Shutdown();
			ImGui_ImplSDL3_Shutdown();
			ImGui::DestroyContext();
		}
		if (m_surf) {
			SDL_DestroyTexture(m_surf);
			m_surf = nullptr;
		}
		if (m_renderer) {
			SDL_DestroyRenderer(m_renderer);
			m_renderer = nullptr;
		}
		if (m_window) {
			SDL_DestroyWindow(m_window);
			m_window = nullptr;
		}
		m_closed = true;
	}

	// Window dimensions
	uint32_t width() const { return m_width; }
	uint32_t height() const { return m_height; }
	std::pair<uint32_t, uint32_t> dims() const { return {m_width, m_height}; }

	std::tuple<unsigned char *, unsigned, unsigned> get_surf() {
		unsigned char *pixels = nullptr;
		int pitch = 0;
		if (!m_locked_surf && m_surf) {
			SDL_LockTexture(m_surf, nullptr, (void **)&pixels, &pitch);
			m_locked_surf = true;
		}
		return {pixels, unsigned(pitch), height()};
	}

	void set_surf() {
		if (m_locked_surf && m_surf) {
			SDL_UnlockTexture(m_surf);
			m_locked_surf = false;
		}
	}

	void close() {
		SDL_HideWindow(m_window);
		m_closed = true;
	}

	bool resize(uint32_t width, uint32_t height) {
		if (height && width && height != m_height && width != m_width) {
			m_width = width;
			m_height = height;
			if (m_surf) {
				SDL_DestroyTexture(m_surf);
				m_surf = SDL_CreateTexture(m_renderer, m_format, SDL_TEXTUREACCESS_STREAMING, width, height);
			}
			return true;
		}
		return false;
	}

	bool mouse_focus() const { return m_mouse_focus; }
	bool keyboard_focus() const { return m_keyboard_focus; }
	bool minimized() const { return m_minimized; }
	bool shown() const { return m_shown; }
	bool valid() const { return !m_closed && m_window && m_renderer; }
	void present() { SDL_RenderPresent(m_renderer); }
	bool imgui() const { return m_imgui_ctx; }

  private:
	// Window data
	std::string m_title = "";
	SDL_Window *m_window = nullptr;
	SDL_Renderer *m_renderer = nullptr;
	SDL_Texture *m_surf = nullptr;
	ImGuiContext *m_imgui_ctx = nullptr;
	std::function<void()> m_imgui_menu;
	SDL_PixelFormat m_format = SDL_PixelFormat(0);
	uint32_t m_windowID = -1;
	uint32_t m_width = 0;
	uint32_t m_height = 0;

	bool m_mouse_focus = false;
	bool m_keyboard_focus = false;
	bool m_fullscreen = false;
	bool m_minimized = false;
	bool m_shown = false;
	bool m_locked_surf = false;
	bool m_closed = false;
};
