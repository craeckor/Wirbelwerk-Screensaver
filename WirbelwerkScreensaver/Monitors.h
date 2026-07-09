#pragma once
#include <vector>

// A monitor's on-screen rectangle expressed in virtual-desktop-relative, top-left-origin,
// Y-down pixel coordinates (i.e. already translated so that (0,0) matches the top-left
// corner of the borderless window that spans SM_XVIRTUALSCREEN/SM_YVIRTUALSCREEN sized
// SM_CXVIRTUALSCREEN x SM_CYVIRTUALSCREEN). Callers rendering with OpenGL must flip the
// Y axis themselves (GL viewport Y grows upward from the bottom-left).
struct MonitorViewport {
	int x = 0, y = 0, w = 0, h = 0;
};

// Enumerates all currently active display monitors via EnumDisplayMonitors. Used to
// give each physical display its own independent fluid simulation instance when
// per-monitor mode is enabled. Returns an empty vector if enumeration fails for any
// reason; callers should fall back to a single full-virtual-desktop viewport.
std::vector<MonitorViewport> EnumerateMonitorViewports();
