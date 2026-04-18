#include "GLViewportFramebuffer.h"
#include "../core/Logger.h"

// The GL FBO implementation requires an OpenGL context and GL extension
// function pointers.  On platforms where GL is not available, the methods
// are safe no-ops so that the class can still be instantiated (e.g. in
// headless tests) without linker errors.

#if defined(ATLAS_HAS_X11) || defined(ATLAS_HAS_WIN32) || defined(__APPLE__) || defined(_WIN32)
#define ATLAS_HAS_GL_FBO 1
#else
#define ATLAS_HAS_GL_FBO 0
#endif

#if ATLAS_HAS_GL_FBO

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <GL/gl.h>

// --- GL FBO extension constants ---
#ifndef GL_FRAMEBUFFER
#define GL_FRAMEBUFFER                   0x8D40
#define GL_COLOR_ATTACHMENT0             0x8CE0
#define GL_DEPTH_ATTACHMENT              0x8D00
#define GL_RENDERBUFFER                  0x8D41
#define GL_DEPTH_COMPONENT24             0x81A6
#define GL_FRAMEBUFFER_COMPLETE          0x8CD5
#define GL_READ_FRAMEBUFFER              0x8CA8
#define GL_DRAW_FRAMEBUFFER              0x8CA9
#endif

// --- GL FBO extension function pointer types ---
using PFNGLGENFRAMEBUFFERSPROC     = void (*)(int n, unsigned int* framebuffers);
using PFNGLDELETEFRAMEBUFFERSPROC  = void (*)(int n, const unsigned int* framebuffers);
using PFNGLBINDFRAMEBUFFERPROC     = void (*)(unsigned int target, unsigned int framebuffer);
using PFNGLFRAMEBUFFERTEXTURE2DPROC = void (*)(unsigned int target, unsigned int attachment,
                                                unsigned int textarget, unsigned int texture, int level);
using PFNGLGENRENDERBUFFERSPROC    = void (*)(int n, unsigned int* renderbuffers);
using PFNGLDELETERENDERBUFFERSPROC = void (*)(int n, const unsigned int* renderbuffers);
using PFNGLBINDRENDERBUFFERPROC    = void (*)(unsigned int target, unsigned int renderbuffer);
using PFNGLRENDERBUFFERSTORAGEPROC = void (*)(unsigned int target, unsigned int internalformat,
                                               int width, int height);
using PFNGLFRAMEBUFFERRENDERBUFFERPROC = void (*)(unsigned int target, unsigned int attachment,
                                                    unsigned int renderbuffertarget,
                                                    unsigned int renderbuffer);
using PFNGLCHECKFRAMEBUFFERSTATUSPROC = unsigned int (*)(unsigned int target);

// Function pointers (resolved once at runtime)
static PFNGLGENFRAMEBUFFERSPROC          pglGenFramebuffers          = nullptr;
static PFNGLDELETEFRAMEBUFFERSPROC       pglDeleteFramebuffers       = nullptr;
static PFNGLBINDFRAMEBUFFERPROC          pglBindFramebuffer          = nullptr;
static PFNGLFRAMEBUFFERTEXTURE2DPROC     pglFramebufferTexture2D     = nullptr;
static PFNGLGENRENDERBUFFERSPROC         pglGenRenderbuffers         = nullptr;
static PFNGLDELETERENDERBUFFERSPROC      pglDeleteRenderbuffers      = nullptr;
static PFNGLBINDRENDERBUFFERPROC         pglBindRenderbuffer         = nullptr;
static PFNGLRENDERBUFFERSTORAGEPROC      pglRenderbufferStorage      = nullptr;
static PFNGLFRAMEBUFFERRENDERBUFFERPROC  pglFramebufferRenderbuffer  = nullptr;
static PFNGLCHECKFRAMEBUFFERSTATUSPROC   pglCheckFramebufferStatus   = nullptr;

// Platform-specific function loader
#if defined(__linux__)
extern "C" void* dlsym(void*, const char*);
#ifndef RTLD_DEFAULT
#define RTLD_DEFAULT ((void*)0)
#endif
#endif

static void* GLGetProcAddress(const char* name) {
#if defined(__linux__)
    return dlsym(RTLD_DEFAULT, name);
#elif defined(_WIN32)
    // wglGetProcAddress is already declared via windows.h/wingdi.h and returns
    // PROC (a FARPROC alias).  Cast the result to void* for our generic loader.
    return reinterpret_cast<void*>(wglGetProcAddress(name));
#else
    (void)name;
    return nullptr;
#endif
}

#endif // ATLAS_HAS_GL_FBO

namespace atlas::render {

GLViewportFramebuffer::~GLViewportFramebuffer() {
    Destroy();
}

bool GLViewportFramebuffer::ResolveExtensions() {
    if (m_extensionsResolved) return true;

#if ATLAS_HAS_GL_FBO
    pglGenFramebuffers         = reinterpret_cast<PFNGLGENFRAMEBUFFERSPROC>(GLGetProcAddress("glGenFramebuffers"));
    pglDeleteFramebuffers      = reinterpret_cast<PFNGLDELETEFRAMEBUFFERSPROC>(GLGetProcAddress("glDeleteFramebuffers"));
    pglBindFramebuffer         = reinterpret_cast<PFNGLBINDFRAMEBUFFERPROC>(GLGetProcAddress("glBindFramebuffer"));
    pglFramebufferTexture2D    = reinterpret_cast<PFNGLFRAMEBUFFERTEXTURE2DPROC>(GLGetProcAddress("glFramebufferTexture2D"));
    pglGenRenderbuffers        = reinterpret_cast<PFNGLGENRENDERBUFFERSPROC>(GLGetProcAddress("glGenRenderbuffers"));
    pglDeleteRenderbuffers     = reinterpret_cast<PFNGLDELETERENDERBUFFERSPROC>(GLGetProcAddress("glDeleteRenderbuffers"));
    pglBindRenderbuffer        = reinterpret_cast<PFNGLBINDRENDERBUFFERPROC>(GLGetProcAddress("glBindRenderbuffer"));
    pglRenderbufferStorage     = reinterpret_cast<PFNGLRENDERBUFFERSTORAGEPROC>(GLGetProcAddress("glRenderbufferStorage"));
    pglFramebufferRenderbuffer = reinterpret_cast<PFNGLFRAMEBUFFERRENDERBUFFERPROC>(GLGetProcAddress("glFramebufferRenderbuffer"));
    pglCheckFramebufferStatus  = reinterpret_cast<PFNGLCHECKFRAMEBUFFERSTATUSPROC>(GLGetProcAddress("glCheckFramebufferStatus"));

    m_extensionsResolved = (pglGenFramebuffers && pglDeleteFramebuffers &&
                            pglBindFramebuffer && pglFramebufferTexture2D &&
                            pglGenRenderbuffers && pglDeleteRenderbuffers &&
                            pglBindRenderbuffer && pglRenderbufferStorage &&
                            pglFramebufferRenderbuffer && pglCheckFramebufferStatus);

    if (!m_extensionsResolved) {
        Logger::Error("GLViewportFramebuffer: failed to resolve GL FBO extensions");
    }
#else
    Logger::Error("GLViewportFramebuffer: GL not available on this platform");
    m_extensionsResolved = false;
#endif

    return m_extensionsResolved;
}

bool GLViewportFramebuffer::Create(uint32_t width, uint32_t height) {
    if (width == 0 || height == 0) return false;

    Destroy();

    if (!ResolveExtensions()) return false;

#if ATLAS_HAS_GL_FBO
    // Create FBO
    pglGenFramebuffers(1, &m_fbo);
    pglBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Create color attachment texture
    glGenTextures(1, &m_colorTexture);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<int>(width),
                 static_cast<int>(height), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    pglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                            GL_TEXTURE_2D, m_colorTexture, 0);

    // Create depth renderbuffer
    pglGenRenderbuffers(1, &m_depthRbo);
    pglBindRenderbuffer(GL_RENDERBUFFER, m_depthRbo);
    pglRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
                           static_cast<int>(width), static_cast<int>(height));
    pglFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                               GL_RENDERBUFFER, m_depthRbo);

    // Check completeness
    unsigned int status = pglCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        Logger::Error("GLViewportFramebuffer: framebuffer incomplete (status=" +
                      std::to_string(status) + ")");
        Destroy();
        pglBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }

    m_width = width;
    m_height = height;

    // Restore default framebuffer
    pglBindFramebuffer(GL_FRAMEBUFFER, 0);

    Logger::Info("GLViewportFramebuffer: created " + std::to_string(width) +
                 "x" + std::to_string(height) + " FBO");
    return true;
#else
    (void)width;
    (void)height;
    return false;
#endif
}

void GLViewportFramebuffer::Resize(uint32_t width, uint32_t height) {
    if (width == m_width && height == m_height) return;
    if (width == 0 || height == 0) {
        Destroy();
        m_width = 0;
        m_height = 0;
        return;
    }
    Create(width, height);
}

void GLViewportFramebuffer::Bind() {
#if ATLAS_HAS_GL_FBO
    if (m_fbo && pglBindFramebuffer) {
        pglBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glViewport(0, 0, static_cast<int>(m_width), static_cast<int>(m_height));
    }
#endif
}

void GLViewportFramebuffer::Unbind() {
#if ATLAS_HAS_GL_FBO
    if (pglBindFramebuffer) {
        pglBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
#endif
}

void GLViewportFramebuffer::Destroy() {
#if ATLAS_HAS_GL_FBO
    if (m_fbo && pglDeleteFramebuffers) {
        pglDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }
    if (m_colorTexture) {
        glDeleteTextures(1, &m_colorTexture);
        m_colorTexture = 0;
    }
    if (m_depthRbo && pglDeleteRenderbuffers) {
        pglDeleteRenderbuffers(1, &m_depthRbo);
        m_depthRbo = 0;
    }
#else
    m_fbo = 0;
    m_colorTexture = 0;
    m_depthRbo = 0;
#endif
}

} // namespace atlas::render
