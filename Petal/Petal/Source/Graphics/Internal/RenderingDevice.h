#pragma once

#include "DeviceRequirements.h"
#include "Graphics/GraphicsContext.h"

namespace Petal {
    class VulkanQueue;
    class Engine;
    class GraphicsSystem;

    class RenderingDevice {
    public:
        // Select a physical device and create a logical device for a particular surface
        // deviceRequirements - List of requirements which the physical device must fulfill
        //    APIVersion - The physical device must support this version of Vulkan
        //    QueueFamilies - The physical device must have a queue family meeting all requirements for each requirement in the vector.
        //       The requested queue family indices will be stored in GetQueueFamilies()
        //       You must request exactly one "Graphics" queue family, this means a queue family with VK_QUEUE_GRAPHICS_BIT and supporting presenting to the current surface.
        //       The graphics queue family will be stored in the GetQueueFamilies() vector but also is available via GetGraphicsQueueFamily()
        explicit RenderingDevice(
            Engine &engine,
            GraphicsSystem &renderingSystem,
            GraphicsContext &renderer,
            const DeviceRequirements &deviceRequirements,
            Result &resultOut
        );

        ~RenderingDevice();

    public:
        VkDevice GetDevice() const;

        VkPhysicalDevice GetPhysicalDevice() const;

        VkSurfaceCapabilities2KHR GetSurfaceCapabilities() const;

        const std::vector<VkPresentModeKHR> &GetPresentModes() const;

        const std::vector<VkSurfaceFormat2KHR> &GetSurfaceFormats() const;

        // Returns a vector of queue families. GetQueueFamilies[i] meets deviceRequirements.QueueFamilies[i]
        std::vector<VulkanQueue> &GetQueueFamilies();

        // Returns a queue family supporting present with graphics type, this is the GPU index and is also contained in the GetQueueFamilies() vector
        VulkanQueue &GetGraphicsQueueFamily();

        void InitializeQueueFamilies();

        // Search for a supported depth format
        Optional<VkFormat> FindDepthFormat() const;

    private:
        Optional<VkFormat> FindSupportedFormat(
            const std::vector<VkFormat> &candidates,
            VkImageTiling tiling,
            VkFormatFeatureFlags requestedFeatures
        ) const;

        Result QueryDeviceSurfaceCapabilities();

        Result SelectPhysicalDevice(const DeviceRequirements &deviceRequirements);

        Result QuerySupportedSurfaceFormats();

        Result QuerySupportedPresentModes();

        Optional<std::vector<VkPhysicalDevice> > GetDevices();

        Result GetRequiredQueueFamilyIndices(VkPhysicalDevice physicalDevice, const std::vector<VkQueueFamilyProperties> &queueFamilies,
                                             const std::vector<QueueFamilyRequirements> &requirements);

        bool DoesQueueFamilyMeetsRequirements(VkPhysicalDevice physicalDevice, glm::u32 queueFamilyIndex, VkQueueFamilyProperties properties,
                                              QueueFamilyRequirements requirements) const;

        bool DoesDeviceMeetAllQueueFamilyRequirements(const std::vector<VkQueueFamilyProperties> &queueFamilies, const std::vector<QueueFamilyRequirements> &requirements) const;

        Optional<std::vector<VkQueueFamilyProperties> > GetQueueFamilyProperties(VkPhysicalDevice device);

        Result GetExtensions();

        void GetDeviceFeatures(
            VkPhysicalDeviceFeatures2 &deviceFeatures,
            VkPhysicalDeviceDescriptorIndexingFeaturesEXT &indexingFeatures
        ) const;

        Result CreateLogicalDevice(DeviceRequirements deviceRequirements);

        // Find which of the requested queue families is the graphics one
        Result FindRequestedGraphicsQueueFamily(const DeviceRequirements &deviceRequirements);

    private:
        Engine &m_engine;
        GraphicsSystem &m_renderingSystem;
        GraphicsContext &m_renderer;
        std::shared_ptr<Logger> m_logger;
        VkDevice m_device;
        VkPhysicalDevice m_physicalDevice;
        std::vector<glm::u32> m_requiredQueueFamiliesIndices;
        std::vector<VulkanQueue> m_requiredQueueFamilies;
        glm::u32 m_indexOfGraphicsQueueFamily; // index into m_requiredQueueFamilies
        std::unordered_map<std::string, Version> m_deviceExtensions; // Extension name -> specVersion
        VkSurfaceCapabilities2KHR m_surfaceCapabilities = {
            .sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR,
            .pNext = nullptr
        };
        std::vector<VkPresentModeKHR> m_presentModes;
        std::vector<VkSurfaceFormat2KHR> m_surfaceFormats;
    };
} // Petal
