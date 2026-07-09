#pragma once

// Shows the native "Wirbelwerk Settings" modal dialog (IDD_CONFIGDIALOG). Loads the
// current on-disk config, lets the user edit every simulation/appearance/behavior
// setting, and - only if the user presses Save - persists changes via Config::Save()
// (which writes to %LOCALAPPDATA%\Wirbelwerk, never to the possibly admin-protected
// install folder). Returns true if the user saved changes, false if they cancelled.
//
// hInstance: the module instance owning the IDD_CONFIGDIALOG resource (the .exe/.scr itself).
// hwndParent: optional owner window (nullptr is fine when launched standalone via "/c").
bool ShowConfigDialog(void* hInstance, void* hwndParent);
