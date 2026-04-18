#pragma once
// NF::SoftwareViewportRenderer — Software rasterizer for viewports.
//
// Draws a visible grid + background into any IViewportSurface that provides
// writable scanline access (currently GDIViewportSurface).
//
// When scene entities are present in the ViewportSceneState, the renderer
// also projects each entity's world position through the slot camera and
// draws a screen-space wireframe box and an optional centre cross marker.
// This proves the full data pipeline:
//   NovaForgePreviewWorld → provideScene() → ViewportSceneState::entities
//   → SoftwareViewportRenderer → IViewportSurface::writeScanlines → screen.
//
// This is NOT a production renderer — it is a diagnostic and bootstrap tool.
// Replace with real GPU rendering when D3D11/OpenGL is activated.

#include "NF/Workspace/ViewportHostContract.h"
#include "NF/Workspace/IViewportSurface.h"
#include "NF/Workspace/IViewportSceneProvider.h"
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace NF {

class SoftwareViewportRenderer {
public:
    // ── Background and grid colours (BGRA format) ─────────────────────────────
    uint32_t bgColor       = 0xFF1C1C1C;  // dark grey
    uint32_t gridColor     = 0xFF3A3A3A;  // lighter grey
    uint32_t axisXColor    = 0xFF4444FF;  // blue  (BGRA: B=FF, G=44, R=44)
    uint32_t axisYColor    = 0xFF44FF44;  // green (BGRA: B=44, G=FF, R=44)
    uint32_t entityColor   = 0xFF00AAFF;  // orange-ish: entity wireframe
    uint32_t selectColor   = 0xFF00FF88;  // bright green: selected entity
    uint32_t gridSpacing   = 32;          // pixels between grid lines

    // ── renderGrid ────────────────────────────────────────────────────────────
    /// Render the background grid and any scene entities into the surface.
    ///
    /// Entities in `scene.entities` are projected through the camera stored in
    /// `slot` to compute a screen-space 2D position; a cross marker and a
    /// projected wireframe quad are then painted at that position.
    void renderGrid(IViewportSurface&        surface,
                    const ViewportSlot&      slot,
                    const ViewportSceneState& scene)
    {
        uint32_t w = surface.width();
        uint32_t h = surface.height();
        if (w == 0 || h == 0) return;

        ++m_renderCount;
        m_lastWidth  = w;
        m_lastHeight = h;

        m_buffer.resize(static_cast<size_t>(w) * h);

        // 1. Fill background grid.
        fillGrid(m_buffer.data(), w, h, slot);

        // 2. Project and draw entities when the scene has content.
        if (scene.hasContent && !scene.entities.empty()) {
            Camera cam = buildCamera(slot, w, h);
            for (const auto& proxy : scene.entities)
                drawEntity(m_buffer.data(), w, h, proxy, cam);
        }

        // 3. Push pixels into the surface's backing store.
        surface.writeScanlines(m_buffer.data(), m_buffer.size());
    }

    // ── Accessors ─────────────────────────────────────────────────────────────
    [[nodiscard]] const uint32_t* pixels()      const { return m_buffer.data(); }
    [[nodiscard]] size_t          pixelCount()  const { return m_buffer.size(); }
    [[nodiscard]] uint32_t        renderCount() const { return m_renderCount; }
    [[nodiscard]] uint32_t        lastWidth()   const { return m_lastWidth; }
    [[nodiscard]] uint32_t        lastHeight()  const { return m_lastHeight; }

    // ── projectPoint ──────────────────────────────────────────────────────────
    /// Project a world-space point to screen space using the camera built from
    /// `slot`.  Returns true if the point is in front of the camera; in that
    /// case `sx` and `sy` are the (possibly out-of-bounds) pixel coordinates.
    /// Returns false (and leaves sx/sy unchanged) if the point is behind or
    /// at the camera plane.
    bool projectPoint(const ViewportSlot& slot,
                      uint32_t viewW, uint32_t viewH,
                      float wx, float wy, float wz,
                      float& sx, float& sy) const
    {
        if (viewW == 0 || viewH == 0) return false;
        Camera cam = buildCamera(slot, viewW, viewH);
        float vx, vy, vz;
        worldToView(cam, wx, wy, wz, vx, vy, vz);
        if (vz <= 0.f) return false;
        viewToScreen(cam, vx, vy, vz, sx, sy);
        return true;
    }

private:
    // ── Camera ────────────────────────────────────────────────────────────────
    struct Camera {
        // Camera world position
        float cx = 0.f, cy = 0.f, cz = 0.f;
        // Right, up, forward axes (derived from yaw/pitch)
        float rx = 1.f, ry = 0.f, rz = 0.f;
        float ux = 0.f, uy = 1.f, uz = 0.f;
        float fx = 0.f, fy = 0.f, fz = -1.f;
        // Projection
        float halfFovTan = 1.f;   // tan(fovY/2)
        float aspect     = 1.f;
        float sw = 1.f, sh = 1.f; // screen width/height
    };

    static Camera buildCamera(const ViewportSlot& slot,
                              uint32_t w, uint32_t h)
    {
        Camera cam;
        cam.cx = slot.camera.position.x;
        cam.cy = slot.camera.position.y;
        cam.cz = slot.camera.position.z;
        cam.sw = static_cast<float>(w);
        cam.sh = static_cast<float>(h);
        cam.aspect = cam.sh > 0.f ? cam.sw / cam.sh : 1.f;

        float fovRad   = static_cast<float>(slot.camera.fovDegrees * M_PI / 180.0);
        cam.halfFovTan = std::tan(fovRad * 0.5f);

        float yawRad   = static_cast<float>(slot.camera.yaw   * M_PI / 180.0);
        float pitchRad = static_cast<float>(slot.camera.pitch * M_PI / 180.0);

        float cosY = std::cos(yawRad),   sinY = std::sin(yawRad);
        float cosP = std::cos(pitchRad), sinP = std::sin(pitchRad);

        // Forward = direction the camera is looking
        cam.fx = cosY * cosP;
        cam.fy = sinP;
        cam.fz = sinY * cosP;

        // Right = forward × worldUp (with worldUp = (0,1,0))
        // right = normalize(forward × (0,1,0))
        cam.rx = -cam.fz; cam.ry = 0.f; cam.rz = cam.fx;
        float rLen = std::sqrt(cam.rx*cam.rx + cam.ry*cam.ry + cam.rz*cam.rz);
        if (rLen > 1e-6f) { cam.rx /= rLen; cam.ry /= rLen; cam.rz /= rLen; }

        // Up = right × forward
        cam.ux = cam.ry*cam.fz - cam.rz*cam.fy;
        cam.uy = cam.rz*cam.fx - cam.rx*cam.fz;
        cam.uz = cam.rx*cam.fy - cam.ry*cam.fx;
        float uLen = std::sqrt(cam.ux*cam.ux + cam.uy*cam.uy + cam.uz*cam.uz);
        if (uLen > 1e-6f) { cam.ux /= uLen; cam.uy /= uLen; cam.uz /= uLen; }

        return cam;
    }

    /// Transform a world point to camera (view) space.
    static void worldToView(const Camera& cam,
                            float wx, float wy, float wz,
                            float& vx, float& vy, float& vz)
    {
        float dx = wx - cam.cx, dy = wy - cam.cy, dz = wz - cam.cz;
        vx =  dx*cam.rx + dy*cam.ry + dz*cam.rz;
        vy =  dx*cam.ux + dy*cam.uy + dz*cam.uz;
        vz =  dx*cam.fx + dy*cam.fy + dz*cam.fz;
    }

    /// Project a view-space point to screen pixel coordinates.
    /// Caller must have verified vz > 0 beforehand.
    static void viewToScreen(const Camera& cam,
                             float vx, float vy, float vz,
                             float& sx, float& sy)
    {
        // Perspective divide: ndcX in [-aspect, aspect] when vx/vz == ±halfFovTan
        float ndcX =  vx / (vz * cam.halfFovTan * cam.aspect);
        float ndcY = -vy / (vz * cam.halfFovTan);   // Y flipped: screen Y grows down
        sx = (ndcX * 0.5f + 0.5f) * cam.sw;
        sy = (ndcY * 0.5f + 0.5f) * cam.sh;
    }

    // ── Drawing helpers ───────────────────────────────────────────────────────

    static void setPixelSafe(uint32_t* buf, uint32_t w, uint32_t h,
                             int px, int py, uint32_t color)
    {
        if (px < 0 || py < 0) return;
        if (static_cast<uint32_t>(px) >= w || static_cast<uint32_t>(py) >= h) return;
        buf[static_cast<uint32_t>(py) * w + static_cast<uint32_t>(px)] = color;
    }

    /// Bresenham line between two screen pixels.
    static void drawLine(uint32_t* buf, uint32_t w, uint32_t h,
                         int x0, int y0, int x1, int y1, uint32_t color)
    {
        int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = (dx > dy ? dx : -dy) / 2;
        for (;;) {
            setPixelSafe(buf, w, h, x0, y0, color);
            if (x0 == x1 && y0 == y1) break;
            int e2 = err;
            if (e2 > -dx) { err -= dy; x0 += sx; }
            if (e2 <  dy) { err += dx; y0 += sy; }
        }
    }

    /// Draw a cross marker at (cx, cy) with arm length `arm`.
    static void drawCross(uint32_t* buf, uint32_t w, uint32_t h,
                          int cx, int cy, int arm, uint32_t color)
    {
        drawLine(buf, w, h, cx - arm, cy, cx + arm, cy, color);
        drawLine(buf, w, h, cx, cy - arm, cx, cy + arm, color);
    }

    /// Project one entity and draw its screen-space marker.
    void drawEntity(uint32_t* buf, uint32_t w, uint32_t h,
                    const ViewportEntityProxy& proxy, const Camera& cam) const
    {
        float vx, vy, vz;
        worldToView(cam, proxy.x, proxy.y, proxy.z, vx, vy, vz);
        if (vz <= 0.f) return;  // behind camera

        float sx, sy;
        viewToScreen(cam, vx, vy, vz, sx, sy);

        int cx = static_cast<int>(sx);
        int cy = static_cast<int>(sy);
        uint32_t color = proxy.selected ? selectColor : entityColor;

        // Centre cross
        drawCross(buf, w, h, cx, cy, 6, color);

        // Project the 8 corners of the bounding box and draw a simplified
        // screen-space rect (project just the ±X/±Y corners at this Z depth,
        // giving a reliable rectangle even for near/far entity distances).
        float hW = proxy.halfW > 0.f ? proxy.halfW : 0.5f;
        float hH = proxy.halfH > 0.f ? proxy.halfH : 0.5f;

        // We project 4 corners using only the right/up axes (screen-aligned
        // bounding quad — simple but consistent with the software-render goal).
        struct Corner { float dvx, dvy; };
        Corner corners[4] = {
            {-hW, -hH}, { hW, -hH},
            { hW,  hH}, {-hW,  hH},
        };
        int scx[4], scy[4];
        bool valid[4] = {};
        for (int i = 0; i < 4; ++i) {
            float cvx = vx + corners[i].dvx;
            float cvy = vy + corners[i].dvy;
            float tsx, tsy;
            viewToScreen(cam, cvx, cvy, vz, tsx, tsy);
            scx[i] = static_cast<int>(tsx);
            scy[i] = static_cast<int>(tsy);
            valid[i] = true;
        }
        // Draw the four edges of the quad
        for (int i = 0; i < 4; ++i) {
            int j = (i + 1) % 4;
            if (valid[i] && valid[j])
                drawLine(buf, w, h, scx[i], scy[i], scx[j], scy[j], color);
        }
    }

    // ── fillGrid ─────────────────────────────────────────────────────────────
    void fillGrid(uint32_t* buf, uint32_t w, uint32_t h,
                  const ViewportSlot& slot) const
    {
        (void)slot;
        uint32_t cx = w / 2;
        uint32_t cy = h / 2;

        for (uint32_t y = 0; y < h; ++y) {
            for (uint32_t x = 0; x < w; ++x) {
                uint32_t color = bgColor;

                // Grid lines
                if (gridSpacing > 0 && (x % gridSpacing == 0 || y % gridSpacing == 0))
                    color = gridColor;

                // Centre axis cross — X axis (horizontal)
                if (y == cy && x < w)
                    color = axisXColor;
                // Centre axis cross — Y axis (vertical)
                if (x == cx && y < h)
                    color = axisYColor;

                buf[y * w + x] = color;
            }
        }
    }

    // ── State ─────────────────────────────────────────────────────────────────
    std::vector<uint32_t> m_buffer;
    uint32_t m_renderCount = 0;
    uint32_t m_lastWidth   = 0;
    uint32_t m_lastHeight  = 0;
};

} // namespace NF
