#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include "Monitors.h"

static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC, LPRECT lprcMonitor, LPARAM lParam) {
	auto* out = reinterpret_cast<std::vector<MonitorViewport>*>(lParam);
	MonitorViewport v;
	v.x = lprcMonitor->left;
	v.y = lprcMonitor->top;
	v.w = lprcMonitor->right - lprcMonitor->left;
	v.h = lprcMonitor->bottom - lprcMonitor->top;
	out->push_back(v);
	return TRUE;
}

std::vector<MonitorViewport> EnumerateMonitorViewports() {
	std::vector<MonitorViewport> result;
	EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, reinterpret_cast<LPARAM>(&result));

	// Normalize to virtual-desktop-relative coordinates (top-left origin at the
	// virtual screen's own origin, which can be negative if a monitor is left/above
	// the primary display).
	int originX = GetSystemMetrics(SM_XVIRTUALSCREEN);
	int originY = GetSystemMetrics(SM_YVIRTUALSCREEN);
	for (auto& v : result) {
		v.x -= originX;
		v.y -= originY;
	}
	return result;
}
