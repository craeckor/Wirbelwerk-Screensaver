#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <string>
#include <algorithm>

#include "ConfigDialog.h"
#include "resource.h"
#include "Config.h"
#include "Preview.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")

// ==================== TRANSLATION ====================
struct Tr {
	struct Entry {
		const char* en;
		const char* de;
	};
	static const char* Get(const Entry& e, Language lang) {
		return lang == Language::German ? e.de : e.en;
	}

	static Entry DialogTitle;
	static Entry LanguageLabel;
	static Entry LangEnglish;
	static Entry LangGerman;

	static Entry GroupColor;
	static Entry Rainbow;
	static Entry SingleColor;
	static Entry ColorRange;
	static Entry SingleColorLabel;
	static Entry RangeStartLabel;
	static Entry RangeEndLabel;
	static Entry Pick;
	static Entry SmoothSpawn;

	static Entry GroupFlow;
	static Entry VortexStrength;
	static Entry VelocityDecay;
	static Entry ColorDecay;
	static Entry Pressure;
	static Entry PressureIter;

	static Entry GroupBrush;
	static Entry SplatSize;
	static Entry SplatForce;
	static Entry DyeAmount;

	static Entry GroupQuality;
	static Entry Simulation;
	static Entry DyeRes;
	static Entry Shading;

	static Entry GroupBehavior;
	static Entry AutoBursts;
	static Entry FpsLimit;
	static Entry Uncapped;
	static Entry PerMonitor;

	static Entry BtnPreview;
	static Entry BtnDefaults;
	static Entry BtnSave;
	static Entry BtnCancel;
	static Entry ConfigPath;

	// Info texts (longer descriptions)
	static Entry InfoColorMode;
	static Entry InfoSmoothSpawn;
	static Entry InfoCurl;
	static Entry InfoVelDiss;
	static Entry InfoDenDiss;
	static Entry InfoPressure;
	static Entry InfoPressIter;
	static Entry InfoSplatRadius;
	static Entry InfoSplatForce;
	static Entry InfoDyeAmount;
	static Entry InfoSimRes;
	static Entry InfoDyeRes;
	static Entry InfoShading;
	static Entry InfoAutoBurst;
	static Entry InfoFpsLimit;
	static Entry InfoPerMonitor;
};

#define TR(e) Tr::Get(e, g_config.language)

// ==================== TRANSLATION DATA ====================
Tr::Entry Tr::DialogTitle        = { "Wirbelwerk Settings", "Wirbelwerk Einstellungen" };
Tr::Entry Tr::LanguageLabel      = { "Language:", "Sprache:" };
Tr::Entry Tr::LangEnglish        = { "English", "Englisch" };
Tr::Entry Tr::LangGerman         = { "German", "Deutsch" };

Tr::Entry Tr::GroupColor         = { "Color", "Farbe" };
Tr::Entry Tr::Rainbow            = { "Rainbow", "Regenbogen" };
Tr::Entry Tr::SingleColor        = { "Single color", "Einzelfarbe" };
Tr::Entry Tr::ColorRange         = { "Color range", "Farbverlauf" };
Tr::Entry Tr::SingleColorLabel   = { "Single color:", "Einzelfarbe:" };
Tr::Entry Tr::RangeStartLabel    = { "Range start:", "Startfarbe:" };
Tr::Entry Tr::RangeEndLabel      = { "Range end:", "Endfarbe:" };
Tr::Entry Tr::Pick               = { "Pick...", "W\xE4hlen..." };
Tr::Entry Tr::SmoothSpawn        = { "Smooth spawn (fade new splats in)", "Sanftes Einblenden" };

Tr::Entry Tr::GroupFlow          = { "Flow", "Str\xF6mung" };
Tr::Entry Tr::VortexStrength     = { "Vortex strength", "Wirbelst\xE4rke" };
Tr::Entry Tr::VelocityDecay      = { "Velocity decay", "Geschwindigkeitsabfall" };
Tr::Entry Tr::ColorDecay         = { "Color decay", "Farbabfall" };
Tr::Entry Tr::Pressure           = { "Pressure", "Druck" };
Tr::Entry Tr::PressureIter       = { "Pressure iterations", "Druck-Iterationen" };

Tr::Entry Tr::GroupBrush         = { "Brush", "Pinsel" };
Tr::Entry Tr::SplatSize          = { "Splat size", "Spritzergr\xF6""\xDF""e" };
Tr::Entry Tr::SplatForce         = { "Splat force", "Spritzerkraft" };
Tr::Entry Tr::DyeAmount          = { "Dye amount", "Farbmenge" };

Tr::Entry Tr::GroupQuality       = { "Quality", "Qualit\xE4t" };
Tr::Entry Tr::Simulation         = { "Simulation:", "Simulation:" };
Tr::Entry Tr::DyeRes             = { "Dye res.:", "Farbaufl\xF6sung:" };
Tr::Entry Tr::Shading            = { "3D shading (lighting)", "3D-Schattierung" };

Tr::Entry Tr::GroupBehavior      = { "Behavior", "Verhalten" };
Tr::Entry Tr::AutoBursts         = { "Automatic bursts", "Automatische Spritzer" };
Tr::Entry Tr::FpsLimit           = { "FPS limit", "FPS-Grenze" };
Tr::Entry Tr::Uncapped           = { "Uncapped", "Unbegrenzt" };
Tr::Entry Tr::PerMonitor         = { "Run independently on every display", "Unabh\xE4ngig pro Bildschirm" };

Tr::Entry Tr::BtnPreview         = { "Preview", "Vorschau" };
Tr::Entry Tr::BtnDefaults        = { "Defaults", "Standard" };
Tr::Entry Tr::BtnSave            = { "Save", "Speichern" };
Tr::Entry Tr::BtnCancel          = { "Cancel", "Abbrechen" };

Tr::Entry Tr::ConfigPath         = { "Config: ", "Konfiguration: " };

Tr::Entry Tr::InfoColorMode      = { "Color mode:\n\nRainbow: Each splat gets a random hue.\nSingle: All splats use the same fixed color.\nRange: Each splat picks a random color between two endpoints.",
                                     "Farbmodus:\n\nRegenbogen: Jeder Spritzer erh\xE4lt einen zuf\xE4lligen Farbton.\nEinzelfarbe: Alle Spritzer verwenden dieselbe Farbe.\nFarbverlauf: Jeder Spritzer w\xE4hlt eine Zufallsfarbe zwischen zwei Endpunkten." };
Tr::Entry Tr::InfoSmoothSpawn    = { "Smooth spawn: When enabled, new splats fade in gradually over the duration set by the slider instead of appearing instantly.",
                                     "Sanftes Einblenden: Wenn aktiviert, erscheinen neue Spritzer allm\xE4hlich \xFC""ber die eingestellte Dauer statt sofort." };
Tr::Entry Tr::InfoCurl           = { "Vortex strength: Controls how much swirling/rotational force is applied to the fluid. Higher values create more turbulence.",
                                     "Wirbelst\xE4rke: Steuert die Rotationskraft der Str\xF6mung. H\xF6here Werte erzeugen mehr Turbulenzen." };
Tr::Entry Tr::InfoVelDiss        = { "Velocity dissipation: How quickly the fluid motion slows down. Higher values = faster decay, calmer fluid.",
                                     "Geschwindigkeitsabfall: Wie schnell sich die Str\xF6mung beruhigt. H\xF6here Werte = schnellerer Abfall, ruhigere Str\xF6mung." };
Tr::Entry Tr::InfoDenDiss        = { "Color dissipation: How quickly the dye/color fades away. Higher values = colors disappear faster.",
                                     "Farbabfall: Wie schnell die Farbe verblasst. H\xF6here Werte = Farben verschwinden schneller." };
Tr::Entry Tr::InfoPressure       = { "Pressure: Affects fluid incompressibility. Higher values make the fluid behave more rigidly.",
                                     "Druck: Beeinflusst die Inkompressibilit\xE4t der Str\xF6mung. H\xF6here Werte lassen die Str\xF6mung steifer wirken." };
Tr::Entry Tr::InfoPressIter      = { "Pressure iterations: Number of solver iterations per frame. More iterations = more accurate simulation but higher GPU cost.",
                                     "Druck-Iterationen: Anzahl der L\xF6sungsdurchl\xE4ufe pro Frame. Mehr Iterationen = genauere Simulation, aber h\xF6here GPU-Last." };
Tr::Entry Tr::InfoSplatRadius    = { "Splat radius: The size of each ink splat when it hits the fluid. Larger values create broader strokes.",
                                     "Spritzergr\xF6\xDF"
                                     "e: Die Gr\xF6\xDF"
                                     "e jedes Farbspritzers beim Auftreffen auf die Str\xF6mung. Gr\xF6\xDF"
                                     "ere Werte erzeugen breitere Striche." };
Tr::Entry Tr::InfoSplatForce     = { "Splat force: How much momentum each splat transfers to the fluid. Higher values = more energetic splashes.",
                                     "Spritzerkraft: Wie viel Schwung jeder Spritzer auf die Str\xF6mung \xFC""bertr\xE4gt. H\xF6here Werte = energischere Spritzer." };
Tr::Entry Tr::InfoDyeAmount      = { "Dye amount: How much color each splat deposits. Higher values produce more vivid, saturated colors.",
                                     "Farbmenge: Wie viel Farbe jeder Spritzer hinterl\xE4sst. H\xF6here Werte erzeugen lebendigere, ges\xE4ttigtere Farben." };
Tr::Entry Tr::InfoSimRes         = { "Simulation resolution: The internal grid size for velocity/pressure calculations. Higher = more detail but slower. Must be a multiple of 8.",
                                     "Simulationsaufl\xF6sung: Interne Gittergr\xF6\xDF"
                                     "e f\xFC"
                                     "r Geschwindigkeits-/Druckberechnungen. H\xF6her = mehr Details, aber langsamer. Muss ein Vielfaches von 8 sein." };
Tr::Entry Tr::InfoDyeRes         = { "Dye resolution: The texture resolution for the visible color buffer. Higher = sharper details but more GPU memory.",
                                     "Farbaufl\xF6sung: Texturaufl\xF6sung des sichtbaren Farbpuffers. H\xF6her = sch\xE4rfere Details, aber mehr GPU-Speicher." };
Tr::Entry Tr::InfoShading        = { "3D shading: Adds a pseudo-lighting effect to the fluid surface for a more dimensional look. Slightly increases GPU cost.",
                                     "3D-Schattierung: F\xFC"
                                     "gt einen Beleuchtungseffekt zur Str\xF6mungsoberfl\xE4"
                                     "che hinzu f\xFC"
                                     "r einen r\xE4umlicheren Look. Leicht erh\xF6hte GPU-Last." };
Tr::Entry Tr::InfoAutoBurst      = { "Automatic bursts: When enabled, new splats are periodically spawned at random positions to keep the fluid in motion.",
                                     "Automatische Spritzer: Wenn aktiviert, werden periodisch neue Spritzer an zuf\xE4lligen Positionen erzeugt, um die Str\xF6mung in Bewegung zu halten." };
Tr::Entry Tr::InfoFpsLimit       = { "FPS limit: Maximum frames per second. 0 = uncapped. Lower values reduce GPU usage and power consumption.",
                                     "FPS-Grenze: Maximale Bilder pro Sekunde. 0 = unbegrenzt. Niedrigere Werte reduzieren GPU-Auslastung und Stromverbrauch." };
Tr::Entry Tr::InfoPerMonitor     = { "Per-monitor mode: When enabled, each physical display runs its own independent fluid simulation instead of one shared simulation spanning all screens.",
                                     "Pro-Bildschirm-Modus: Wenn aktiviert, f\xFC"
                                     "hrt jeder Bildschirm eine eigene unabh\xE4ngige Simulation aus statt einer gemeinsamen \xFC"
                                     "ber alle Bildschirme." };

// ==================== DIALOG STATE ====================
struct ConfigDialogState {
	Config local;           // working copy edited live by the dialog controls
	Config originalBackup;  // g_config snapshot taken at dialog open, restored on Cancel
	HBRUSH brushSingle = nullptr;
	HBRUSH brushRangeStart = nullptr;
	HBRUSH brushRangeEnd = nullptr;
};

// ==================== SMALL HELPERS ====================
static void SetLabelF(HWND hDlg, int id, const char* fmt, float v) {
	char buf[32];
	snprintf(buf, sizeof(buf), fmt, v);
	SetDlgItemTextA(hDlg, id, buf);
}
static void SetLabelI(HWND hDlg, int id, const char* fmt, int v) {
	char buf[32];
	snprintf(buf, sizeof(buf), fmt, v);
	SetDlgItemTextA(hDlg, id, buf);
}

static void UpdateSwatch(HWND hDlg, ConfigDialogState* state, int swatchId, const std::string& hex) {
	HBRUSH* slot = nullptr;
	switch (swatchId) {
	case IDC_SWATCH_SINGLE:      slot = &state->brushSingle; break;
	case IDC_SWATCH_RANGE_START: slot = &state->brushRangeStart; break;
	case IDC_SWATCH_RANGE_END:   slot = &state->brushRangeEnd; break;
	default: return;
	}
	Color c = HexToColor(hex);
	HBRUSH fresh = CreateSolidBrush(RGB(
		(int)std::lround(c.r * 255.0f),
		(int)std::lround(c.g * 255.0f),
		(int)std::lround(c.b * 255.0f)));
	if (*slot) DeleteObject(*slot);
	*slot = fresh;
	InvalidateRect(GetDlgItem(hDlg, swatchId), nullptr, TRUE);
}

static void NormalizeHexEdit(HWND hDlg, ConfigDialogState* state, int editId, std::string& target, int swatchId) {
	char buf[16] = {};
	GetDlgItemTextA(hDlg, editId, buf, sizeof(buf));
	Color c = HexToColor(buf); // tolerant parsing; falls back to white on malformed input
	target = ColorToHex(c);
	SetDlgItemTextA(hDlg, editId, target.c_str());
	UpdateSwatch(hDlg, state, swatchId, target);
}

static void PickColorInto(HWND hDlg, ConfigDialogState* state, int editId, std::string& target, int swatchId) {
	static COLORREF customColors[16] = {};
	Color cur = HexToColor(target);
	CHOOSECOLORW cc{};
	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = hDlg;
	cc.lpCustColors = customColors;
	cc.rgbResult = RGB(
		(int)std::lround(cur.r * 255.0f),
		(int)std::lround(cur.g * 255.0f),
		(int)std::lround(cur.b * 255.0f));
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;
	if (ChooseColorW(&cc)) {
		Color picked{ GetRValue(cc.rgbResult) / 255.0f, GetGValue(cc.rgbResult) / 255.0f, GetBValue(cc.rgbResult) / 255.0f };
		target = ColorToHex(picked);
		SetDlgItemTextA(hDlg, editId, target.c_str());
		UpdateSwatch(hDlg, state, swatchId, target);
	}
}

static void SelectComboValue(HWND hDlg, int comboId, int value) {
	wchar_t buf[16];
	swprintf_s(buf, L"%d", value);
	HWND combo = GetDlgItem(hDlg, comboId);
	int idx = (int)SendMessageW(combo, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)buf);
	if (idx != CB_ERR) {
		SendMessageW(combo, CB_SETCURSEL, idx, 0);
	} else {
		// Custom value not in list — set the edit text directly.
		SetDlgItemTextW(hDlg, comboId, buf);
	}
}

static void UpdateEnablement(HWND hDlg, const Config& c) {
	BOOL single = c.colorMode == ColorMode::Single;
	BOOL range = c.colorMode == ColorMode::Range;
	EnableWindow(GetDlgItem(hDlg, IDC_EDIT_COLOR_SINGLE), single);
	EnableWindow(GetDlgItem(hDlg, IDC_BTN_PICK_SINGLE), single);
	EnableWindow(GetDlgItem(hDlg, IDC_EDIT_COLOR_RANGE_START), range);
	EnableWindow(GetDlgItem(hDlg, IDC_BTN_PICK_RANGE_START), range);
	EnableWindow(GetDlgItem(hDlg, IDC_EDIT_COLOR_RANGE_END), range);
	EnableWindow(GetDlgItem(hDlg, IDC_BTN_PICK_RANGE_END), range);

	EnableWindow(GetDlgItem(hDlg, IDC_SLIDER_SMOOTH_MS), c.smoothSpawn);
	EnableWindow(GetDlgItem(hDlg, IDC_LABEL_SMOOTH_MS), c.smoothSpawn);
	EnableWindow(GetDlgItem(hDlg, IDC_EDIT_SMOOTH_MS), c.smoothSpawn);

	EnableWindow(GetDlgItem(hDlg, IDC_SLIDER_AUTOINTERVAL), c.autoBurst);
	EnableWindow(GetDlgItem(hDlg, IDC_LABEL_AUTOINTERVAL), c.autoBurst);
	EnableWindow(GetDlgItem(hDlg, IDC_EDIT_AUTOINTERVAL), c.autoBurst);
}

// ==================== MANUAL EDIT APPLY ====================
// Reads the manual edit field, sets the value directly on the Config (bypassing
// slider range so the user can enter any number), then snaps the slider to the
// closest representable position.
template<typename T>
static void ApplyEditField(HWND hDlg, int editId, T& target, int sliderId,
	bool isFloat, float scale)
{
	char buf[64] = {};
	GetDlgItemTextA(hDlg, editId, buf, sizeof(buf));
	if (strlen(buf) == 0) return;
	// Always use atof so both integer and decimal input parse correctly;
	// the static_cast to T handles narrowing for int fields.
	target = static_cast<T>(atof(buf));

	// Snap slider to nearest position within its range
	int minPos = (int)SendMessageW(GetDlgItem(hDlg, sliderId), TBM_GETRANGEMIN, 0, 0);
	int maxPos = (int)SendMessageW(GetDlgItem(hDlg, sliderId), TBM_GETRANGEMAX, 0, 0);
	int pos;
	if (isFloat) {
		pos = (int)std::lround(static_cast<float>(target) * scale);
	}
	else {
		pos = static_cast<int>(target);
	}
	pos = std::clamp(pos, minPos, maxPos);
	SendMessageW(GetDlgItem(hDlg, sliderId), TBM_SETPOS, TRUE, pos);
}

// ==================== SHOW INFO ====================
static void ShowInfo(HWND hDlg, const char* text) {
	MessageBoxA(hDlg, text, "Wirbelwerk", MB_OK | MB_ICONINFORMATION);
}

// ==================== APPLY LANGUAGE ====================
static void ApplyLanguage(HWND hDlg) {
	Language lang = g_config.language;

	SetWindowTextA(hDlg, TR(Tr::DialogTitle));

	// Language combo
	HWND langCombo = GetDlgItem(hDlg, IDC_COMBO_LANGUAGE);
	SendMessageW(langCombo, CB_RESETCONTENT, 0, 0);
	SendMessageA(langCombo, CB_ADDSTRING, 0, (LPARAM)TR(Tr::LangEnglish));
	SendMessageA(langCombo, CB_ADDSTRING, 0, (LPARAM)TR(Tr::LangGerman));
	SendMessageW(langCombo, CB_SETCURSEL, (lang == Language::German) ? 1 : 0, 0);

	// Group boxes
	SetDlgItemTextA(hDlg, IDC_GROUP_COLOR, TR(Tr::GroupColor));
	SetDlgItemTextA(hDlg, IDC_GROUP_FLOW, TR(Tr::GroupFlow));
	SetDlgItemTextA(hDlg, IDC_GROUP_BRUSH, TR(Tr::GroupBrush));
	SetDlgItemTextA(hDlg, IDC_GROUP_QUALITY, TR(Tr::GroupQuality));
	SetDlgItemTextA(hDlg, IDC_GROUP_BEHAVIOR, TR(Tr::GroupBehavior));

	SetDlgItemTextA(hDlg, IDC_COLOR_RAINBOW, TR(Tr::Rainbow));
	SetDlgItemTextA(hDlg, IDC_COLOR_SINGLE, TR(Tr::SingleColor));
	SetDlgItemTextA(hDlg, IDC_COLOR_RANGE, TR(Tr::ColorRange));
	SetDlgItemTextA(hDlg, IDC_CHECK_SMOOTH_SPAWN, TR(Tr::SmoothSpawn));
	SetDlgItemTextA(hDlg, IDC_CHECK_SHADING, TR(Tr::Shading));
	SetDlgItemTextA(hDlg, IDC_CHECK_AUTOBURST, TR(Tr::AutoBursts));
	SetDlgItemTextA(hDlg, IDC_CHECK_PERMONITOR, TR(Tr::PerMonitor));

	SetDlgItemTextA(hDlg, IDC_BTN_PICK_SINGLE, TR(Tr::Pick));
	SetDlgItemTextA(hDlg, IDC_BTN_PICK_RANGE_START, TR(Tr::Pick));
	SetDlgItemTextA(hDlg, IDC_BTN_PICK_RANGE_END, TR(Tr::Pick));

	SetDlgItemTextA(hDlg, IDC_BTN_PREVIEW, TR(Tr::BtnPreview));
	SetDlgItemTextA(hDlg, IDC_BTN_DEFAULTS, TR(Tr::BtnDefaults));
	SetDlgItemTextA(hDlg, IDOK, TR(Tr::BtnSave));
	SetDlgItemTextA(hDlg, IDCANCEL, TR(Tr::BtnCancel));

	// Descriptive static labels
	SetDlgItemTextA(hDlg, IDC_LABEL_LANGUAGE, TR(Tr::LanguageLabel));
	SetDlgItemTextA(hDlg, IDC_LABEL_COLOR_SINGLE_DESC, TR(Tr::SingleColorLabel));
	SetDlgItemTextA(hDlg, IDC_LABEL_RANGE_START_DESC, TR(Tr::RangeStartLabel));
	SetDlgItemTextA(hDlg, IDC_LABEL_RANGE_END_DESC, TR(Tr::RangeEndLabel));
	SetDlgItemTextA(hDlg, IDC_LABEL_CURL_DESC, TR(Tr::VortexStrength));
	SetDlgItemTextA(hDlg, IDC_LABEL_VELDISS_DESC, TR(Tr::VelocityDecay));
	SetDlgItemTextA(hDlg, IDC_LABEL_DENDISS_DESC, TR(Tr::ColorDecay));
	SetDlgItemTextA(hDlg, IDC_LABEL_PRESSURE_DESC, TR(Tr::Pressure));
	SetDlgItemTextA(hDlg, IDC_LABEL_PRESSITER_DESC, TR(Tr::PressureIter));
	SetDlgItemTextA(hDlg, IDC_LABEL_SPLATRADIUS_DESC, TR(Tr::SplatSize));
	SetDlgItemTextA(hDlg, IDC_LABEL_SPLATFORCE_DESC, TR(Tr::SplatForce));
	SetDlgItemTextA(hDlg, IDC_LABEL_DYEAMOUNT_DESC, TR(Tr::DyeAmount));
	SetDlgItemTextA(hDlg, IDC_LABEL_SIMRES_DESC, TR(Tr::Simulation));
	SetDlgItemTextA(hDlg, IDC_LABEL_DYERES_DESC, TR(Tr::DyeRes));
	SetDlgItemTextA(hDlg, IDC_LABEL_FPSLIMIT_DESC, TR(Tr::FpsLimit));

	// Config path label (prefix is translated)
	std::wstring configPath = Config::GetConfigFilePath().wstring();
	std::wstring pathLabel = std::wstring(TR(Tr::ConfigPath), TR(Tr::ConfigPath) + strlen(TR(Tr::ConfigPath))) + configPath;
	SetDlgItemTextW(hDlg, IDC_LABEL_CONFIGPATH, pathLabel.c_str());
}

// ==================== REFRESH ALL CONTROLS ====================
static void RefreshAllControls(HWND hDlg, ConfigDialogState* state) {
	const Config& c = state->local;
	auto setPos = [&](int id, int pos) { SendMessageW(GetDlgItem(hDlg, id), TBM_SETPOS, TRUE, pos); };

	setPos(IDC_SLIDER_CURL, (int)std::lround(c.curl));
	SetLabelF(hDlg, IDC_LABEL_CURL, "%.0f", c.curl);
	SetDlgItemTextA(hDlg, IDC_EDIT_CURL, std::to_string((int)std::lround(c.curl)).c_str());

	setPos(IDC_SLIDER_VELDISS, (int)std::lround(c.velDiss * 20.0f));
	SetLabelF(hDlg, IDC_LABEL_VELDISS, "%.2f", c.velDiss);
	{
		char buf[16]; snprintf(buf, sizeof(buf), "%.2f", c.velDiss);
		SetDlgItemTextA(hDlg, IDC_EDIT_VELDISS, buf);
	}

	setPos(IDC_SLIDER_DENDISS, (int)std::lround(c.denDiss * 20.0f));
	SetLabelF(hDlg, IDC_LABEL_DENDISS, "%.2f", c.denDiss);
	{
		char buf[16]; snprintf(buf, sizeof(buf), "%.2f", c.denDiss);
		SetDlgItemTextA(hDlg, IDC_EDIT_DENDISS, buf);
	}

	setPos(IDC_SLIDER_PRESSURE, (int)std::lround(c.pressure * 50.0f));
	SetLabelF(hDlg, IDC_LABEL_PRESSURE, "%.2f", c.pressure);
	{
		char buf[16]; snprintf(buf, sizeof(buf), "%.2f", c.pressure);
		SetDlgItemTextA(hDlg, IDC_EDIT_PRESSURE, buf);
	}

	setPos(IDC_SLIDER_PRESSITER, c.pressureIter);
	SetLabelI(hDlg, IDC_LABEL_PRESSITER, "%d", c.pressureIter);
	SetDlgItemTextA(hDlg, IDC_EDIT_PRESSITER, std::to_string(c.pressureIter).c_str());

	setPos(IDC_SLIDER_SPLATRADIUS, (int)std::lround(c.splatRadius * 100.0f));
	SetLabelF(hDlg, IDC_LABEL_SPLATRADIUS, "%.2f", c.splatRadius);
	{
		char buf[16]; snprintf(buf, sizeof(buf), "%.2f", c.splatRadius);
		SetDlgItemTextA(hDlg, IDC_EDIT_SPLATRADIUS, buf);
	}

	setPos(IDC_SLIDER_SPLATFORCE, (int)std::lround(c.splatForce));
	SetLabelF(hDlg, IDC_LABEL_SPLATFORCE, "%.1fk", c.splatForce / 1000.0f);
	SetDlgItemTextA(hDlg, IDC_EDIT_SPLATFORCE, std::to_string((int)std::lround(c.splatForce)).c_str());

	setPos(IDC_SLIDER_DYEAMOUNT, (int)std::lround(c.dyeAmount * 100.0f));
	SetLabelF(hDlg, IDC_LABEL_DYEAMOUNT, "%.2f", c.dyeAmount);
	{
		char buf[16]; snprintf(buf, sizeof(buf), "%.2f", c.dyeAmount);
		SetDlgItemTextA(hDlg, IDC_EDIT_DYEAMOUNT, buf);
	}

	setPos(IDC_SLIDER_SMOOTH_MS, (int)std::lround(c.smoothSpawnMs));
	SetLabelI(hDlg, IDC_LABEL_SMOOTH_MS, "%dms", (int)std::lround(c.smoothSpawnMs));
	SetDlgItemTextA(hDlg, IDC_EDIT_SMOOTH_MS, std::to_string((int)std::lround(c.smoothSpawnMs)).c_str());

	setPos(IDC_SLIDER_AUTOINTERVAL, (int)std::lround(c.autoIntervalMs));
	SetLabelF(hDlg, IDC_LABEL_AUTOINTERVAL, "%.1fs", c.autoIntervalMs / 1000.0f);
	SetDlgItemTextA(hDlg, IDC_EDIT_AUTOINTERVAL, std::to_string((int)std::lround(c.autoIntervalMs)).c_str());

	setPos(IDC_SLIDER_FPSLIMIT, c.fpsLimit);
	if (c.fpsLimit <= 0) SetDlgItemTextA(hDlg, IDC_LABEL_FPSLIMIT, TR(Tr::Uncapped));
	else SetLabelI(hDlg, IDC_LABEL_FPSLIMIT, "%d", c.fpsLimit);
	SetDlgItemTextA(hDlg, IDC_EDIT_FPSLIMIT, std::to_string(c.fpsLimit).c_str());

	CheckDlgButton(hDlg, IDC_COLOR_RAINBOW, c.colorMode == ColorMode::Rainbow ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_COLOR_SINGLE, c.colorMode == ColorMode::Single ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_COLOR_RANGE, c.colorMode == ColorMode::Range ? BST_CHECKED : BST_UNCHECKED);

	SetDlgItemTextA(hDlg, IDC_EDIT_COLOR_SINGLE, c.colorSingleHex.c_str());
	SetDlgItemTextA(hDlg, IDC_EDIT_COLOR_RANGE_START, c.colorRangeStartHex.c_str());
	SetDlgItemTextA(hDlg, IDC_EDIT_COLOR_RANGE_END, c.colorRangeEndHex.c_str());

	UpdateSwatch(hDlg, state, IDC_SWATCH_SINGLE, c.colorSingleHex);
	UpdateSwatch(hDlg, state, IDC_SWATCH_RANGE_START, c.colorRangeStartHex);
	UpdateSwatch(hDlg, state, IDC_SWATCH_RANGE_END, c.colorRangeEndHex);

	CheckDlgButton(hDlg, IDC_CHECK_SMOOTH_SPAWN, c.smoothSpawn ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_CHECK_SHADING, c.shading ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_CHECK_AUTOBURST, c.autoBurst ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_CHECK_PERMONITOR, c.perMonitor ? BST_CHECKED : BST_UNCHECKED);

	SelectComboValue(hDlg, IDC_COMBO_SIMRES, c.simRes);
	SelectComboValue(hDlg, IDC_COMBO_DYERES, c.dyeRes);

	UpdateEnablement(hDlg, c);
	ApplyLanguage(hDlg);
}

// ==================== DIALOG PROC ====================
static INT_PTR CALLBACK ConfigDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_INITDIALOG: {
		auto* state = new ConfigDialogState();
		state->local = g_config;
		state->originalBackup = g_config;
		SetWindowLongPtrW(hDlg, GWLP_USERDATA, (LONG_PTR)state);

		HWND simCombo = GetDlgItem(hDlg, IDC_COMBO_SIMRES);
		for (int v : { 64, 128, 192, 256 }) {
			wchar_t buf[16]; swprintf_s(buf, L"%d", v);
			SendMessageW(simCombo, CB_ADDSTRING, 0, (LPARAM)buf);
		}
		HWND dyeCombo = GetDlgItem(hDlg, IDC_COMBO_DYERES);
		for (int v : { 512, 1024, 2048 }) {
			wchar_t buf[16]; swprintf_s(buf, L"%d", v);
			SendMessageW(dyeCombo, CB_ADDSTRING, 0, (LPARAM)buf);
		}

		SendMessageW(GetDlgItem(hDlg, IDC_SLIDER_CURL), TBM_SETRANGE, TRUE, MAKELONG(0, 50));
		SendMessageW(GetDlgItem(hDlg, IDC_SLIDER_VELDISS), TBM_SETRANGE, TRUE, MAKELONG(0, 80));
		SendMessageW(GetDlgItem(hDlg, IDC_SLIDER_DENDISS), TBM_SETRANGE, TRUE, MAKELONG(0, 80));
		SendMessageW(GetDlgItem(hDlg, IDC_SLIDER_PRESSURE), TBM_SETRANGE, TRUE, MAKELONG(0, 50));
		SendMessageW(GetDlgItem(hDlg, IDC_SLIDER_PRESSITER), TBM_SETRANGE, TRUE, MAKELONG(1, 60));
		SendMessageW(GetDlgItem(hDlg, IDC_SLIDER_SPLATRADIUS), TBM_SETRANGE, TRUE, MAKELONG(5, 100));
		SendMessageW(GetDlgItem(hDlg, IDC_SLIDER_SPLATFORCE), TBM_SETRANGE, TRUE, MAKELONG(1000, 12000));
		SendMessageW(GetDlgItem(hDlg, IDC_SLIDER_DYEAMOUNT), TBM_SETRANGE, TRUE, MAKELONG(5, 50));
		SendMessageW(GetDlgItem(hDlg, IDC_SLIDER_SMOOTH_MS), TBM_SETRANGE, TRUE, MAKELONG(100, 3000));
		SendMessageW(GetDlgItem(hDlg, IDC_SLIDER_AUTOINTERVAL), TBM_SETRANGE, TRUE, MAKELONG(0, 8000));
		SendMessageW(GetDlgItem(hDlg, IDC_SLIDER_FPSLIMIT), TBM_SETRANGE, TRUE, MAKELONG(0, 240));

		std::wstring configPath = Config::GetConfigFilePath().wstring();
		std::wstring pathLabel = std::wstring(TR(Tr::ConfigPath), TR(Tr::ConfigPath) + strlen(TR(Tr::ConfigPath))) + configPath;
		SetDlgItemTextW(hDlg, IDC_LABEL_CONFIGPATH, pathLabel.c_str());

		RefreshAllControls(hDlg, state);
		return TRUE;
	}

	case WM_HSCROLL: {
		auto* state = (ConfigDialogState*)GetWindowLongPtrW(hDlg, GWLP_USERDATA);
		HWND ctl = (HWND)lParam;
		if (!state || !ctl) break;
		int id = GetDlgCtrlID(ctl);
		int pos = (int)SendMessageW(ctl, TBM_GETPOS, 0, 0);
		Config& c = state->local;
		switch (id) {
		case IDC_SLIDER_CURL:
			c.curl = (float)pos; SetLabelF(hDlg, IDC_LABEL_CURL, "%.0f", c.curl);
			SetDlgItemTextA(hDlg, IDC_EDIT_CURL, std::to_string(pos).c_str()); break;
		case IDC_SLIDER_VELDISS:
			c.velDiss = pos / 20.0f; SetLabelF(hDlg, IDC_LABEL_VELDISS, "%.2f", c.velDiss);
			{ char buf[16]; snprintf(buf, sizeof(buf), "%.2f", c.velDiss); SetDlgItemTextA(hDlg, IDC_EDIT_VELDISS, buf); } break;
		case IDC_SLIDER_DENDISS:
			c.denDiss = pos / 20.0f; SetLabelF(hDlg, IDC_LABEL_DENDISS, "%.2f", c.denDiss);
			{ char buf[16]; snprintf(buf, sizeof(buf), "%.2f", c.denDiss); SetDlgItemTextA(hDlg, IDC_EDIT_DENDISS, buf); } break;
		case IDC_SLIDER_PRESSURE:
			c.pressure = pos / 50.0f; SetLabelF(hDlg, IDC_LABEL_PRESSURE, "%.2f", c.pressure);
			{ char buf[16]; snprintf(buf, sizeof(buf), "%.2f", c.pressure); SetDlgItemTextA(hDlg, IDC_EDIT_PRESSURE, buf); } break;
		case IDC_SLIDER_PRESSITER:
			c.pressureIter = pos; SetLabelI(hDlg, IDC_LABEL_PRESSITER, "%d", c.pressureIter);
			SetDlgItemTextA(hDlg, IDC_EDIT_PRESSITER, std::to_string(pos).c_str()); break;
		case IDC_SLIDER_SPLATRADIUS:
			c.splatRadius = pos / 100.0f; SetLabelF(hDlg, IDC_LABEL_SPLATRADIUS, "%.2f", c.splatRadius);
			{ char buf[16]; snprintf(buf, sizeof(buf), "%.2f", c.splatRadius); SetDlgItemTextA(hDlg, IDC_EDIT_SPLATRADIUS, buf); } break;
		case IDC_SLIDER_SPLATFORCE:
			c.splatForce = (float)pos; SetLabelF(hDlg, IDC_LABEL_SPLATFORCE, "%.1fk", c.splatForce / 1000.0f);
			SetDlgItemTextA(hDlg, IDC_EDIT_SPLATFORCE, std::to_string(pos).c_str()); break;
		case IDC_SLIDER_DYEAMOUNT:
			c.dyeAmount = pos / 100.0f; SetLabelF(hDlg, IDC_LABEL_DYEAMOUNT, "%.2f", c.dyeAmount);
			{ char buf[16]; snprintf(buf, sizeof(buf), "%.2f", c.dyeAmount); SetDlgItemTextA(hDlg, IDC_EDIT_DYEAMOUNT, buf); } break;
		case IDC_SLIDER_SMOOTH_MS:
			c.smoothSpawnMs = (float)pos; SetLabelI(hDlg, IDC_LABEL_SMOOTH_MS, "%dms", pos);
			SetDlgItemTextA(hDlg, IDC_EDIT_SMOOTH_MS, std::to_string(pos).c_str()); break;
		case IDC_SLIDER_AUTOINTERVAL:
			c.autoIntervalMs = (float)pos; SetLabelF(hDlg, IDC_LABEL_AUTOINTERVAL, "%.1fs", pos / 1000.0f);
			SetDlgItemTextA(hDlg, IDC_EDIT_AUTOINTERVAL, std::to_string(pos).c_str()); break;
		case IDC_SLIDER_FPSLIMIT:
			c.fpsLimit = pos;
			if (pos <= 0) SetDlgItemTextA(hDlg, IDC_LABEL_FPSLIMIT, TR(Tr::Uncapped));
			else SetLabelI(hDlg, IDC_LABEL_FPSLIMIT, "%d", pos);
			SetDlgItemTextA(hDlg, IDC_EDIT_FPSLIMIT, std::to_string(pos).c_str());
			break;
		}
		return TRUE;
	}

	case WM_CTLCOLORSTATIC: {
		auto* state = (ConfigDialogState*)GetWindowLongPtrW(hDlg, GWLP_USERDATA);
		if (state) {
			HWND ctl = (HWND)lParam;
			if (ctl == GetDlgItem(hDlg, IDC_SWATCH_SINGLE) && state->brushSingle)
				return (INT_PTR)state->brushSingle;
			if (ctl == GetDlgItem(hDlg, IDC_SWATCH_RANGE_START) && state->brushRangeStart)
				return (INT_PTR)state->brushRangeStart;
			if (ctl == GetDlgItem(hDlg, IDC_SWATCH_RANGE_END) && state->brushRangeEnd)
				return (INT_PTR)state->brushRangeEnd;
		}
		return FALSE;
	}

	case WM_COMMAND: {
		auto* state = (ConfigDialogState*)GetWindowLongPtrW(hDlg, GWLP_USERDATA);
		if (!state) break;
		int id = LOWORD(wParam);
		int code = HIWORD(wParam);
		Config& c = state->local;

		switch (id) {
		// ---- Language ----
		case IDC_COMBO_LANGUAGE:
			if (code == CBN_SELCHANGE) {
				int sel = (int)SendMessageW(GetDlgItem(hDlg, IDC_COMBO_LANGUAGE), CB_GETCURSEL, 0, 0);
				c.language = (sel == 1) ? Language::German : Language::English;
				g_config.language = c.language;
				ApplyLanguage(hDlg);
			}
			return TRUE;

		// ---- Color mode ----
		case IDC_COLOR_RAINBOW:
		case IDC_COLOR_SINGLE:
		case IDC_COLOR_RANGE:
			if (code == BN_CLICKED) {
				c.colorMode = id == IDC_COLOR_RAINBOW ? ColorMode::Rainbow
					: id == IDC_COLOR_SINGLE ? ColorMode::Single : ColorMode::Range;
				UpdateEnablement(hDlg, c);
			}
			return TRUE;

		// ---- Color edits ----
		case IDC_EDIT_COLOR_SINGLE:
			if (code == EN_KILLFOCUS) NormalizeHexEdit(hDlg, state, IDC_EDIT_COLOR_SINGLE, c.colorSingleHex, IDC_SWATCH_SINGLE);
			return TRUE;
		case IDC_EDIT_COLOR_RANGE_START:
			if (code == EN_KILLFOCUS) NormalizeHexEdit(hDlg, state, IDC_EDIT_COLOR_RANGE_START, c.colorRangeStartHex, IDC_SWATCH_RANGE_START);
			return TRUE;
		case IDC_EDIT_COLOR_RANGE_END:
			if (code == EN_KILLFOCUS) NormalizeHexEdit(hDlg, state, IDC_EDIT_COLOR_RANGE_END, c.colorRangeEndHex, IDC_SWATCH_RANGE_END);
			return TRUE;

		// ---- Color pick buttons ----
		case IDC_BTN_PICK_SINGLE:
			if (code == BN_CLICKED) PickColorInto(hDlg, state, IDC_EDIT_COLOR_SINGLE, c.colorSingleHex, IDC_SWATCH_SINGLE);
			return TRUE;
		case IDC_BTN_PICK_RANGE_START:
			if (code == BN_CLICKED) PickColorInto(hDlg, state, IDC_EDIT_COLOR_RANGE_START, c.colorRangeStartHex, IDC_SWATCH_RANGE_START);
			return TRUE;
		case IDC_BTN_PICK_RANGE_END:
			if (code == BN_CLICKED) PickColorInto(hDlg, state, IDC_EDIT_COLOR_RANGE_END, c.colorRangeEndHex, IDC_SWATCH_RANGE_END);
			return TRUE;

		// ---- Checkboxes ----
		case IDC_CHECK_SMOOTH_SPAWN:
			if (code == BN_CLICKED) {
				c.smoothSpawn = IsDlgButtonChecked(hDlg, IDC_CHECK_SMOOTH_SPAWN) == BST_CHECKED;
				UpdateEnablement(hDlg, c);
			}
			return TRUE;
		case IDC_CHECK_SHADING:
			if (code == BN_CLICKED) c.shading = IsDlgButtonChecked(hDlg, IDC_CHECK_SHADING) == BST_CHECKED;
			return TRUE;
		case IDC_CHECK_AUTOBURST:
			if (code == BN_CLICKED) {
				c.autoBurst = IsDlgButtonChecked(hDlg, IDC_CHECK_AUTOBURST) == BST_CHECKED;
				UpdateEnablement(hDlg, c);
			}
			return TRUE;
		case IDC_CHECK_PERMONITOR:
			if (code == BN_CLICKED) c.perMonitor = IsDlgButtonChecked(hDlg, IDC_CHECK_PERMONITOR) == BST_CHECKED;
			return TRUE;

		// ---- Combos ----
		case IDC_COMBO_SIMRES:
			if (code == CBN_SELCHANGE || code == CBN_EDITCHANGE || code == CBN_KILLFOCUS) {
				char buf[16]; GetDlgItemTextA(hDlg, IDC_COMBO_SIMRES, buf, sizeof(buf));
				int v = atoi(buf);
				if (v >= 8) c.simRes = ((v + 7) / 8) * 8; // snap to multiple of 8
			}
			return TRUE;
		case IDC_COMBO_DYERES:
			if (code == CBN_SELCHANGE || code == CBN_EDITCHANGE || code == CBN_KILLFOCUS) {
				char buf[16]; GetDlgItemTextA(hDlg, IDC_COMBO_DYERES, buf, sizeof(buf));
				int v = atoi(buf);
				if (v >= 8) c.dyeRes = ((v + 7) / 8) * 8;
			}
			return TRUE;

		// ---- Manual numeric edits (apply on kill focus) ----
		case IDC_EDIT_CURL:
			if (code == EN_KILLFOCUS) {
				ApplyEditField(hDlg, IDC_EDIT_CURL, c.curl, IDC_SLIDER_CURL, false, 1.0f);
				SetLabelF(hDlg, IDC_LABEL_CURL, "%.0f", c.curl);
			}
			return TRUE;
		case IDC_EDIT_VELDISS:
			if (code == EN_KILLFOCUS) {
				ApplyEditField(hDlg, IDC_EDIT_VELDISS, c.velDiss, IDC_SLIDER_VELDISS, true, 20.0f);
				SetLabelF(hDlg, IDC_LABEL_VELDISS, "%.2f", c.velDiss);
			}
			return TRUE;
		case IDC_EDIT_DENDISS:
			if (code == EN_KILLFOCUS) {
				ApplyEditField(hDlg, IDC_EDIT_DENDISS, c.denDiss, IDC_SLIDER_DENDISS, true, 20.0f);
				SetLabelF(hDlg, IDC_LABEL_DENDISS, "%.2f", c.denDiss);
			}
			return TRUE;
		case IDC_EDIT_PRESSURE:
			if (code == EN_KILLFOCUS) {
				ApplyEditField(hDlg, IDC_EDIT_PRESSURE, c.pressure, IDC_SLIDER_PRESSURE, true, 50.0f);
				SetLabelF(hDlg, IDC_LABEL_PRESSURE, "%.2f", c.pressure);
			}
			return TRUE;
		case IDC_EDIT_PRESSITER:
			if (code == EN_KILLFOCUS) {
				ApplyEditField(hDlg, IDC_EDIT_PRESSITER, c.pressureIter, IDC_SLIDER_PRESSITER, false, 1.0f);
				SetLabelI(hDlg, IDC_LABEL_PRESSITER, "%d", c.pressureIter);
			}
			return TRUE;
		case IDC_EDIT_SPLATRADIUS:
			if (code == EN_KILLFOCUS) {
				ApplyEditField(hDlg, IDC_EDIT_SPLATRADIUS, c.splatRadius, IDC_SLIDER_SPLATRADIUS, true, 100.0f);
				SetLabelF(hDlg, IDC_LABEL_SPLATRADIUS, "%.2f", c.splatRadius);
			}
			return TRUE;
		case IDC_EDIT_SPLATFORCE:
			if (code == EN_KILLFOCUS) {
				ApplyEditField(hDlg, IDC_EDIT_SPLATFORCE, c.splatForce, IDC_SLIDER_SPLATFORCE, false, 1.0f);
				SetLabelF(hDlg, IDC_LABEL_SPLATFORCE, "%.1fk", c.splatForce / 1000.0f);
			}
			return TRUE;
		case IDC_EDIT_DYEAMOUNT:
			if (code == EN_KILLFOCUS) {
				ApplyEditField(hDlg, IDC_EDIT_DYEAMOUNT, c.dyeAmount, IDC_SLIDER_DYEAMOUNT, true, 100.0f);
				SetLabelF(hDlg, IDC_LABEL_DYEAMOUNT, "%.2f", c.dyeAmount);
			}
			return TRUE;
		case IDC_EDIT_SMOOTH_MS:
			if (code == EN_KILLFOCUS) {
				ApplyEditField(hDlg, IDC_EDIT_SMOOTH_MS, c.smoothSpawnMs, IDC_SLIDER_SMOOTH_MS, false, 1.0f);
				SetLabelI(hDlg, IDC_LABEL_SMOOTH_MS, "%dms", (int)std::lround(c.smoothSpawnMs));
			}
			return TRUE;
		case IDC_EDIT_AUTOINTERVAL:
			if (code == EN_KILLFOCUS) {
				ApplyEditField(hDlg, IDC_EDIT_AUTOINTERVAL, c.autoIntervalMs, IDC_SLIDER_AUTOINTERVAL, false, 1.0f);
				SetLabelF(hDlg, IDC_LABEL_AUTOINTERVAL, "%.1fs", c.autoIntervalMs / 1000.0f);
			}
			return TRUE;
		case IDC_EDIT_FPSLIMIT:
			if (code == EN_KILLFOCUS) {
				ApplyEditField(hDlg, IDC_EDIT_FPSLIMIT, c.fpsLimit, IDC_SLIDER_FPSLIMIT, false, 1.0f);
				if (c.fpsLimit <= 0) SetDlgItemTextA(hDlg, IDC_LABEL_FPSLIMIT, TR(Tr::Uncapped));
				else SetLabelI(hDlg, IDC_LABEL_FPSLIMIT, "%d", c.fpsLimit);
			}
			return TRUE;

		// ---- Info buttons ----
		case IDC_INFO_COLORMODE:
			if (code == BN_CLICKED) ShowInfo(hDlg, TR(Tr::InfoColorMode)); return TRUE;
		case IDC_INFO_SMOOTHSPAWN:
			if (code == BN_CLICKED) ShowInfo(hDlg, TR(Tr::InfoSmoothSpawn)); return TRUE;
		case IDC_INFO_CURL:
			if (code == BN_CLICKED) ShowInfo(hDlg, TR(Tr::InfoCurl)); return TRUE;
		case IDC_INFO_VELDISS:
			if (code == BN_CLICKED) ShowInfo(hDlg, TR(Tr::InfoVelDiss)); return TRUE;
		case IDC_INFO_DENDISS:
			if (code == BN_CLICKED) ShowInfo(hDlg, TR(Tr::InfoDenDiss)); return TRUE;
		case IDC_INFO_PRESSURE:
			if (code == BN_CLICKED) ShowInfo(hDlg, TR(Tr::InfoPressure)); return TRUE;
		case IDC_INFO_PRESSITER:
			if (code == BN_CLICKED) ShowInfo(hDlg, TR(Tr::InfoPressIter)); return TRUE;
		case IDC_INFO_SPLATRADIUS:
			if (code == BN_CLICKED) ShowInfo(hDlg, TR(Tr::InfoSplatRadius)); return TRUE;
		case IDC_INFO_SPLATFORCE:
			if (code == BN_CLICKED) ShowInfo(hDlg, TR(Tr::InfoSplatForce)); return TRUE;
		case IDC_INFO_DYEAMOUNT:
			if (code == BN_CLICKED) ShowInfo(hDlg, TR(Tr::InfoDyeAmount)); return TRUE;
		case IDC_INFO_SIMRES:
			if (code == BN_CLICKED) ShowInfo(hDlg, TR(Tr::InfoSimRes)); return TRUE;
		case IDC_INFO_DYERES:
			if (code == BN_CLICKED) ShowInfo(hDlg, TR(Tr::InfoDyeRes)); return TRUE;
		case IDC_INFO_SHADING:
			if (code == BN_CLICKED) ShowInfo(hDlg, TR(Tr::InfoShading)); return TRUE;
		case IDC_INFO_AUTOBURST:
			if (code == BN_CLICKED) ShowInfo(hDlg, TR(Tr::InfoAutoBurst)); return TRUE;
		case IDC_INFO_FPSLIMIT:
			if (code == BN_CLICKED) ShowInfo(hDlg, TR(Tr::InfoFpsLimit)); return TRUE;
		case IDC_INFO_PERMONITOR:
			if (code == BN_CLICKED) ShowInfo(hDlg, TR(Tr::InfoPerMonitor)); return TRUE;

		// ---- Preview ----
		case IDC_BTN_PREVIEW:
			if (code == BN_CLICKED) {
				RunFloatingPreviewWindow(&c);
			}
			return TRUE;

		// ---- Defaults ----
		case IDC_BTN_DEFAULTS:
			if (code == BN_CLICKED) {
				c = Config();
				RefreshAllControls(hDlg, state);
			}
			return TRUE;

		// ---- Save ----
		case IDOK:
			g_config = c;
			g_config.Save();
			state->originalBackup = c;
			return TRUE;

		// ---- Cancel ----
		case IDCANCEL:
			g_config = state->originalBackup;
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		break;
	}

	case WM_CLOSE: {
		auto* state = (ConfigDialogState*)GetWindowLongPtrW(hDlg, GWLP_USERDATA);
		if (state) g_config = state->originalBackup;
		EndDialog(hDlg, IDCANCEL);
		return TRUE;
	}

	case WM_DESTROY: {
		auto* state = (ConfigDialogState*)GetWindowLongPtrW(hDlg, GWLP_USERDATA);
		if (state) {
			if (state->brushSingle) DeleteObject(state->brushSingle);
			if (state->brushRangeStart) DeleteObject(state->brushRangeStart);
			if (state->brushRangeEnd) DeleteObject(state->brushRangeEnd);
			delete state;
			SetWindowLongPtrW(hDlg, GWLP_USERDATA, 0);
		}
		return TRUE;
	}
	}
	return FALSE;
}

// ==================== ENTRY POINT ====================
bool ShowConfigDialog(void* hInstanceVoid, void* hwndParentVoid) {
	HINSTANCE hInstance = (HINSTANCE)hInstanceVoid;
	HWND hwndParent = (HWND)hwndParentVoid;

	INITCOMMONCONTROLSEX icc{ sizeof(icc), ICC_BAR_CLASSES | ICC_STANDARD_CLASSES };
	InitCommonControlsEx(&icc);

	g_config.Load();

	INT_PTR result = DialogBoxParamW(hInstance, MAKEINTRESOURCEW(IDD_CONFIGDIALOG), hwndParent, ConfigDlgProc, 0);
	return result == IDOK;
}
