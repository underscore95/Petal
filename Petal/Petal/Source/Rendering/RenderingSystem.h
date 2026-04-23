#pragma once

#include <vulkan/vulkan.h>
#include "Common.h"
#include "Internal/DeviceRequirements.h"
#include "Internal/RenderingDevice.h"
#include "Renderer.h"
#include "Internal/CommandBuffers/CommandBuffer.h"
#include "Internal/CommandBuffers/CommandBufferVector.h"
#include "Internal/VulkanSwapchain.h"

namespace Petal {
    class Engine;
    class Window;

    class RenderingSystem {
    public:
        explicit RenderingSystem(
            Engine &engine,
            Version apiVersion
        );

        ~RenderingSystem();

        DISABLE_COPY_AND_MOVE(RenderingSystem);

    public:
        const VkInstance &GetInstance() const;

        OptionalRef<Renderer> CreateRenderer(Ref<Window> window, DeviceRequirements deviceRequirements);

        const Version &GetAPIVersion() const;

    private:
        // Instance
        Result CreateInstance();

        static VkValidationFeaturesEXT CreateValidationFeatures(
            const std::vector<VkValidationFeatureEnableEXT> &enables,
            const void *next
        );

        VkApplicationInfo CreateApplicationInfo() const;

        static VkInstanceCreateInfo CreateInstanceCreateInfo(
            const VkApplicationInfo &appInfo,
            const std::vector<const char *> &layerNames,
            const std::vector<const char *> &extensionNames,
            const void *next
        );

        static std::vector<const char *> GetExtensionNames();

        static std::vector<const char *> GetLayerNames();

        // Debug callback
        Result CreateDebugCallback();

        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallbackStatic(
            VkDebugUtilsMessageSeverityFlagBitsEXT severity,
            VkDebugUtilsMessageTypeFlagsEXT type,
            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
            void *pUserData
        );

        VkBool32 DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT severity,
            VkDebugUtilsMessageTypeFlagsEXT type,
            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData
        ) const;

        VkDebugUtilsMessengerCreateInfoEXT CreateDebugMessengerCreateInfo();

        void DestroyDebugMessenger() const;

    private:
        Engine &m_engine;
        Version m_apiVersion;
        Ref<Logger> m_logger;
        Ref<Logger> m_graphicsAPILogger;
        VkInstance m_instance;
        VkDebugUtilsMessengerEXT m_debugMessenger;
        std::unordered_set<std::unique_ptr<Renderer> > m_renderers;
    };
} // Petal
