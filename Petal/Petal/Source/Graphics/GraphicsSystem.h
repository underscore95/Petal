#pragma once

#include <vulkan/vulkan.h>
#include "Common.h"
#include "Internal/DeviceRequirements.h"
#include "Internal/RenderingDevice.h"
#include "GraphicsContext.h"
#include "Internal/CommandBuffers/CommandBuffer.h"
#include "Internal/CommandBuffers/CommandBufferVector.h"
#include "Internal/VulkanSwapchain.h"

namespace Petal {
    class Engine;
    class ShaderSubsystem;
    class Window;

    class GraphicsSystem {
    public:
        explicit GraphicsSystem(
            Engine &engine,
            Version apiVersion
        );

        ~GraphicsSystem();

        DISABLE_COPY_AND_MOVE(GraphicsSystem);

    public:
        const VkInstance &GetInstance() const;

        OptionalRef<GraphicsContext> CreateGraphicsContext(std::shared_ptr<Window> window, DeviceRequirements deviceRequirements);

        const Version &GetAPIVersion() const;

        ShaderSubsystem &GetShaderSubsystem() const;

#ifndef NDEBUG
        const PFN_vkSetDebugUtilsObjectNameEXT &VulkanSetDebugObjectNameFunction() const;
#endif

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

#ifndef NDEBUG
        Result FindSetObjectDebugNameFunction();

        void SetObjectDebugNameImpl();
#endif

    private:
        Engine &m_engine;
        Version m_apiVersion;
        std::shared_ptr<Logger> m_logger;
        std::shared_ptr<Logger> m_graphicsAPILogger;
        VkInstance m_instance;
        VkDebugUtilsMessengerEXT m_debugMessenger;
        std::unordered_set<std::unique_ptr<GraphicsContext> > m_renderers;
        std::unique_ptr<ShaderSubsystem> m_shaderSubsystem;
#ifndef NDEBUG
        PFN_vkSetDebugUtilsObjectNameEXT m_vkSetDebugUtilsObjectNameEXT;
#endif
    };
} // Petal
