// Both entry points below assume g_config has already been populated (main() calls
// Config::Load() once at startup before dispatching to any mode) and that no other
// GLFW window/context is active in this process when they are called - each function
// owns the full GLFW lifecycle (glfwInit...glfwTerminate) for its own preview context.
#define GLFW_INCLUDE_NONE
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <glad/glad.h>
#include "Preview.h"
#include "Simulation.h"
#include "Config.h"
#include <chrono>
#include <thread>
#include <algorithm>

void RunEmbeddedPreview(void* parentHwndVoid) {
	HWND parent = (HWND)parentHwndVoid;
	if (!parent || !IsWindow(parent)) return;

	if (!glfwInit()) return;

	RECT rc{};
	GetClientRect(parent, &rc);
	int w = std::max<long>(1, rc.right - rc.left);
	int h = std::max<long>(1, rc.bottom - rc.top);

	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(w, h, "Wirbelwerk Preview", nullptr, nullptr);
	if (!window) { glfwTerminate(); return; }

	// Reparent the GLFW-owned HWND into the host's preview box. This is the standard
	// trick to give a GLFW window a Win32 parent, since GLFW itself has no public API
	// for creating a window that is a child of a foreign HWND from the start.
	HWND hwnd = glfwGetWin32Window(window);
	LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
	style = (style & ~(WS_POPUP | WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_THICKFRAME)) | WS_CHILD;
	SetWindowLongPtr(hwnd, GWL_STYLE, style);
	SetParent(hwnd, parent);
	SetWindowPos(hwnd, nullptr, 0, 0, w, h, SWP_NOZORDER | SWP_FRAMECHANGED);
	ShowWindow(hwnd, SW_SHOW);

	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		glfwDestroyWindow(window);
		glfwTerminate();
		return;
	}

	InitSharedSimResources();
	FluidSim sim;
	sim.Init(w, h);

	auto pollResize = [parent](int& outW, int& outH) -> bool {
		if (!IsWindow(parent)) return false;
		RECT r{};
		GetClientRect(parent, &r);
		outW = std::max<long>(1, r.right - r.left);
		outH = std::max<long>(1, r.bottom - r.top);
		return true;
	};

	auto lastTime = std::chrono::steady_clock::now();
	while (IsWindow(parent) && !glfwWindowShouldClose(window)) {
		int newW = w, newH = h;
		if (pollResize(newW, newH) && (newW != w || newH != h)) {
			w = newW; h = newH;
			SetWindowPos(hwnd, nullptr, 0, 0, w, h, SWP_NOZORDER);
			sim.Resize(w, h);
		}
		auto now = std::chrono::steady_clock::now();
		float dt = std::chrono::duration<float>(now - lastTime).count();
		lastTime = now;
		dt = std::min(dt, 0.033f);

		sim.Update(dt);
		sim.Render(0, 0, w, h);
		glfwSwapBuffers(window);
		glfwPollEvents();
		std::this_thread::sleep_for(std::chrono::milliseconds(15));
	}

	// sim goes out of scope here → ~FluidSim() frees GPU resources
	DestroySharedSimResources();
	glfwDestroyWindow(window);
	glfwTerminate();
}

void RunFloatingPreviewWindow(Config* liveConfig) {
	static bool s_previewActive = false;
	if (s_previewActive) return;
	s_previewActive = true;

	if (!glfwInit()) { s_previewActive = false; return; }

	glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	int w = 480, h = 300;
	GLFWwindow* window = glfwCreateWindow(w, h, "Wirbelwerk \xe2\x80\x94 Preview (Esc to close)", nullptr, nullptr);
	if (!window) { glfwTerminate(); s_previewActive = false; return; }

	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		glfwDestroyWindow(window);
		glfwTerminate();
		s_previewActive = false;
		return;
	}
	glfwSwapInterval(1);

	InitSharedSimResources();
	FluidSim sim;
	glfwGetFramebufferSize(window, &w, &h);
	sim.Init(w, h);

	auto lastTime = std::chrono::steady_clock::now();
	while (!glfwWindowShouldClose(window)) {
		// Live-sync: pull latest dialog edits into g_config so the simulation uses them.
		if (liveConfig) g_config = *liveConfig;

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GLFW_TRUE);

		int newW, newH;
		glfwGetFramebufferSize(window, &newW, &newH);
		if (newW != w || newH != h) {
			w = newW; h = newH;
			sim.Resize(w, h);
		}

		auto now = std::chrono::steady_clock::now();
		float dt = std::chrono::duration<float>(now - lastTime).count();
		lastTime = now;
		dt = std::min(dt, 0.033f);

		sim.Update(dt);
		sim.Render(0, 0, w, h);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// sim goes out of scope here → ~FluidSim() frees GPU resources
	DestroySharedSimResources();
	glfwDestroyWindow(window);
	glfwTerminate();
	s_previewActive = false;
}
