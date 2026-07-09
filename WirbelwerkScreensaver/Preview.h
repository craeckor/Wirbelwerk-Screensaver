#pragma once

struct Config; // forward declaration

// Implements the official "/p <hwnd>" screensaver launch argument: renders a live,
// lightweight preview of the simulation embedded as a child window inside the given
// parent window handle (the small thumbnail box in the Windows Display Settings /
// legacy Control Panel screen saver dialog). Blocks until the parent window goes
// away (the host dialog closes) or is otherwise no longer valid.
void RunEmbeddedPreview(void* parentHwndVoid);

// Opens a small, decorated, resizable window that runs a live preview of the current
// (in-memory) configuration. Used by the "Preview" button in the settings dialog.
// Unlike the real screensaver, mouse movement/clicks do NOT close this window - only
// Escape or the window's own close button do, since the user is expected to keep
// interacting with the settings dialog while it runs. Blocks the calling thread until
// the preview window is closed.
// If liveConfig is non-null, g_config will be synced from *liveConfig every frame so
// changes made in the settings dialog take effect immediately in the running preview.
void RunFloatingPreviewWindow(Config* liveConfig = nullptr);
