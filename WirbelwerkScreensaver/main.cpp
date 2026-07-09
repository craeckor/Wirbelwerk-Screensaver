// ==================== WIRBELWERK SCREENSAVER — ENTRY POINT ====================
// Implements the standard Win32 screensaver command-line contract described in
// docs-1.md / docs-2.md / docs-3.txt:
//   (no args)   -> show the settings dialog (matches typical .scr behavior when
//                  double-clicked from Explorer without switches)
//   /s          -> run fullscreen across all monitors (or one instance per monitor
//                  when "run independently on every display" is enabled)
//   /c          -> show the configuration dialog
//   /c:<hwnd>   -> show the configuration dialog owned by the given window handle
//   /p <hwnd>   -> render an embedded live preview into the given window handle
//   /a          -> legacy password-prompt switch; no password model exists here, so
//                  it is accepted and simply falls through to normal fullscreen behavior
// Switches are also accepted with a leading '-' and are case-insensitive, which is
// tolerant of the same variations described in docs-3.txt.
#define GLFW_INCLUDE_NONE
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include "Config.h"
#include "Simulation.h"
#include "Monitors.h"
#include "Preview.h"
#include "ConfigDialog.h"

#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <cctype>
#include <cmath>

// ==================== ARGUMENT PARSING ====================
enum class LaunchMode { Config, FullScreen, Preview };

struct LaunchArgs {
	LaunchMode mode = LaunchMode::Config;
	HWND targetHwnd = nullptr; // used by /p <hwnd> and /c:<hwnd>
};

static bool ArgIs(const std::string& arg, const char* name) {
	if (arg.empty() || (arg[0] != '/' && arg[0] != '-')) return false;
	std::string body = arg.substr(1);
	std::string want = name;
	if (body.size() < want.size()) return false;
	std::string bodyLower = body.substr(0, want.size());
	std::transform(bodyLower.begin(), bodyLower.end(), bodyLower.begin(),
		[](unsigned char ch) { return (char)std::tolower(ch); });
	return bodyLower == want;
}

static HWND ParseHwndSuffix(const std::string& arg) {
	// Handles "/p:123456" and "/c:123456" forms; the space-separated "/p 123456" form
	// is handled by looking at the following argv entry instead.
	size_t colon = arg.find(':');
	if (colon == std::string::npos) return nullptr;
	long long v = std::atoll(arg.c_str() + colon + 1);
	return v != 0 ? (HWND)(intptr_t)v : nullptr;
}

static LaunchArgs ParseArgs(int argc, char** argv) {
	LaunchArgs result;
	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (ArgIs(arg, "s")) {
			result.mode = LaunchMode::FullScreen;
		}
		else if (ArgIs(arg, "a")) {
			// No legacy password model; treat like a plain run request.
			result.mode = LaunchMode::FullScreen;
		}
		else if (ArgIs(arg, "p")) {
			result.mode = LaunchMode::Preview;
			HWND h = ParseHwndSuffix(arg);
			if (!h && i + 1 < argc) {
				long long v = std::atoll(argv[i + 1]);
				if (v != 0) { h = (HWND)(intptr_t)v; ++i; }
			}
			result.targetHwnd = h;
		}
		else if (ArgIs(arg, "c")) {
			result.mode = LaunchMode::Config;
			HWND h = ParseHwndSuffix(arg);
			if (!h && i + 1 < argc) {
				long long v = std::atoll(argv[i + 1]);
				if (v != 0) { h = (HWND)(intptr_t)v; ++i; }
			}
			result.targetHwnd = h;
		}
	}
	return result;
}

// ==================== INPUT-TERMINATION TRACKING ====================
// Per docs-1.md, a real screensaver must exit on essentially any user input: mouse
// movement beyond a small noise threshold, any mouse button, any key, and scroll wheel
// activity. GLFW callbacks give us every event type without polling raw Win32 messages.
struct TerminationState {
	bool shouldClose = false;
	bool baselineSet = false;
	double baseX = 0.0, baseY = 0.0;
};

static TerminationState g_term;

static void RequestClose(GLFWwindow* window) {
	g_term.shouldClose = true;
	glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void OnKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
	(void)scancode; (void)mods;
	if (action == GLFW_PRESS) RequestClose(window);
}
static void OnChar(GLFWwindow* window, unsigned int codepoint) {
	(void)codepoint;
	RequestClose(window);
}
static void OnMouseButton(GLFWwindow* window, int button, int action, int mods) {
	(void)button; (void)mods;
	if (action == GLFW_PRESS) RequestClose(window);
}
static void OnScroll(GLFWwindow* window, double xoff, double yoff) {
	(void)xoff; (void)yoff;
	RequestClose(window);
}
static void OnCursorPos(GLFWwindow* window, double x, double y) {
	if (!g_term.baselineSet) {
		// The first callback after window creation often fires once with the initial
		// position; seed the baseline instead of treating it as movement.
		g_term.baseX = x; g_term.baseY = y;
		g_term.baselineSet = true;
		return;
	}
	// Small dead-zone avoids false positives from OS cursor jitter/DPI rounding.
	if (std::abs(x - g_term.baseX) > 3.0 || std::abs(y - g_term.baseY) > 3.0) {
		RequestClose(window);
	}
}
static void OnWindowFocus(GLFWwindow* window, int focused) {
	// Per docs-1.md's WM_ACTIVATE guidance: losing foreground focus (e.g. Ctrl+Alt+Del,
	// a UAC prompt, or another app stealing focus) should also end the screensaver.
	if (!focused) RequestClose(window);
}

static void InstallTerminationCallbacks(GLFWwindow* window) {
	glfwSetKeyCallback(window, OnKey);
	glfwSetCharCallback(window, OnChar);
	glfwSetMouseButtonCallback(window, OnMouseButton);
	glfwSetScrollCallback(window, OnScroll);
	glfwSetCursorPosCallback(window, OnCursorPos);
	glfwSetWindowFocusCallback(window, OnWindowFocus);
}

// ==================== FULLSCREEN RUNTIME ====================
struct DisplayInstance {
	MonitorViewport viewport;
	FluidSim sim;
};

// Runs one borderless window spanning the full virtual desktop, either as a single
// shared simulation (classic behavior) or as N independent FluidSim instances (one per
// monitor, each with its own splats/bursts/color state) when g_config.perMonitor is set.
static int RunFullScreen() {
	if (!glfwInit()) return -1;

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);

	int screenX = GetSystemMetrics(SM_XVIRTUALSCREEN);
	int screenY = GetSystemMetrics(SM_YVIRTUALSCREEN);
	int screenW = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int screenH = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	GLFWwindow* window = glfwCreateWindow(screenW, screenH, "Wirbelwerk", nullptr, nullptr);
	if (!window) { glfwTerminate(); return -1; }
	glfwSetWindowPos(window, screenX, screenY);

	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		glfwTerminate();
		return -1;
	}

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	InstallTerminationCallbacks(window);
	glfwShowWindow(window);
	glfwFocusWindow(window);

	InitSharedSimResources();

	std::vector<DisplayInstance> displays;
	if (g_config.perMonitor) {
		auto viewports = EnumerateMonitorViewports();
		if (viewports.empty()) {
			// Enumeration failure fallback: treat the whole virtual desktop as one display.
			DisplayInstance one;
			one.viewport = { 0, 0, screenW, screenH };
			displays.push_back(std::move(one));
		}
		else {
			for (auto& vp : viewports) {
				DisplayInstance d;
				d.viewport = vp;
				displays.push_back(std::move(d));
			}
		}
	}
	else {
		DisplayInstance one;
		one.viewport = { 0, 0, screenW, screenH };
		displays.push_back(std::move(one));
	}

	for (auto& d : displays) {
		d.sim.Init(d.viewport.w, d.viewport.h);
	}

	auto lastTime = std::chrono::steady_clock::now();

	while (!glfwWindowShouldClose(window) && !g_term.shouldClose) {
		glfwPollEvents();
		if (glfwWindowShouldClose(window) || g_term.shouldClose) break;

		auto now = std::chrono::steady_clock::now();
		float dt = std::chrono::duration<float>(now - lastTime).count();
		lastTime = now;
		dt = std::min(dt, 0.033f);

		for (auto& d : displays) {
			d.sim.Update(dt);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, screenW, screenH);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		for (auto& d : displays) {
			// GL viewport Y grows upward from the bottom-left, but our monitor rects are
			// top-left-origin/Y-down, so flip before handing off to FluidSim::Render.
			int vy = screenH - (d.viewport.y + d.viewport.h);
			d.sim.Render(d.viewport.x, vy, d.viewport.w, d.viewport.h);
		}
		glfwSwapBuffers(window);

		int targetMs = g_config.fpsLimit > 0 ? (1000 / g_config.fpsLimit) : 0;
		if (targetMs > 0) {
			auto frameEnd = std::chrono::steady_clock::now();
			auto frameDuration = std::chrono::duration<float, std::milli>(frameEnd - now).count();
			if (frameDuration < targetMs) {
				std::this_thread::sleep_for(std::chrono::milliseconds((int)(targetMs - frameDuration)));
			}
		}
	}

	// displays goes out of scope here → ~FluidSim() frees each instance's GPU resources
	DestroySharedSimResources();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

// ==================== MAIN ====================
int main(int argc, char** argv) {
	g_config.Load();

	LaunchArgs args = ParseArgs(argc, argv);
	HINSTANCE hInstance = GetModuleHandleW(nullptr);

	switch (args.mode) {
	case LaunchMode::Preview:
		if (args.targetHwnd && IsWindow(args.targetHwnd)) {
			RunEmbeddedPreview(args.targetHwnd);
		}
		return 0;

	case LaunchMode::Config:
		ShowConfigDialog(hInstance, args.targetHwnd);
		return 0;

	case LaunchMode::FullScreen:
	default:
		return RunFullScreen();
	}
}
