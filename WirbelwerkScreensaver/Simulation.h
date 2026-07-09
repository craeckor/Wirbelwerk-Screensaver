#pragma once
#include <glad/glad.h>
#include <chrono>
#include <random>
#include <vector>
#include "Config.h"

// ==================== FBO / TEXTURE STRUCTS ====================
struct Texture {
	GLuint id = 0;
	int width = 0, height = 0;
	float texelSizeX = 0, texelSizeY = 0;
};

struct FBO {
	Texture tex;
	GLuint fbo = 0;
};

struct DoubleFBO {
	FBO read, write;
	int width = 0, height = 0;
	float texelSizeX = 0, texelSizeY = 0;
	void swap() { std::swap(read, write); }
};

// Compiles all shared shader programs and the fullscreen quad geometry.
// Must be called once after a GL context is current, before any FluidSim::Init() call.
void InitSharedSimResources();
// Frees all shared shader programs and geometry. Safe to call after all FluidSim
// instances have been destroyed and the GL context is still current.
void DestroySharedSimResources();

// Maximum number of pending smooth-spawn splats queued at once. If the queue
// exceeds this, the oldest entries are dropped to prevent unbounded memory growth
// (e.g. when the simulation is paused while splats keep being queued).
constexpr size_t kMaxPendingSplats = 256;

// One independent fluid simulation instance. Multiple instances can coexist in the same
// GL context (used for per-monitor mode, where each display gets its own instance so the
// simulations are not synchronized/mirrored copies of one another).
class FluidSim {
public:
	~FluidSim(); // Frees all GPU resources (textures, FBOs) for this instance.

	// viewportW/viewportH: the on-screen pixel size this instance renders into. Drives the
	// internal simulation/dye framebuffer resolution (scaled by g_config.simRes/dyeRes).
	void Init(int viewportW, int viewportH);
	void Resize(int viewportW, int viewportH);

	// Advances fluid step, auto-burst scheduling, and smooth-spawn ramping by dt seconds.
	void Update(float dt);
	// Renders the current dye buffer into the given viewport rectangle of the bound framebuffer.
	void Render(int vx, int vy, int vw, int vh);

	// Immediately injects a single splat at full strength (bypasses smooth-spawn ramping).
	void Splat(float x, float y, float dx, float dy, Color color, float amount);
	// Spawns `count` random splats; honors g_config.smoothSpawn (ramped fade-in) when enabled.
	void MultiSplat(int count, float amountScale = 1.0f);
	void ClearAll();

	bool paused = false;

private:
	Color PickSplatColor();
	void SplatInternal(float x, float y, float dx, float dy, Color color, float amount);
	void QueueSplat(float x, float y, float dx, float dy, Color color, float amount);
	void AdvancePending(float dt);
	void Step(float dt);
	void ComputeSizes(int viewportW, int viewportH);

	struct PendingSplat {
		float x = 0, y = 0, dx = 0, dy = 0;
		Color color;
		float totalAmount = 0;
		float durationMs = 600.0f;
		float elapsedMs = 0.0f;
		float injectedFraction = 0.0f;
	};
	std::vector<PendingSplat> m_pending;

	int m_simW = 0, m_simH = 0, m_dyeW = 0, m_dyeH = 0;
	float m_texelX = 0, m_texelY = 0, m_dyeTexelX = 0, m_dyeTexelY = 0;
	int m_viewportW = 1, m_viewportH = 1;

	DoubleFBO m_dye, m_velocity;
	FBO m_divergence, m_curl;
	DoubleFBO m_pressure;

	std::mt19937 m_rng{ std::random_device{}() };
	std::uniform_real_distribution<float> m_dist01{ 0.0f, 1.0f };

	std::chrono::steady_clock::time_point m_lastBurst = std::chrono::steady_clock::now();
	bool m_initialized = false;
};
