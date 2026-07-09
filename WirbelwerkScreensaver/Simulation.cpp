#define NOMINMAX
#include "Simulation.h"
#include <iostream>
#include <algorithm>

// ==================== SHADER SOURCES (GLSL 330 core) — unchanged fluid-sim math ====================
static const char* vertexShaderSrc = R"(#version 330 core
layout(location = 0) in vec2 aPos;
out vec2 vUv;
out vec2 vL;
out vec2 vR;
out vec2 vT;
out vec2 vB;
uniform vec2 texelSize;
void main() {
	vUv = aPos * 0.5 + 0.5;
	vL = vUv - vec2(texelSize.x, 0.0);
	vR = vUv + vec2(texelSize.x, 0.0);
	vT = vUv + vec2(0.0, texelSize.y);
	vB = vUv - vec2(0.0, texelSize.y);
	gl_Position = vec4(aPos, 0.0, 1.0);
})";

static const char* copyFragSrc = R"(#version 330 core
in vec2 vUv;
uniform sampler2D uTexture;
out vec4 FragColor;
void main() { FragColor = texture(uTexture, vUv); })";

static const char* clearFragSrc = R"(#version 330 core
in vec2 vUv;
uniform sampler2D uTexture;
uniform float value;
out vec4 FragColor;
void main() { FragColor = value * texture(uTexture, vUv); })";

static const char* splatFragSrc = R"(#version 330 core
in vec2 vUv;
uniform sampler2D uTarget;
uniform float aspectRatio;
uniform vec3 color;
uniform vec2 point;
uniform float radius;
out vec4 FragColor;
void main() {
	vec2 p = vUv - point;
	p.x *= aspectRatio;
	vec3 splat = exp(-dot(p, p) / radius) * color;
	vec3 base = texture(uTarget, vUv).xyz;
	FragColor = vec4(base + splat, 1.0);
})";

static const char* advectionFragSrc = R"(#version 330 core
in vec2 vUv;
uniform sampler2D uVelocity;
uniform sampler2D uSource;
uniform vec2 texelSize;
uniform float dt;
uniform float dissipation;
out vec4 FragColor;
void main() {
	vec2 coord = vUv - dt * texture(uVelocity, vUv).xy * texelSize;
	vec4 result = texture(uSource, coord);
	float decay = 1.0 + dissipation * dt;
	FragColor = result / decay;
})";

static const char* divergenceFragSrc = R"(#version 330 core
in vec2 vUv;
in vec2 vL;
in vec2 vR;
in vec2 vT;
in vec2 vB;
uniform sampler2D uVelocity;
out vec4 FragColor;
void main() {
	float L = texture(uVelocity, vL).x;
	float R = texture(uVelocity, vR).x;
	float T = texture(uVelocity, vT).y;
	float B = texture(uVelocity, vB).y;
	vec2 C = texture(uVelocity, vUv).xy;
	if (vL.x < 0.0) L = -C.x;
	if (vR.x > 1.0) R = -C.x;
	if (vT.y > 1.0) T = -C.y;
	if (vB.y < 0.0) B = -C.y;
	float div = 0.5 * (R - L + T - B);
	FragColor = vec4(div, 0.0, 0.0, 1.0);
})";

static const char* curlFragSrc = R"(#version 330 core
in vec2 vUv;
in vec2 vL;
in vec2 vR;
in vec2 vT;
in vec2 vB;
uniform sampler2D uVelocity;
out vec4 FragColor;
void main() {
	float L = texture(uVelocity, vL).y;
	float R = texture(uVelocity, vR).y;
	float T = texture(uVelocity, vT).x;
	float B = texture(uVelocity, vB).x;
	float vorticity = R - L - T + B;
	FragColor = vec4(0.5 * vorticity, 0.0, 0.0, 1.0);
})";

static const char* vorticityFragSrc = R"(#version 330 core
in vec2 vUv;
in vec2 vL;
in vec2 vR;
in vec2 vT;
in vec2 vB;
uniform sampler2D uVelocity;
uniform sampler2D uCurl;
uniform float curl;
uniform float dt;
out vec4 FragColor;
void main() {
	float L = texture(uCurl, vL).x;
	float R = texture(uCurl, vR).x;
	float T = texture(uCurl, vT).x;
	float B = texture(uCurl, vB).x;
	float C = texture(uCurl, vUv).x;
	vec2 force = 0.5 * vec2(abs(T) - abs(B), abs(R) - abs(L));
	force /= length(force) + 0.0001;
	force *= curl * C;
	force.y *= -1.0;
	vec2 velocity = texture(uVelocity, vUv).xy;
	velocity += force * dt;
	velocity = clamp(velocity, -1000.0, 1000.0);
	FragColor = vec4(velocity, 0.0, 1.0);
})";

static const char* pressureFragSrc = R"(#version 330 core
in vec2 vUv;
in vec2 vL;
in vec2 vR;
in vec2 vT;
in vec2 vB;
uniform sampler2D uPressure;
uniform sampler2D uDivergence;
out vec4 FragColor;
void main() {
	float L = texture(uPressure, vL).x;
	float R = texture(uPressure, vR).x;
	float T = texture(uPressure, vT).x;
	float B = texture(uPressure, vB).x;
	float div = texture(uDivergence, vUv).x;
	float pressure = (L + R + B + T - div) * 0.25;
	FragColor = vec4(pressure, 0.0, 0.0, 1.0);
})";

static const char* gradientSubFragSrc = R"(#version 330 core
in vec2 vUv;
in vec2 vL;
in vec2 vR;
in vec2 vT;
in vec2 vB;
uniform sampler2D uPressure;
uniform sampler2D uVelocity;
out vec4 FragColor;
void main() {
	float L = texture(uPressure, vL).x;
	float R = texture(uPressure, vR).x;
	float T = texture(uPressure, vT).x;
	float B = texture(uPressure, vB).x;
	vec2 velocity = texture(uVelocity, vUv).xy;
	velocity -= vec2(R - L, T - B);
	FragColor = vec4(velocity, 0.0, 1.0);
})";

static const char* displayFragSrc = R"(#version 330 core
in vec2 vUv;
in vec2 vL;
in vec2 vR;
in vec2 vT;
in vec2 vB;
uniform sampler2D uTexture;
uniform vec2 texelSize;
uniform bool shading;
out vec4 FragColor;
void main() {
	vec3 c = texture(uTexture, vUv).rgb;
	if (shading) {
		vec3 lc = texture(uTexture, vL).rgb;
		vec3 rc = texture(uTexture, vR).rgb;
		vec3 tc = texture(uTexture, vT).rgb;
		vec3 bc = texture(uTexture, vB).rgb;
		float dx = length(rc) - length(lc);
		float dy = length(tc) - length(bc);
		vec3 n = normalize(vec3(dx, dy, length(texelSize)));
		vec3 l = vec3(0.0, 0.0, 1.0);
		float diffuse = clamp(dot(n, l) + 0.7, 0.7, 1.0);
		c *= diffuse;
	}
	float a = max(c.r, max(c.g, c.b));
	FragColor = vec4(c, a);
})";

// ==================== SHARED GL RESOURCES (one context, safe to share) ====================
static GLuint g_quadVAO = 0, g_quadVBO = 0;
static GLuint g_progCopy = 0, g_progClear = 0, g_progSplat = 0, g_progAdvection = 0;
static GLuint g_progDivergence = 0, g_progCurl = 0, g_progVorticity = 0, g_progPressure = 0, g_progGradientSub = 0;
static GLuint g_progDisplay = 0;

static GLuint compileShader(GLenum type, const char* src) {
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);
	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char info[512];
		glGetShaderInfoLog(shader, 512, nullptr, info);
		std::cerr << "Shader compile error:\n" << info << std::endl;
	}
	return shader;
}

static GLuint createProgram(const char* vertSrc, const char* fragSrc) {
	GLuint vs = compileShader(GL_VERTEX_SHADER, vertSrc);
	GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragSrc);
	GLuint prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);
	glDeleteShader(vs);
	glDeleteShader(fs);
	return prog;
}

static void createQuad() {
	float vertices[] = { -1,-1, -1,1, 1,1, 1,-1 };
	glGenVertexArrays(1, &g_quadVAO);
	glGenBuffers(1, &g_quadVBO);
	glBindVertexArray(g_quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, g_quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBindVertexArray(0);
}

void InitSharedSimResources() {
	createQuad();
	g_progCopy = createProgram(vertexShaderSrc, copyFragSrc);
	g_progClear = createProgram(vertexShaderSrc, clearFragSrc);
	g_progSplat = createProgram(vertexShaderSrc, splatFragSrc);
	g_progAdvection = createProgram(vertexShaderSrc, advectionFragSrc);
	g_progDivergence = createProgram(vertexShaderSrc, divergenceFragSrc);
	g_progCurl = createProgram(vertexShaderSrc, curlFragSrc);
	g_progVorticity = createProgram(vertexShaderSrc, vorticityFragSrc);
	g_progPressure = createProgram(vertexShaderSrc, pressureFragSrc);
	g_progGradientSub = createProgram(vertexShaderSrc, gradientSubFragSrc);
	g_progDisplay = createProgram(vertexShaderSrc, displayFragSrc);
}

static void destroyTexture(Texture& t) {
	if (t.id) { glDeleteTextures(1, &t.id); t.id = 0; }
	t.width = t.height = 0;
}

static void destroyFBO(FBO& f) {
	if (f.fbo) { glDeleteFramebuffers(1, &f.fbo); f.fbo = 0; }
	destroyTexture(f.tex);
}

static void destroyDoubleFBO(DoubleFBO& d) {
	destroyFBO(d.read);
	destroyFBO(d.write);
	d.width = d.height = 0;
}

void DestroySharedSimResources() {
	auto delProg = [](GLuint& p) { if (p) { glDeleteProgram(p); p = 0; } };
	delProg(g_progCopy);
	delProg(g_progClear);
	delProg(g_progSplat);
	delProg(g_progAdvection);
	delProg(g_progDivergence);
	delProg(g_progCurl);
	delProg(g_progVorticity);
	delProg(g_progPressure);
	delProg(g_progGradientSub);
	delProg(g_progDisplay);
	if (g_quadVAO) { glDeleteVertexArrays(1, &g_quadVAO); g_quadVAO = 0; }
	if (g_quadVBO) { glDeleteBuffers(1, &g_quadVBO); g_quadVBO = 0; }
}

// ==================== FBO HELPERS (per-instance state) ====================
static Texture createTexture(int w, int h, GLenum internalFormat, GLenum format, GLenum type, GLenum filter) {
	Texture tex;
	tex.width = w; tex.height = h;
	tex.texelSizeX = 1.0f / w; tex.texelSizeY = 1.0f / h;
	glGenTextures(1, &tex.id);
	glBindTexture(GL_TEXTURE_2D, tex.id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, type, nullptr);
	return tex;
}

static FBO createFBO(int w, int h, GLenum internalFormat, GLenum format, GLenum type, GLenum filter) {
	FBO fbo;
	fbo.tex = createTexture(w, h, internalFormat, format, type, filter);
	glGenFramebuffers(1, &fbo.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo.fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo.tex.id, 0);
	glViewport(0, 0, w, h);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return fbo;
}

static DoubleFBO createDoubleFBO(int w, int h, GLenum internalFormat, GLenum format, GLenum type, GLenum filter) {
	DoubleFBO dfbo;
	dfbo.width = w; dfbo.height = h;
	dfbo.texelSizeX = 1.0f / w; dfbo.texelSizeY = 1.0f / h;
	dfbo.read = createFBO(w, h, internalFormat, format, type, filter);
	dfbo.write = createFBO(w, h, internalFormat, format, type, filter);
	return dfbo;
}

static void resizeDoubleFBO(DoubleFBO& dfbo, int w, int h, GLenum internalFormat, GLenum format, GLenum type, GLenum filter) {
	if (dfbo.width == w && dfbo.height == h) return;
	dfbo.read = createFBO(w, h, internalFormat, format, type, filter);
	dfbo.write = createFBO(w, h, internalFormat, format, type, filter);
	dfbo.width = w; dfbo.height = h;
	dfbo.texelSizeX = 1.0f / w; dfbo.texelSizeY = 1.0f / h;
}

static void blit(const FBO* target, bool clear = false) {
	if (target) {
		glViewport(0, 0, target->tex.width, target->tex.height);
		glBindFramebuffer(GL_FRAMEBUFFER, target->fbo);
	}
	if (clear) {
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	glBindVertexArray(g_quadVAO);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindVertexArray(0);
}

// ==================== COLOR HELPER ====================
static Color hsvToRgb(float h, float s, float v) {
	int i = int(h * 6);
	float f = h * 6 - i;
	float p = v * (1 - s);
	float q = v * (1 - f * s);
	float t = v * (1 - (1 - f) * s);
	switch (i % 6) {
	case 0: return { v, t, p };
	case 1: return { q, v, p };
	case 2: return { p, v, t };
	case 3: return { p, q, v };
	case 4: return { t, p, v };
	default: return { v, p, q };
	}
}

// ==================== FluidSim ====================
void FluidSim::ComputeSizes(int viewportW, int viewportH) {
	float aspect = (float)viewportW / (float)viewportH;
	if (aspect < 1) {
		m_simW = g_config.simRes; m_simH = (int)(g_config.simRes / aspect);
		m_dyeW = g_config.dyeRes; m_dyeH = (int)(g_config.dyeRes / aspect);
	}
	else {
		m_simW = (int)(g_config.simRes * aspect); m_simH = g_config.simRes;
		m_dyeW = (int)(g_config.dyeRes * aspect); m_dyeH = g_config.dyeRes;
	}
	m_simW = ((m_simW + 7) / 8) * 8; m_simH = ((m_simH + 7) / 8) * 8;
	m_dyeW = ((m_dyeW + 7) / 8) * 8; m_dyeH = ((m_dyeH + 7) / 8) * 8;
	m_texelX = 1.0f / m_simW; m_texelY = 1.0f / m_simH;
	m_dyeTexelX = 1.0f / m_dyeW; m_dyeTexelY = 1.0f / m_dyeH;
}

FluidSim::~FluidSim() {
	destroyDoubleFBO(m_dye);
	destroyDoubleFBO(m_velocity);
	destroyFBO(m_divergence);
	destroyFBO(m_curl);
	destroyDoubleFBO(m_pressure);
	m_initialized = false;
}

void FluidSim::Init(int viewportW, int viewportH) {
	m_viewportW = viewportW; m_viewportH = viewportH;
	ComputeSizes(viewportW, viewportH);

	GLenum texType = GL_HALF_FLOAT;
	GLenum filter = GL_LINEAR;

	m_dye = createDoubleFBO(m_dyeW, m_dyeH, GL_RGBA16F, GL_RGBA, texType, filter);
	m_velocity = createDoubleFBO(m_simW, m_simH, GL_RG16F, GL_RG, texType, filter);
	m_divergence = createFBO(m_simW, m_simH, GL_R16F, GL_RED, texType, GL_NEAREST);
	m_curl = createFBO(m_simW, m_simH, GL_R16F, GL_RED, texType, GL_NEAREST);
	m_pressure = createDoubleFBO(m_simW, m_simH, GL_R16F, GL_RED, texType, GL_NEAREST);

	m_initialized = true;
	m_lastBurst = std::chrono::steady_clock::now();
	MultiSplat(12);
}

void FluidSim::Resize(int viewportW, int viewportH) {
	if (!m_initialized) { Init(viewportW, viewportH); return; }
	if (m_viewportW == viewportW && m_viewportH == viewportH) return;
	m_viewportW = viewportW; m_viewportH = viewportH;
	ComputeSizes(viewportW, viewportH);

	GLenum texType = GL_HALF_FLOAT;
	GLenum filter = GL_LINEAR;
	resizeDoubleFBO(m_dye, m_dyeW, m_dyeH, GL_RGBA16F, GL_RGBA, texType, filter);
	resizeDoubleFBO(m_velocity, m_simW, m_simH, GL_RG16F, GL_RG, texType, filter);
	m_divergence = createFBO(m_simW, m_simH, GL_R16F, GL_RED, texType, GL_NEAREST);
	m_curl = createFBO(m_simW, m_simH, GL_R16F, GL_RED, texType, GL_NEAREST);
	m_pressure = createDoubleFBO(m_simW, m_simH, GL_R16F, GL_RED, texType, GL_NEAREST);
}

void FluidSim::Step(float dt) {
	glDisable(GL_BLEND);

	glUseProgram(g_progCurl);
	glUniform2f(glGetUniformLocation(g_progCurl, "texelSize"), m_texelX, m_texelY);
	glUniform1i(glGetUniformLocation(g_progCurl, "uVelocity"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_velocity.read.tex.id);
	blit(&m_curl);

	glUseProgram(g_progVorticity);
	glUniform2f(glGetUniformLocation(g_progVorticity, "texelSize"), m_texelX, m_texelY);
	glUniform1i(glGetUniformLocation(g_progVorticity, "uVelocity"), 0);
	glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, m_velocity.read.tex.id);
	glUniform1i(glGetUniformLocation(g_progVorticity, "uCurl"), 1);
	glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, m_curl.tex.id);
	glUniform1f(glGetUniformLocation(g_progVorticity, "curl"), g_config.curl);
	glUniform1f(glGetUniformLocation(g_progVorticity, "dt"), dt);
	blit(&m_velocity.write);
	m_velocity.swap();

	glUseProgram(g_progDivergence);
	glUniform2f(glGetUniformLocation(g_progDivergence, "texelSize"), m_texelX, m_texelY);
	glUniform1i(glGetUniformLocation(g_progDivergence, "uVelocity"), 0);
	glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, m_velocity.read.tex.id);
	blit(&m_divergence);

	glUseProgram(g_progClear);
	glUniform1i(glGetUniformLocation(g_progClear, "uTexture"), 0);
	glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, m_pressure.read.tex.id);
	glUniform1f(glGetUniformLocation(g_progClear, "value"), g_config.pressure);
	blit(&m_pressure.write);
	m_pressure.swap();

	glUseProgram(g_progPressure);
	glUniform2f(glGetUniformLocation(g_progPressure, "texelSize"), m_texelX, m_texelY);
	glUniform1i(glGetUniformLocation(g_progPressure, "uDivergence"), 0);
	glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, m_divergence.tex.id);
	for (int i = 0; i < g_config.pressureIter; ++i) {
		glUniform1i(glGetUniformLocation(g_progPressure, "uPressure"), 1);
		glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, m_pressure.read.tex.id);
		blit(&m_pressure.write);
		m_pressure.swap();
	}

	glUseProgram(g_progGradientSub);
	glUniform2f(glGetUniformLocation(g_progGradientSub, "texelSize"), m_texelX, m_texelY);
	glUniform1i(glGetUniformLocation(g_progGradientSub, "uPressure"), 0);
	glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, m_pressure.read.tex.id);
	glUniform1i(glGetUniformLocation(g_progGradientSub, "uVelocity"), 1);
	glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, m_velocity.read.tex.id);
	blit(&m_velocity.write);
	m_velocity.swap();

	glUseProgram(g_progAdvection);
	glUniform2f(glGetUniformLocation(g_progAdvection, "texelSize"), m_texelX, m_texelY);
	glUniform1i(glGetUniformLocation(g_progAdvection, "uVelocity"), 0);
	glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, m_velocity.read.tex.id);
	glUniform1i(glGetUniformLocation(g_progAdvection, "uSource"), 0);
	glUniform1f(glGetUniformLocation(g_progAdvection, "dt"), dt);
	glUniform1f(glGetUniformLocation(g_progAdvection, "dissipation"), g_config.velDiss);
	blit(&m_velocity.write);
	m_velocity.swap();

	glUseProgram(g_progAdvection);
	glUniform2f(glGetUniformLocation(g_progAdvection, "texelSize"), m_dyeTexelX, m_dyeTexelY);
	glUniform1i(glGetUniformLocation(g_progAdvection, "uVelocity"), 0);
	glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, m_velocity.read.tex.id);
	glUniform1i(glGetUniformLocation(g_progAdvection, "uSource"), 1);
	glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, m_dye.read.tex.id);
	glUniform1f(glGetUniformLocation(g_progAdvection, "dt"), dt);
	glUniform1f(glGetUniformLocation(g_progAdvection, "dissipation"), g_config.denDiss);
	blit(&m_dye.write);
	m_dye.swap();

	glEnable(GL_BLEND);
}

void FluidSim::Render(int vx, int vy, int vw, int vh) {
	glDisable(GL_BLEND);
	glUseProgram(g_progDisplay);
	glUniform1i(glGetUniformLocation(g_progDisplay, "uTexture"), 0);
	glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, m_dye.read.tex.id);
	glUniform2f(glGetUniformLocation(g_progDisplay, "texelSize"), 1.0f / m_dyeW, 1.0f / m_dyeH);
	glUniform1i(glGetUniformLocation(g_progDisplay, "shading"), g_config.shading ? 1 : 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(vx, vy, vw, vh);
	glBindVertexArray(g_quadVAO);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindVertexArray(0);
}

void FluidSim::SplatInternal(float x, float y, float dx, float dy, Color color, float amount) {
	float aspect = 1.0f; // textures are square
	float radius = g_config.splatRadius / 100.0f;

	glUseProgram(g_progSplat);
	glUniform1i(glGetUniformLocation(g_progSplat, "uTarget"), 0);
	glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, m_velocity.read.tex.id);
	glUniform1f(glGetUniformLocation(g_progSplat, "aspectRatio"), aspect);
	glUniform2f(glGetUniformLocation(g_progSplat, "point"), x, y);
	glUniform3f(glGetUniformLocation(g_progSplat, "color"), dx * g_config.splatForce, dy * g_config.splatForce, 0.0f);
	glUniform1f(glGetUniformLocation(g_progSplat, "radius"), radius);
	blit(&m_velocity.write);
	m_velocity.swap();

	glUseProgram(g_progSplat);
	glUniform1i(glGetUniformLocation(g_progSplat, "uTarget"), 0);
	glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, m_dye.read.tex.id);
	glUniform1f(glGetUniformLocation(g_progSplat, "aspectRatio"), aspect);
	glUniform2f(glGetUniformLocation(g_progSplat, "point"), x, y);
	glUniform3f(glGetUniformLocation(g_progSplat, "color"), color.r * amount, color.g * amount, color.b * amount);
	glUniform1f(glGetUniformLocation(g_progSplat, "radius"), radius);
	blit(&m_dye.write);
	m_dye.swap();
}

void FluidSim::Splat(float x, float y, float dx, float dy, Color color, float amount) {
	SplatInternal(x, y, dx, dy, color, amount);
}

Color FluidSim::PickSplatColor() {
	switch (g_config.colorMode) {
	case ColorMode::Single:
		return HexToColor(g_config.colorSingleHex);
	case ColorMode::Range: {
		Color a = HexToColor(g_config.colorRangeStartHex);
		Color b = HexToColor(g_config.colorRangeEndHex);
		return LerpColor(a, b, m_dist01(m_rng));
	}
	case ColorMode::Rainbow:
	default:
		return hsvToRgb(m_dist01(m_rng), 1.0f, 1.0f);
	}
}

void FluidSim::QueueSplat(float x, float y, float dx, float dy, Color color, float amount) {
	// Drop oldest pending splats when the queue exceeds the cap to prevent
	// unbounded memory growth (e.g. when the simulation is paused while bursts
	// keep queuing new splats).
	while (m_pending.size() >= kMaxPendingSplats)
		m_pending.erase(m_pending.begin());

	PendingSplat p;
	p.x = x; p.y = y; p.dx = dx; p.dy = dy; p.color = color;
	p.totalAmount = amount;
	p.durationMs = std::max(16.0f, g_config.smoothSpawnMs);
	p.elapsedMs = 0.0f;
	p.injectedFraction = 0.0f;
	m_pending.push_back(p);
}

void FluidSim::AdvancePending(float dt) {
	if (m_pending.empty()) return;
	float dtMs = dt * 1000.0f;
	for (auto it = m_pending.begin(); it != m_pending.end(); ) {
		PendingSplat& p = *it;
		float newElapsed = p.elapsedMs + dtMs;
		float newFraction = std::min(1.0f, newElapsed / p.durationMs);
		float deltaFraction = newFraction - p.injectedFraction;
		if (deltaFraction > 0.0f) {
			// Ease-out ramp: inject faster at first, tapering off, so bursts feel organic
			// rather than a linear brightness ramp.
			SplatInternal(p.x, p.y, p.dx * deltaFraction, p.dy * deltaFraction, p.color, p.totalAmount * deltaFraction);
			p.injectedFraction = newFraction;
		}
		p.elapsedMs = newElapsed;
		if (newFraction >= 1.0f) it = m_pending.erase(it);
		else ++it;
	}
}

void FluidSim::MultiSplat(int count, float amountScale) {
	for (int i = 0; i < count; ++i) {
		float x = m_dist01(m_rng), y = m_dist01(m_rng);
		float dx = m_dist01(m_rng) - 0.5f;
		float dy = m_dist01(m_rng) - 0.5f;
		Color c = PickSplatColor();
		float amount = g_config.dyeAmount * amountScale;
		if (g_config.smoothSpawn) QueueSplat(x, y, dx, dy, c, amount);
		else SplatInternal(x, y, dx, dy, c, amount);
	}
}

void FluidSim::ClearAll() {
	m_pending.clear();

	glUseProgram(g_progClear);
	glUniform1i(glGetUniformLocation(g_progClear, "uTexture"), 0);
	glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, m_dye.read.tex.id);
	glUniform1f(glGetUniformLocation(g_progClear, "value"), 0.0f);
	blit(&m_dye.write);
	m_dye.swap();

	glUseProgram(g_progClear);
	glUniform1i(glGetUniformLocation(g_progClear, "uTexture"), 0);
	glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, m_velocity.read.tex.id);
	glUniform1f(glGetUniformLocation(g_progClear, "value"), 0.0f);
	blit(&m_velocity.write);
	m_velocity.swap();
}

void FluidSim::Update(float dt) {
	if (!m_initialized) return;

	if (!paused && g_config.autoBurst) {
		auto now = std::chrono::steady_clock::now();
		float elapsed = std::chrono::duration<float, std::milli>(now - m_lastBurst).count();
		if (elapsed > g_config.autoIntervalMs) {
			MultiSplat(2 + rand() % 4, 1.4f);
			m_lastBurst = now;
		}
	}

	AdvancePending(dt);

	if (!paused) Step(dt);
}
