#pragma once
#include "../ui/UIRenderer.h"
#include <climits>
#include <cstdint>
#include <string>
#include <vector>

#ifdef DrawText
#undef DrawText
#endif

namespace atlas::render {

/// Stub Vulkan API version for environments without the Vulkan SDK
constexpr uint32_t VK_HEADER_VERSION_STUB = ((1 << 22) | (3 << 12) | 0); // 1.3.0

struct VkDrawCommand {
    enum class Kind : uint8_t {
        Rect,
        Text,
        Icon,
        Border,
        Image
    };

    Kind kind = Kind::Rect;
    ui::UIRect rect{};
    ui::UIColor color{};
    int32_t thickness = 0;
    uint32_t resourceId = 0;
    std::string text;
};

struct VkGPUCommandBuffer {
    uint32_t frameIndex = 0;
    std::vector<VkDrawCommand> commands;
    bool submitted = false;
    uint64_t submitTimestamp = 0;
};

struct VkRenderPassDesc {
    std::string name;
    uint32_t colorAttachmentCount = 1;
    bool hasDepth = false;
    bool clear = true;
};

struct VkPipelineStateDesc {
    std::string vertexShader;
    std::string fragmentShader;
    bool depthTest = true;
    bool depthWrite = true;
    bool blending = false;
    uint32_t id = 0;
};

struct VkGPUResource {
    enum class Type : uint8_t { VertexBuffer, IndexBuffer, UniformBuffer };
    Type type = Type::VertexBuffer;
    uint32_t id = 0;
    size_t sizeBytes = 0;
    bool mapped = false;
};

struct VkDescriptorSetLayoutDesc {
    std::string name;
    uint32_t bindingCount = 0;
    uint32_t id = 0;
};

struct VkTextureDesc {
    std::string name;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t mipLevels = 1;
    uint32_t id = 0;
};

struct VkSamplerDesc {
    std::string name;
    bool linearFilter = true;
    bool clampToEdge = true;
    uint32_t id = 0;
};

/// A push-constant block pushed to a shader stage.
struct VkPushConstantRange {
    std::string name;
    uint32_t offset = 0;
    uint32_t size = 0;   ///< Size in bytes (max 128 bytes per Vulkan spec)
    uint32_t id = 0;
};

/// A scalar/vector/matrix uniform value bound to a shader.
struct VkShaderUniform {
    std::string name;
    uint32_t binding = 0;
    uint32_t set = 0;
    uint32_t sizeBytes = 0;
    std::vector<uint8_t> data;
    uint32_t id = 0;
};

/// GPU fence for CPU-GPU synchronization
struct VkFenceDesc {
    std::string name;
    bool signaled = false;
    uint32_t id = 0;
};

/// GPU semaphore for GPU-GPU synchronization between queue submissions
struct VkSemaphoreDesc {
    std::string name;
    bool signaled = false;
    uint32_t id = 0;
};

/// Memory pool for GPU allocations
struct VkMemoryPool {
    std::string name;
    size_t totalSize = 0;
    size_t usedSize = 0;
    uint32_t allocationCount = 0;
    uint32_t id = 0;
};

struct VkMemoryAllocation {
    uint32_t poolId = 0;
    size_t offset = 0;
    size_t size = 0;
    uint32_t id = 0;
};

/// Vulkan physical device properties (hardware GPU info)
struct VkPhysicalDeviceInfo {
    std::string deviceName;
    uint32_t vendorId = 0;
    uint32_t deviceId = 0;
    uint32_t driverVersion = 0;
    uint32_t apiVersion = 0;
    enum class DeviceType : uint8_t { Other, IntegratedGPU, DiscreteGPU, VirtualGPU, CPU };
    DeviceType deviceType = DeviceType::Other;
    size_t totalMemoryBytes = 0;
    bool supportsGeometryShader = false;
    bool supportsTessellation = false;
    bool supportsCompute = true;
};

/// Vulkan queue family properties
struct VkQueueFamilyInfo {
    uint32_t index = 0;
    uint32_t queueCount = 0;
    bool supportsGraphics = false;
    bool supportsCompute = false;
    bool supportsTransfer = false;
    bool supportsPresent = false;
};

/// Vulkan swap chain configuration
struct VkSwapChainDesc {
    uint32_t imageCount = 2;            ///< Double or triple buffering
    uint32_t width = 0;
    uint32_t height = 0;
    enum class PresentMode : uint8_t { Immediate, Mailbox, FIFO, FIFORelaxed };
    PresentMode presentMode = PresentMode::FIFO;
    enum class Format : uint8_t { BGRA8_SRGB, RGBA8_SRGB, BGRA8_UNORM, RGBA8_UNORM };
    Format format = Format::BGRA8_SRGB;
    bool valid = false;
};

/// Vulkan logical device configuration
struct VkDeviceConfig {
    std::string applicationName = "AtlasEngine";
    uint32_t applicationVersion = 1;
    bool enableValidationLayers = false;
    bool enableDebugMessenger = false;
    std::vector<std::string> requiredExtensions;
    std::vector<std::string> requiredDeviceExtensions;
};

class VulkanRenderer : public ui::UIRenderer {
public:
    void BeginFrame() override;
    void EndFrame() override;

    void DrawRect(const ui::UIRect& rect, const ui::UIColor& color) override;
    void DrawText(const ui::UIRect& rect, const std::string& text, const ui::UIColor& color) override;
    void DrawIcon(const ui::UIRect& rect, uint32_t iconId, const ui::UIColor& tint) override;
    void DrawBorder(const ui::UIRect& rect, int32_t thickness, const ui::UIColor& color) override;
    void DrawImage(const ui::UIRect& rect, uint32_t textureId, const ui::UIColor& tint) override;

    void SetViewport(int32_t width, int32_t height) override;

    const std::vector<VkDrawCommand>& DrawCommands() const;
    size_t DrawCommandCount() const;
    bool IsFrameActive() const;
    uint32_t FrameCount() const;

    void SubmitCommandBuffer();
    const VkGPUCommandBuffer& LastSubmittedBuffer() const;
    uint32_t SubmittedBufferCount() const;
    bool HasPendingCommands() const;

    // Render pass management
    uint32_t CreateRenderPass(const VkRenderPassDesc& desc);
    void BeginRenderPass(uint32_t passId);
    void EndRenderPass();
    bool IsRenderPassActive() const;
    const VkRenderPassDesc* GetRenderPass(uint32_t id) const;
    uint32_t RenderPassCount() const;

    // Pipeline state management
    uint32_t CreatePipelineState(const VkPipelineStateDesc& desc);
    void BindPipeline(uint32_t pipelineId);
    uint32_t BoundPipelineId() const;
    const VkPipelineStateDesc* GetPipelineState(uint32_t id) const;
    uint32_t PipelineStateCount() const;

    // GPU resource management
    uint32_t CreateBuffer(VkGPUResource::Type type, size_t sizeBytes);
    bool DestroyBuffer(uint32_t bufferId);
    const VkGPUResource* GetBuffer(uint32_t id) const;
    uint32_t BufferCount() const;
    bool MapBuffer(uint32_t bufferId);
    bool UnmapBuffer(uint32_t bufferId);

    // Descriptor set management
    uint32_t CreateDescriptorSetLayout(const VkDescriptorSetLayoutDesc& desc);
    void BindDescriptorSet(uint32_t layoutId);
    uint32_t BoundDescriptorSetId() const;
    const VkDescriptorSetLayoutDesc* GetDescriptorSetLayout(uint32_t id) const;
    uint32_t DescriptorSetLayoutCount() const;

    // Texture management
    uint32_t CreateTexture(const VkTextureDesc& desc);
    bool DestroyTexture(uint32_t textureId);
    const VkTextureDesc* GetTexture(uint32_t id) const;
    uint32_t TextureCount() const;

    // Sampler management
    uint32_t CreateSampler(const VkSamplerDesc& desc);
    bool DestroySampler(uint32_t samplerId);
    const VkSamplerDesc* GetSampler(uint32_t id) const;
    uint32_t SamplerCount() const;

    // Push constant management
    uint32_t RegisterPushConstantRange(const VkPushConstantRange& range);
    bool PushConstants(uint32_t rangeId, const void* data, uint32_t sizeBytes);
    const VkPushConstantRange* GetPushConstantRange(uint32_t id) const;
    uint32_t PushConstantRangeCount() const;
    const std::vector<uint8_t>& PushConstantData(uint32_t rangeId) const;

    // Shader uniform management
    uint32_t BindUniform(const VkShaderUniform& uniform);
    bool UpdateUniform(uint32_t uniformId, const void* data, uint32_t sizeBytes);
    const VkShaderUniform* GetUniform(uint32_t id) const;
    const VkShaderUniform* GetUniformByName(const std::string& name) const;
    uint32_t UniformCount() const;
    void ClearUniforms();

    // Fence management
    uint32_t CreateFence(const std::string& name, bool signaled = false);
    bool DestroyFence(uint32_t fenceId);
    bool WaitFence(uint32_t fenceId);
    bool ResetFence(uint32_t fenceId);
    bool IsFenceSignaled(uint32_t fenceId) const;
    const VkFenceDesc* GetFence(uint32_t id) const;
    uint32_t FenceCount() const;

    // Semaphore management
    uint32_t CreateSemaphore(const std::string& name);
    bool DestroySemaphore(uint32_t semaphoreId);
    bool SignalSemaphore(uint32_t semaphoreId);
    bool WaitSemaphore(uint32_t semaphoreId);
    const VkSemaphoreDesc* GetSemaphore(uint32_t id) const;
    uint32_t SemaphoreCount() const;

    // Memory pool management
    uint32_t CreateMemoryPool(const std::string& name, size_t totalSize);
    bool DestroyMemoryPool(uint32_t poolId);
    uint32_t AllocateFromPool(uint32_t poolId, size_t size);
    bool FreeAllocation(uint32_t allocationId);
    const VkMemoryPool* GetMemoryPool(uint32_t id) const;
    const VkMemoryAllocation* GetAllocation(uint32_t id) const;
    uint32_t MemoryPoolCount() const;
    size_t PoolUsedSize(uint32_t poolId) const;
    size_t PoolFreeSize(uint32_t poolId) const;

    // --- Vulkan device management ---

    /// Initialize the Vulkan instance and select a physical device.
    /// Returns true if a device was selected (or stub mode is active).
    bool InitDevice(const VkDeviceConfig& config);

    /// Shut down the Vulkan device and release all resources.
    void ShutdownDevice();

    /// Whether the device has been initialized.
    bool IsDeviceInitialized() const;

    /// Returns the selected physical device info.
    const VkPhysicalDeviceInfo& GetPhysicalDeviceInfo() const;

    /// Returns discovered queue families.
    const std::vector<VkQueueFamilyInfo>& GetQueueFamilies() const;

    /// Returns the index of the graphics queue family, or UINT32_MAX if none.
    uint32_t GetGraphicsQueueFamily() const;

    /// Returns the index of the compute queue family, or UINT32_MAX if none.
    uint32_t GetComputeQueueFamily() const;

    /// Returns the index of the transfer queue family, or UINT32_MAX if none.
    uint32_t GetTransferQueueFamily() const;

    /// Create a swap chain with the given description.
    bool CreateSwapChain(const VkSwapChainDesc& desc);

    /// Returns the current swap chain description.
    const VkSwapChainDesc& GetSwapChain() const;

    /// Resize the swap chain (e.g. on window resize).
    bool ResizeSwapChain(uint32_t width, uint32_t height);

    /// Whether a valid swap chain is active.
    bool HasSwapChain() const;

    /// Returns the device configuration used during InitDevice.
    const VkDeviceConfig& GetDeviceConfig() const;

    /// Enumerate available physical devices (returns device info for each).
    const std::vector<VkPhysicalDeviceInfo>& EnumerateDevices() const;

    /// Select a specific device by index from EnumerateDevices() result.
    bool SelectDevice(uint32_t deviceIndex);

    static constexpr uint32_t MAX_BUFFERED_FRAMES = 3;

private:
    int32_t m_viewportWidth = 1280;
    int32_t m_viewportHeight = 720;
    std::vector<VkDrawCommand> m_drawCommands;
    bool m_frameActive = false;
    uint32_t m_frameCount = 0;
    std::vector<VkGPUCommandBuffer> m_submittedBuffers;
    uint64_t m_submitCounter = 0;

    std::vector<VkRenderPassDesc> m_renderPasses;
    uint32_t m_activeRenderPass = 0;
    bool m_renderPassActive = false;

    std::vector<VkPipelineStateDesc> m_pipelineStates;
    uint32_t m_boundPipeline = 0;

    std::vector<VkGPUResource> m_buffers;
    uint32_t m_nextBufferId = 1;
    uint32_t m_nextPassId = 1;
    uint32_t m_nextPipelineId = 1;

    std::vector<VkDescriptorSetLayoutDesc> m_descriptorSetLayouts;
    uint32_t m_boundDescriptorSet = 0;
    uint32_t m_nextDescriptorSetId = 1;

    std::vector<VkTextureDesc> m_textures;
    uint32_t m_nextTextureId = 1;

    std::vector<VkSamplerDesc> m_samplers;
    uint32_t m_nextSamplerId = 1;

    std::vector<VkPushConstantRange> m_pushConstantRanges;
    std::vector<std::vector<uint8_t>>  m_pushConstantData;
    uint32_t m_nextPushConstantId = 1;

    std::vector<VkShaderUniform> m_uniforms;
    uint32_t m_nextUniformId = 1;

    std::vector<VkFenceDesc> m_fences;
    uint32_t m_nextFenceId = 1;

    std::vector<VkSemaphoreDesc> m_semaphores;
    uint32_t m_nextSemaphoreId = 1;

    std::vector<VkMemoryPool> m_memoryPools;
    uint32_t m_nextPoolId = 1;

    std::vector<VkMemoryAllocation> m_allocations;
    uint32_t m_nextAllocationId = 1;

    static const std::vector<uint8_t> s_emptyPushData;

    // Vulkan device state
    bool m_deviceInitialized = false;
    VkDeviceConfig m_deviceConfig;
    VkPhysicalDeviceInfo m_selectedDevice;
    std::vector<VkPhysicalDeviceInfo> m_availableDevices;
    std::vector<VkQueueFamilyInfo> m_queueFamilies;
    VkSwapChainDesc m_swapChain;
};

} // namespace atlas::render
