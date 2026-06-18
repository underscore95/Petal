#include "GraphicsSystem.h"
#include "Engine.h"
#include "Internal/Vulkan.h"
#include "Memory/MemorySystem.h"
#include "Window/Window.h"
#include "Window/WindowSystem.h"
#include "Shaders/ShaderSubsystem.h"

namespace Petal {
    GraphicsSystem::GraphicsSystem(
        Engine &engine,
        Version apiVersion
    ) : m_engine(engine),
        m_apiVersion(apiVersion) {
        m_logger = engine.GetLoggerSystem().GetLogger(LoggerSystem::GRAPHICS_LOGGER);
        m_graphicsAPILogger = engine.GetLoggerSystem().GetLogger(LoggerSystem::GRAPHICS_API_LOGGER);
        m_graphicsAPILogger->SetLevel(LogLevel::Warn);

        if (CreateInstance() != Result::SUCCESS) return;

#ifndef NDEBUG
        if (CreateDebugCallback() != Result::SUCCESS) return;

        if (FindSetObjectDebugNameFunction() != Result::SUCCESS) return;
#endif

        Result result;
        m_shaderSubsystem = std::make_unique<ShaderSubsystem>(m_engine, *this, m_logger, result);
        if (result != Result::SUCCESS) return;
    }

    GraphicsSystem::~GraphicsSystem() {
        m_renderers.clear();

#ifndef NDEBUG
        DestroyDebugMessenger();
#endif
        if (m_instance) vkDestroyInstance(m_instance, nullptr);
    }

    const VkInstance &GraphicsSystem::GetInstance() const {
        return m_instance;
    }

    OptionalRef<GraphicsContext> GraphicsSystem::CreateGraphicsContext(std::shared_ptr<Window> window, DeviceRequirements deviceRequirements) {
        Result result = Result::SUCCESS;

        auto renderer = std::make_unique<GraphicsContext>(
            m_engine,
            *this,
            window,
            deviceRequirements,
            GraphicsSettings{},
            result
        );

        PETAL_CHECK_COND(result != Result::SUCCESS, result, m_logger, "Failed to create renderer: {}", result);

        auto it = m_renderers.insert(std::move(renderer));
        return *it.first->get();
    }

    const Version &GraphicsSystem::GetAPIVersion() const {
        return m_apiVersion;
    }

    ShaderSubsystem &GraphicsSystem::GetShaderSubsystem() const {
        return *m_shaderSubsystem;
    }

    const PFN_vkSetDebugUtilsObjectNameEXT &GraphicsSystem::VulkanSetDebugObjectNameFunction() const {
        return m_vkSetDebugUtilsObjectNameEXT;
    }

    Result GraphicsSystem::CreateInstance() {
        std::vector enables = {
            VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
            VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,
            VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
            VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT
        };
        VkValidationFeaturesEXT validationFeaturesEXT = CreateValidationFeatures(enables, nullptr);
        VkApplicationInfo appInfo = CreateApplicationInfo();

        std::vector<const char *> layerNames = GetLayerNames();
        std::vector<const char *> extensionNames = GetExtensionNames();
        const void *next =
#ifndef NDEBUG
                &validationFeaturesEXT;
#else
                nullptr;
#endif
        VkInstanceCreateInfo createInfo = CreateInstanceCreateInfo(
            appInfo,
            layerNames,
            extensionNames,
            next
        );

        VkResult res = vkCreateInstance(&createInfo, nullptr, &m_instance);
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::VULKAN_INSTANCE_CREATION_FAILED, m_logger, "Failed to create Vulkan instance: {}", res);

        m_logger->Verbose("Created vulkan instance");
        return Result::SUCCESS;
    }

    VkValidationFeaturesEXT GraphicsSystem::CreateValidationFeatures(
        const std::vector<VkValidationFeatureEnableEXT> &enables,
        const void *next
    ) {
        return {
            .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
            .pNext = next,
            .enabledValidationFeatureCount = static_cast<glm::u32>(enables.size()),
            .pEnabledValidationFeatures = enables.empty() ? nullptr : enables.data(),
            .disabledValidationFeatureCount = 0,
            .pDisabledValidationFeatures = VK_NULL_HANDLE
        };
    }

    VkApplicationInfo GraphicsSystem::CreateApplicationInfo() const {
        return {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = m_engine.GetAppInfo().Name.c_str(),
            .applicationVersion = m_engine.GetAppInfo().AppVersion.ToVulkanVersion(),
            .pEngineName = m_engine.GetEngineInfo().Name.c_str(),
            .engineVersion = m_engine.GetEngineInfo().EngineVersion.ToVulkanVersion(),
            .apiVersion = m_apiVersion.ToVulkanVersion()
        };
    }

    VkInstanceCreateInfo GraphicsSystem::CreateInstanceCreateInfo(
        const VkApplicationInfo &appInfo,
        const std::vector<const char *> &layerNames,
        const std::vector<const char *> &extensionNames,
        const void *next
    ) {
        return {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = next,
            .flags = 0,
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = static_cast<glm::u32>(layerNames.size()),
            .ppEnabledLayerNames = layerNames.data(),
            .enabledExtensionCount = static_cast<glm::u32>(extensionNames.size()),
            .ppEnabledExtensionNames = extensionNames.data()
        };
    }

    const char *GetOSSurfaceExtension() {
#if defined (_WIN32)
        return "VK_KHR_win32_surface";
#elif defined (__APPLE__)
    return        "VK_MVK_macos_surface";
#elif defined (__linux__)
   return         "VK_KHR_xcb_surface";
#else
        static_assert(false, "Invalid platform");
#endif
    }

    std::vector<const char *> GraphicsSystem::GetExtensionNames() {
        return {
            VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
            VK_KHR_SURFACE_EXTENSION_NAME,
            GetOSSurfaceExtension(),

#ifndef NDEBUG
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
        };
    }

    std::vector<const char *> GraphicsSystem::GetLayerNames() {
        return {
#ifndef NDEBUG
            "VK_LAYER_KHRONOS_validation"
#endif
        };
    }

    Result GraphicsSystem::CreateDebugCallback() {
        auto vkCreateDebugUtilsMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(
                m_instance,
                "vkCreateDebugUtilsMessengerEXT"
            )
        );

        PETAL_CHECK_COND(!vkCreateDebugUtilsMessenger, Result::VULKAN_CANNOT_FIND_ADDRESS_VK_CREATE_DEBUG_UTILS_MESSENGER, m_logger, "");

        VkDebugUtilsMessengerCreateInfoEXT createInfo = CreateDebugMessengerCreateInfo();

        // Create it
        VkResult res = vkCreateDebugUtilsMessenger(m_instance, &createInfo, nullptr, &m_debugMessenger);
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::VULKAN_DEBUG_UTILS_MESSENGER_CREATION_FAILED, m_logger, "Failed to create vulkan DebugUtilsMessenger: {}", res);

        m_logger->Verbose("Created vulkan debug callback");

        return Result::SUCCESS;
    }

    VkBool32 GraphicsSystem::DebugCallbackStatic(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData
    ) {
        auto renderingSystem = static_cast<GraphicsSystem *>(pUserData);
        assert(renderingSystem);

        return renderingSystem->DebugCallback(severity, type, pCallbackData);
    }

    VkBool32 GraphicsSystem::DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData
    ) const {
        // Get log level
        LogLevel logLevel;
        switch (severity) {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                logLevel = LogLevel::Error;
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                logLevel = LogLevel::Warn;
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                logLevel = LogLevel::Info;
                break;
            default:
                logLevel = LogLevel::Verbose;
        }

        // Objects
        std::string objectsStr;
        for (uint32_t i = 0; i < pCallbackData->objectCount; i++) {
            objectsStr += std::format(
                "{:016x} (Type: {})",
                pCallbackData->pObjects[i].objectHandle,
                string_VkObjectType(pCallbackData->pObjects[i].objectType)
            );

            if (pCallbackData->pObjects[i].pObjectName) {
                objectsStr += std::format(" \"{}\"", pCallbackData->pObjects[i].pObjectName);
            }

            if (i < pCallbackData->objectCount - 1) {
                objectsStr += ", ";
            }
        }
        if (!objectsStr.empty()) {
            objectsStr = "\nObjects: " + objectsStr;
        }

        // Build message
        std::string message = std::format(
            "[{} ({})] {}{}",
            pCallbackData->pMessageIdName ? pCallbackData->pMessageIdName : "NoID",
            pCallbackData->messageIdNumber,
            pCallbackData->pMessage,
            objectsStr
        );

        m_graphicsAPILogger->Log(logLevel, "(Vulkan: {})\n{}", string_VkDebugUtilsMessageTypeFlagsEXT(type), message);

        return false;
    }

    VkDebugUtilsMessengerCreateInfoEXT GraphicsSystem::CreateDebugMessengerCreateInfo() {
        return {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext = nullptr,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = &DebugCallbackStatic,
            .pUserData = this
        };
    }

    void GraphicsSystem::DestroyDebugMessenger() const {
        if (!m_debugMessenger) return;
        PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessenger = VK_NULL_HANDLE;
        vkDestroyDebugUtilsMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
            m_instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (!vkDestroyDebugUtilsMessenger) {
            m_logger->Error("Cannot find address of vkDestroyDebugUtilsMessenger");
        } else {
            vkDestroyDebugUtilsMessenger(m_instance, m_debugMessenger, nullptr);
            m_logger->Verbose("Destroyed Debug Callback");
        }
    }

#ifndef NDEBUG
    Result GraphicsSystem::FindSetObjectDebugNameFunction() {
        m_vkSetDebugUtilsObjectNameEXT = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
            vkGetInstanceProcAddr(GetInstance(), "vkSetDebugUtilsObjectNameEXT")
        );
        PETAL_CHECK_COND(m_vkSetDebugUtilsObjectNameEXT == nullptr, Result::VULKAN_FIND_SET_OBJECT_DEBUG_NAME_FUNCTION_FAILED, m_logger, "");
        return Result::SUCCESS;
    }

    void GraphicsSystem::SetObjectDebugNameImpl() {
    }
#endif
} // Petal
