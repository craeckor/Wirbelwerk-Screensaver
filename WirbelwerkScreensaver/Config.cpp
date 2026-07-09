#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <shlobj.h>
#include "Config.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstdio>

using json = nlohmann::json;

Config g_config;

// ==================== COLOR HELPERS ====================
static int HexNibblePair(const std::string& hex, size_t pos) {
	if (pos + 1 >= hex.size()) return 0;
	return std::stoi(hex.substr(pos, 2), nullptr, 16);
}

Color HexToColor(const std::string& hexIn) {
	std::string hex = hexIn;
	if (!hex.empty() && hex[0] == '#') hex = hex.substr(1);
	if (hex.size() < 6) return Color{ 1.0f, 1.0f, 1.0f };
	Color c;
	try {
		c.r = HexNibblePair(hex, 0) / 255.0f;
		c.g = HexNibblePair(hex, 2) / 255.0f;
		c.b = HexNibblePair(hex, 4) / 255.0f;
	}
	catch (...) {
		c = Color{ 1.0f, 1.0f, 1.0f };
	}
	return c;
}

std::string ColorToHex(const Color& c) {
	char buf[8];
	auto clampByte = [](float v) { return (int)std::clamp(v * 255.0f, 0.0f, 255.0f); };
	std::snprintf(buf, sizeof(buf), "#%02X%02X%02X", clampByte(c.r), clampByte(c.g), clampByte(c.b));
	return std::string(buf);
}

Color LerpColor(const Color& a, const Color& b, float t) {
	return Color{ a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t };
}

// ==================== PATHS ====================
std::filesystem::path Config::GetConfigFilePath() {
	PWSTR path = nullptr;
	std::filesystem::path result;
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &path))) {
		result = std::filesystem::path(path) / L"Wirbelwerk" / L"wirbelwerk.json";
		CoTaskMemFree(path);
	}
	else {
		// Extremely unlikely fallback: current directory.
		result = std::filesystem::path(L"wirbelwerk.json");
	}
	return result;
}

std::filesystem::path Config::GetLegacyConfigFilePath() {
	wchar_t exePath[MAX_PATH] = {};
	GetModuleFileNameW(nullptr, exePath, MAX_PATH);
	std::filesystem::path dir = std::filesystem::path(exePath).parent_path();
	return dir / L"wirbelwerk.json";
}

// ==================== JSON (DE)SERIALIZATION ====================
static void FromJson(const json& j, Config& cfg) {
	if (j.contains("SIM_RESOLUTION")) cfg.simRes = j["SIM_RESOLUTION"];
	if (j.contains("DYE_RESOLUTION")) cfg.dyeRes = j["DYE_RESOLUTION"];
	if (j.contains("VELOCITY_DISSIPATION")) cfg.velDiss = j["VELOCITY_DISSIPATION"];
	if (j.contains("DENSITY_DISSIPATION")) cfg.denDiss = j["DENSITY_DISSIPATION"];
	if (j.contains("PRESSURE")) cfg.pressure = j["PRESSURE"];
	if (j.contains("PRESSURE_ITERATIONS")) cfg.pressureIter = j["PRESSURE_ITERATIONS"];
	if (j.contains("CURL")) cfg.curl = j["CURL"];
	if (j.contains("SPLAT_RADIUS")) cfg.splatRadius = j["SPLAT_RADIUS"];
	if (j.contains("SPLAT_FORCE")) cfg.splatForce = j["SPLAT_FORCE"];
	if (j.contains("DYE_AMOUNT")) cfg.dyeAmount = j["DYE_AMOUNT"];
	if (j.contains("SHADING")) cfg.shading = j["SHADING"];

	if (j.contains("COLOR_MODE")) {
		std::string m = j["COLOR_MODE"];
		if (m == "SINGLE") cfg.colorMode = ColorMode::Single;
		else if (m == "RANGE") cfg.colorMode = ColorMode::Range;
		else cfg.colorMode = ColorMode::Rainbow;
	}
	else if (j.contains("RAINBOW")) {
		// Legacy boolean-only config.
		cfg.colorMode = (bool)j["RAINBOW"] ? ColorMode::Rainbow : ColorMode::Single;
	}
	cfg.rainbow = (cfg.colorMode == ColorMode::Rainbow);

	if (j.contains("COLOR_SINGLE")) cfg.colorSingleHex = j["COLOR_SINGLE"];
	if (j.contains("COLOR_RANGE_START")) cfg.colorRangeStartHex = j["COLOR_RANGE_START"];
	if (j.contains("COLOR_RANGE_END")) cfg.colorRangeEndHex = j["COLOR_RANGE_END"];
	if (j.contains("SMOOTH_SPAWN")) cfg.smoothSpawn = j["SMOOTH_SPAWN"];
	if (j.contains("SMOOTH_SPAWN_MS")) cfg.smoothSpawnMs = j["SMOOTH_SPAWN_MS"];

	if (j.contains("AUTO_BURST")) cfg.autoBurst = j["AUTO_BURST"];
	if (j.contains("AUTO_INTERVAL_MS")) cfg.autoIntervalMs = j["AUTO_INTERVAL_MS"];
	if (j.contains("FPS_LIMIT")) cfg.fpsLimit = j["FPS_LIMIT"];
	if (j.contains("PER_MONITOR")) cfg.perMonitor = j["PER_MONITOR"];

	if (j.contains("LANGUAGE")) {
		std::string lang = j["LANGUAGE"];
		if (lang == "GERMAN") cfg.language = Language::German;
		else cfg.language = Language::English;
	}
}

static json ToJson(const Config& cfg) {
	json j;
	j["SIM_RESOLUTION"] = cfg.simRes;
	j["DYE_RESOLUTION"] = cfg.dyeRes;
	j["VELOCITY_DISSIPATION"] = cfg.velDiss;
	j["DENSITY_DISSIPATION"] = cfg.denDiss;
	j["PRESSURE"] = cfg.pressure;
	j["PRESSURE_ITERATIONS"] = cfg.pressureIter;
	j["CURL"] = cfg.curl;
	j["SPLAT_RADIUS"] = cfg.splatRadius;
	j["SPLAT_FORCE"] = cfg.splatForce;
	j["DYE_AMOUNT"] = cfg.dyeAmount;
	j["SHADING"] = cfg.shading;

	switch (cfg.colorMode) {
	case ColorMode::Single: j["COLOR_MODE"] = "SINGLE"; break;
	case ColorMode::Range:  j["COLOR_MODE"] = "RANGE"; break;
	default:                j["COLOR_MODE"] = "RAINBOW"; break;
	}
	j["RAINBOW"] = (cfg.colorMode == ColorMode::Rainbow); // legacy readers
	j["COLOR_SINGLE"] = cfg.colorSingleHex;
	j["COLOR_RANGE_START"] = cfg.colorRangeStartHex;
	j["COLOR_RANGE_END"] = cfg.colorRangeEndHex;
	j["SMOOTH_SPAWN"] = cfg.smoothSpawn;
	j["SMOOTH_SPAWN_MS"] = cfg.smoothSpawnMs;

	j["AUTO_BURST"] = cfg.autoBurst;
	j["AUTO_INTERVAL_MS"] = cfg.autoIntervalMs;
	j["FPS_LIMIT"] = cfg.fpsLimit;
	j["PER_MONITOR"] = cfg.perMonitor;
	j["LANGUAGE"] = (cfg.language == Language::German) ? "GERMAN" : "ENGLISH";
	return j;
}

// ==================== LOAD / SAVE ====================
static bool LoadFromPath(const std::filesystem::path& path, Config& cfg) {
	std::ifstream f(path);
	if (!f.is_open()) return false;
	json j;
	try { f >> j; }
	catch (...) { return false; }
	FromJson(j, cfg);
	return true;
}

bool Config::Load() {
	const auto primary = GetConfigFilePath();
	if (LoadFromPath(primary, *this)) return true;

	// First run: try importing a legacy config sitting next to the executable
	// (e.g. a hand-copied wirbelwerk.json), then persist it into AppData so
	// future saves never need to touch the (possibly admin-protected) install folder.
	const auto legacy = GetLegacyConfigFilePath();
	if (LoadFromPath(legacy, *this)) {
		Save();
		return true;
	}

	// No config anywhere yet - keep defaults and write them out for next time.
	Save();
	return false;
}

bool Config::Save() const {
	try {
		const auto path = GetConfigFilePath();
		std::error_code ec;
		std::filesystem::create_directories(path.parent_path(), ec);
		std::ofstream f(path, std::ios::trunc);
		if (!f.is_open()) return false;
		f << ToJson(*this).dump(4);
		return true;
	}
	catch (...) {
		return false;
	}
}
