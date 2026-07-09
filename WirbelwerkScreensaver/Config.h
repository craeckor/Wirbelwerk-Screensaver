#pragma once
#include <string>
#include <filesystem>

// ==================== COLOR ====================
struct Color { float r = 1.0f, g = 1.0f, b = 1.0f; };

Color HexToColor(const std::string& hex);
std::string ColorToHex(const Color& c);
Color LerpColor(const Color& a, const Color& b, float t);

// ==================== COLOR MODE ====================
enum class ColorMode {
	Rainbow = 0, // classic HSV cycling, one random hue per splat
	Single = 1,  // every splat uses the same fixed color
	Range = 2,   // every splat uses a random color interpolated between two hex endpoints
};

// ==================== LANGUAGE ====================
enum class Language {
	English = 0,
	German = 1,
};

// ==================== CONFIG ====================
struct Config {
	// Simulation
	int   simRes = 128;
	int   dyeRes = 1024;
	float velDiss = 0.2f;
	float denDiss = 1.0f;
	float pressure = 0.8f;
	int   pressureIter = 20;
	float curl = 30.0f;
	float splatRadius = 0.25f;
	float splatForce = 6000.0f;
	float dyeAmount = 0.18f;

	// Appearance
	bool        shading = true;
	ColorMode   colorMode = ColorMode::Rainbow;
	std::string colorSingleHex = "#6366F1";
	std::string colorRangeStartHex = "#FF3B6B";
	std::string colorRangeEndHex = "#4DD2FF";
	bool        smoothSpawn = true;    // fade new splats in instead of instant pop-in
	float       smoothSpawnMs = 600.0f;

	// Behavior
	bool  autoBurst = true;
	float autoIntervalMs = 2600.0f;
	bool  paused = false;
	int   fpsLimit = 60;
	bool  perMonitor = false; // each display runs its own independent simulation instance

	// Deprecated: superseded by colorMode, kept in sync for any legacy references.
	bool rainbow = true;

	// Language
	Language language = Language::English;

	// %LOCALAPPDATA%\Wirbelwerk\wirbelwerk.json - always user-writable, no admin/UAC needed.
	static std::filesystem::path GetConfigFilePath();
	// Legacy location next to the executable (e.g. a System32-installed .scr); read-only import source.
	static std::filesystem::path GetLegacyConfigFilePath();

	// Loads from the AppData path, importing/migrating a legacy exe-directory config on first
	// run if present. Always leaves *this in a usable state (falls back to defaults on any error).
	bool Load();
	bool Save() const;
};

extern Config g_config;
