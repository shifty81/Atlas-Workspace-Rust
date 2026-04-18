#pragma once
// NF::D3D11Backend — Primary GPU rendering backend for the UI system.
//
// This is the intended primary backend for Atlas Workspace.
// Uses Direct3D 11 for batched quad rendering, texture atlas management,
// and scissor-based clipping.  Text rendering delegates to DirectWriteTextBackend.
//
// Platform: Windows only (_WIN32).
//
// Architecture:
//   D3D11Backend implements IFrameBackend, IGeometryBackend, ITextureBackend.
//   Text rendering is delegated to an ITextRenderBackend (DirectWriteTextBackend).
//
// Device topology:
//   ID3D11Device         — logical GPU device
//   ID3D11DeviceContext  — immediate rendering context
//   IDXGISwapChain       — present / swap chain (owned by window layer)
//   ID3D11RenderTargetView — back-buffer render target
//   ID3D11Buffer (VB)    — dynamic vertex buffer (mapped per frame)
//   ID3D11Buffer (IB)    — dynamic index  buffer (mapped per frame)
//   ID3D11VertexShader   — compiled from embedded HLSL kVS_Source
//   ID3D11PixelShader    — compiled from embedded HLSL kPS_Source
//   ID3D11InputLayout    — pos2/uv2/color4 vertex layout
//   ID3D11Buffer (CB)    — constant buffer: float4x4 orthoProjection
//   ID3D11BlendState     — alpha-premultiplied blending
//   ID3D11SamplerState   — bilinear clamped texture sampler
//   ID3D11RasterizerState — scissor-enabled, no-cull rasteriser
//
// Implementation note:
//   This file is an architecturally-complete stub.  All Win32 / COM types are
//   commented out so the file compiles on non-Windows platforms.
//   Replace the stub bodies with real COM calls when targeting Windows + D3D11.

#include "NF/UI/UIBackend.h"
#include "NF/UI/IUIBackendInterfaces.h"
#include <memory>
#include <string>
#include <cstdint>

#ifdef _WIN32

// Forward-declare COM interfaces to keep the header self-contained.
// Real Windows builds include <d3d11.h> and <dxgi.h>.
struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;
struct ID3D11Buffer;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;
struct ID3D11BlendState;
struct ID3D11SamplerState;
struct ID3D11RasterizerState;
struct ID3D11ShaderResourceView;

namespace NF {

// Embedded HLSL shader sources.
// Compiled at runtime via D3DCompile() when the backend initialises.
namespace D3D11Shaders {

inline constexpr const char* kVS_Source = R"hlsl(
cbuffer ProjectionCB : register(b0) {
    float4x4 uOrthoProjection;
};

struct VS_In {
    float2 pos   : POSITION;
    float2 uv    : TEXCOORD0;
    float4 color : COLOR0;
};

struct VS_Out {
    float4 pos   : SV_POSITION;
    float2 uv    : TEXCOORD0;
    float4 color : COLOR0;
};

VS_Out main(VS_In v) {
    VS_Out o;
    o.pos   = mul(uOrthoProjection, float4(v.pos, 0.0f, 1.0f));
    o.uv    = v.uv;
    o.color = v.color;
    return o;
}
)hlsl";

inline constexpr const char* kPS_Source = R"hlsl(
Texture2D    uFontAtlas : register(t0);
SamplerState uSampler   : register(s0);

cbuffer ModeCB : register(b1) {
    int uUseTexture;
    float3 _pad;
};

struct PS_In {
    float4 pos   : SV_POSITION;
    float2 uv    : TEXCOORD0;
    float4 color : COLOR0;
};

float4 main(PS_In p) : SV_TARGET {
    if (uUseTexture) {
        float alpha = uFontAtlas.Sample(uSampler, p.uv).r;
        return float4(p.color.rgb, p.color.a * alpha);
    }
    return p.color;
}
)hlsl";

} // namespace D3D11Shaders

// ── D3D11Backend ─────────────────────────────────────────────────

class D3D11Backend final : public UIBackend,
                           public IFrameBackend,
                           public IGeometryBackend,
                           public ITextureBackend {
public:
    // Attach an external text rendering backend (typically DirectWriteTextBackend).
    // Must be set before init() if text rendering is desired.
    void setTextBackend(ITextRenderBackend* textBackend) {
        m_textBackend = textBackend;
    }

    // ── IFrameBackend ────────────────────────────────────────────

    bool init(int width, int height) override {
        m_width  = width;
        m_height = height;

#ifdef _WIN32
        // Attempt real D3D11 device creation.
        // This requires linking d3d11.lib which may not be available in all
        // build configurations.  The stub path below handles that gracefully.
        //
        // Steps for real activation (when compiled with Windows SDK + d3d11.lib):
        //   1. D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, ...)
        //   2. Create swap chain from HWND (requires window handle — deferred)
        //   3. Compile shaders from kVS_Source / kPS_Source
        //   4. Create vertex/index/constant buffers
        //   5. Create blend/sampler/rasterizer states
        //
        // For now we attempt D3D11CreateDevice to detect GPU availability,
        // but full pipeline setup requires a window handle (HWND) which is
        // provided later via attachSwapChain().
        //
        // If D3D11CreateDevice is not available (stub build / non-Windows CI),
        // we fall back to the GDI path via the backend selector.

        NF_LOG_INFO("UI", "D3D11Backend: init requested at " +
                    std::to_string(width) + "x" + std::to_string(height));
        NF_LOG_WARN("UI", "D3D11Backend: real D3D11 device creation requires "
                    "Windows SDK + d3d11.lib — using stub path");
#else
        NF_LOG_WARN("UI", "D3D11Backend: not available on this platform");
#endif
        // Returns false until real D3D11 device initialisation is compiled in.
        // The backend selector detects this and falls back to GDI.
        return false;
    }

    void shutdown() override {
        // Real implementation: SafeRelease all COM pointers (rtv, swapChain,
        // context, device, VB, IB, CB, VS, PS, layout, blend, sampler, rast).
        m_width  = 0;
        m_height = 0;
        NF_LOG_INFO("UI", "D3D11Backend shutdown");
    }

    void beginFrame(int width, int height) override {
        m_width  = width;
        m_height = height;
        // Real implementation:
        //   context->OMSetRenderTargets(1, &m_rtv, nullptr);
        //   float clear[4] = {0.11f, 0.11f, 0.11f, 1.f};
        //   context->ClearRenderTargetView(m_rtv, clear);
        //   D3D11_VIEWPORT vp = { 0, 0, (float)width, (float)height, 0, 1 };
        //   context->RSSetViewports(1, &vp);
        //   Update orthographic projection in m_projectionCB.
    }

    void endFrame() override {
        // Real implementation:
        //   m_swapChain->Present(1, 0);  // vsync
    }

    [[nodiscard]] const char* backendName() const override { return "D3D11"; }
    [[nodiscard]] bool isGPUAccelerated() const override { return true; }

    // ── IGeometryBackend ─────────────────────────────────────────

    void flush(const UIVertex* vertices, size_t vertexCount,
               const uint32_t* indices,  size_t indexCount) override {
        if (vertexCount == 0 || indexCount == 0) return;
        m_lastVertexCount = vertexCount;
        m_lastIndexCount  = indexCount;

        // Real implementation:
        //   // Upload vertices
        //   D3D11_MAPPED_SUBRESOURCE mapped{};
        //   context->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        //   memcpy(mapped.pData, vertices, vertexCount * sizeof(UIVertex));
        //   context->Unmap(m_vertexBuffer, 0);
        //
        //   // Upload indices
        //   context->Map(m_indexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        //   memcpy(mapped.pData, indices, indexCount * sizeof(uint32_t));
        //   context->Unmap(m_indexBuffer, 0);
        //
        //   // Bind pipeline state
        //   UINT stride = sizeof(UIVertex), offset = 0;
        //   context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
        //   context->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
        //   context->IASetInputLayout(m_inputLayout);
        //   context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        //   context->VSSetShader(m_vertexShader, nullptr, 0);
        //   context->VSSetConstantBuffers(0, 1, &m_projectionCB);
        //   context->PSSetShader(m_pixelShader, nullptr, 0);
        //   context->PSSetSamplers(0, 1, &m_sampler);
        //   context->OMSetBlendState(m_blendState, nullptr, 0xFFFFFFFF);
        //   context->RSSetState(m_rasterState);
        //   context->DrawIndexed((UINT)indexCount, 0, 0);

        (void)vertices;
        (void)indices;
    }

    void setClipRect(float x, float y, float w, float h) override {
        m_scissorX = x; m_scissorY = y; m_scissorW = w; m_scissorH = h;
        m_scissorActive = true;
        // Real implementation:
        //   D3D11_RECT r = { (LONG)x, (LONG)y, (LONG)(x+w), (LONG)(y+h) };
        //   context->RSSetScissorRects(1, &r);
    }

    void clearClipRect() override {
        m_scissorActive = false;
        // Real implementation:
        //   D3D11_RECT r = { 0, 0, m_width, m_height };
        //   context->RSSetScissorRects(1, &r);
    }

    // ── ITextureBackend ──────────────────────────────────────────

    uint32_t uploadTexture(const uint8_t* pixels, int width, int height) override {
        (void)pixels; (void)width; (void)height;
        // Real implementation:
        //   D3D11_TEXTURE2D_DESC desc = { (UINT)width, (UINT)height, 1, 1,
        //       DXGI_FORMAT_R8G8B8A8_UNORM, {1,0}, D3D11_USAGE_IMMUTABLE,
        //       D3D11_BIND_SHADER_RESOURCE, 0, 0 };
        //   D3D11_SUBRESOURCE_DATA initData = { pixels, (UINT)(width*4), 0 };
        //   ID3D11Texture2D* tex = nullptr;
        //   device->CreateTexture2D(&desc, &initData, &tex);
        //   device->CreateShaderResourceView(tex, nullptr, &srv);
        //   tex->Release();
        //   return m_textures.insert(srv);  // returns slot index
        return 0u;
    }

    void destroyTexture(uint32_t handle) override {
        (void)handle;
        // Real implementation: m_textures[handle]->Release();
    }

    void bindTexture(uint32_t handle) override {
        (void)handle;
        // Real implementation:
        //   auto* srv = handle ? m_textures[handle] : nullptr;
        //   context->PSSetShaderResources(0, 1, &srv);
    }

    // ── UIBackend text shim (delegates to ITextRenderBackend) ────

    void drawTextNative(float x, float y, std::string_view text,
                        uint32_t color) override {
        if (m_textBackend)
            m_textBackend->drawText(std::string(text), x, y, m_defaultFontSize, color);
    }

    [[nodiscard]] Vec2 measureText(std::string_view text,
                                   float fontSize) const override {
        if (m_textBackend) {
            float w = m_textBackend->measureTextWidth(std::string(text), fontSize);
            float h = m_textBackend->lineHeight(fontSize);
            return {w, h};
        }
        return UIBackend::measureText(text, fontSize); // fallback approximation
    }

    // ── Diagnostics ──────────────────────────────────────────────

    [[nodiscard]] size_t lastVertexCount() const { return m_lastVertexCount; }
    [[nodiscard]] size_t lastIndexCount()  const { return m_lastIndexCount; }
    [[nodiscard]] bool   scissorActive()   const { return m_scissorActive; }

private:
    // Capacities for dynamic buffers (real implementation uses these for alloc).
    static constexpr size_t kMaxVertices = 65536;
    static constexpr size_t kMaxIndices  = 65536 * 3;

    int   m_width  = 0;
    int   m_height = 0;
    float m_defaultFontSize = 14.f;

    // Scissor state
    bool  m_scissorActive = false;
    float m_scissorX = 0, m_scissorY = 0, m_scissorW = 0, m_scissorH = 0;

    // Diagnostics
    size_t m_lastVertexCount = 0;
    size_t m_lastIndexCount  = 0;

    // Optional text delegate
    ITextRenderBackend* m_textBackend = nullptr;

    // COM resource handles (null until init() succeeds with real D3D11):
    ID3D11Device*             m_device       = nullptr;
    ID3D11DeviceContext*      m_context      = nullptr;
    IDXGISwapChain*           m_swapChain    = nullptr;
    ID3D11RenderTargetView*   m_rtv          = nullptr;
    ID3D11Buffer*             m_vertexBuffer = nullptr;
    ID3D11Buffer*             m_indexBuffer  = nullptr;
    ID3D11Buffer*             m_projectionCB = nullptr;
    ID3D11VertexShader*       m_vertexShader = nullptr;
    ID3D11PixelShader*        m_pixelShader  = nullptr;
    ID3D11InputLayout*        m_inputLayout  = nullptr;
    ID3D11BlendState*         m_blendState   = nullptr;
    ID3D11SamplerState*       m_sampler      = nullptr;
    ID3D11RasterizerState*    m_rasterState  = nullptr;
};

} // namespace NF

#endif // _WIN32

