#include "RenderingDevice.h"

#include "ConstraintMatching.h"
#include "Engine.h"
#include "VulkanQueue.h"
#include "Graphics/GraphicsSystem.h"

namespace Petal {
    RenderingDevice::RenderingDevice(
        Engine &engine,
        GraphicsSystem &renderingSystem,
        GraphicsContext &renderer,
        const DeviceRequirements &deviceRequirements,
        Result &resultOut
    ) : m_engine(engine),
        m_renderingSystem(renderingSystem),
        m_renderer(renderer) {
        m_logger = engine.GetLoggerSystem().GetLogger(LoggerSystem::GRAPHICS_LOGGER);

        resultOut = FindRequestedGraphicsQueueFamily(deviceRequirements);
        if (resultOut != Result::SUCCESS) return;

        resultOut = SelectPhysicalDevice(deviceRequirements);
        if (resultOut != Result::SUCCESS) return;

        resultOut = GetExtensions();
        if (resultOut != Result::SUCCESS) return;

        resultOut = CreateLogicalDevice(deviceRequirements);

        m_logger->Verbose("Created rendering device");
    }

    RenderingDevice::~RenderingDevice() {
        if (m_device) {
            vkDeviceWaitIdle(m_device);
            vkDestroyDevice(m_device, nullptr);
            m_logger->Verbose("Destroyed rendering device");
        }
    }

    VkDevice RenderingDevice::GetDevice() const {
        return m_device;
    }

    VkPhysicalDevice RenderingDevice::GetPhysicalDevice() const {
        return m_physicalDevice;
    }

    VkSurfaceCapabilities2KHR RenderingDevice::GetSurfaceCapabilities() const {
        return m_surfaceCapabilities;
    }

    const std::vector<VkPresentModeKHR> &RenderingDevice::GetPresentModes() const {
        return m_presentModes;
    }

    const std::vector<VkSurfaceFormat2KHR> &RenderingDevice::GetSurfaceFormats() const {
        return m_surfaceFormats;
    }

    std::vector<VulkanQueue> &RenderingDevice::GetQueueFamilies() {
        if (m_requiredQueueFamilies.empty()) {
            InitializeQueueFamilies();
        }
        return m_requiredQueueFamilies;
    }

    VulkanQueue &RenderingDevice::GetGraphicsQueueFamily() {
        return GetQueueFamilies()[m_indexOfGraphicsQueueFamily];
    }

    void RenderingDevice::InitializeQueueFamilies() {
        if (!m_requiredQueueFamilies.empty()) {
            m_logger->Error("Attempted to call InitializeQueueFamilies() a second time!");
            return;
        }

        m_requiredQueueFamilies.reserve(m_requiredQueueFamiliesIndices.size());
        for (glm::u32 queueFamilyIndex : m_requiredQueueFamiliesIndices) {
            m_requiredQueueFamilies.emplace_back(m_renderer, m_logger, queueFamilyIndex);
        }
    }

    Optional<VkFormat> RenderingDevice::FindDepthFormat() const {
        static const std::vector DEPTH_FORMATS = {
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT
        };

        return FindSupportedFormat(
            DEPTH_FORMATS,
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    Optional<VkFormat> RenderingDevice::FindSupportedFormat(
        const std::vector<VkFormat> &candidates,
        VkImageTiling tiling,
        VkFormatFeatureFlags requestedFeatures
    ) const {
        VkFormat candidateFormat = VK_FORMAT_UNDEFINED;
        for (int i = 0; i < candidates.size(); i++) {
            candidateFormat = candidates[i];
            VkFormatProperties properties;
            vkGetPhysicalDeviceFormatProperties(m_physicalDevice, candidateFormat, &properties);

            if ((tiling == VK_IMAGE_TILING_LINEAR) &&
                (properties.linearTilingFeatures & requestedFeatures) == requestedFeatures) {
                return candidateFormat;
            }
            if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                (properties.optimalTilingFeatures & requestedFeatures) == requestedFeatures) {
                return candidateFormat;
            }
        }

        return Result::PETAL_UNSUPPORTED_FORMAT;
    }

    Result RenderingDevice::QueryDeviceSurfaceCapabilities() {
        VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR,
            .pNext = nullptr,
            .surface = m_renderer.GetSurface()
        };

        VkResult res = vkGetPhysicalDeviceSurfaceCapabilities2KHR(
            m_physicalDevice,
            &surfaceInfo,
            &m_surfaceCapabilities
        );

        PETAL_CHECK_COND(res != VK_SUCCESS, Result::VULKAN_DEVICE_CREATION_FAILED, m_logger, "Failed to query device surface capabilities: {}", res);
        return Result::SUCCESS;
    }

    Result RenderingDevice::SelectPhysicalDevice(
        const DeviceRequirements &deviceRequirements
    ) {
        Optional<std::vector<VkPhysicalDevice> > devices = GetDevices();
        if (devices.IsEmpty()) return devices.GetResult();

        for (VkPhysicalDevice physicalDevice : *devices.Value()) {
            m_physicalDevice = physicalDevice;

            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

            if (Version{deviceProperties.apiVersion} < deviceRequirements.APIVersion) continue;

            Optional<std::vector<VkQueueFamilyProperties> > queueFamilies = GetQueueFamilyProperties(physicalDevice);
            if (queueFamilies.IsEmpty()) continue;
            if (!DoesDeviceMeetAllQueueFamilyRequirements(*queueFamilies.Value(), deviceRequirements.QueueFamilies)) continue;

            Result result = GetRequiredQueueFamilyIndices(
                m_physicalDevice,
                *queueFamilies.Value(),
                deviceRequirements.QueueFamilies
            );
            if (result != Result::SUCCESS) return result;

            result = QueryDeviceSurfaceCapabilities();
            if (result != Result::SUCCESS) return result;

            result = QuerySupportedPresentModes();
            if (result != Result::SUCCESS) return result;

            result = QuerySupportedSurfaceFormats();
            if (result != Result::SUCCESS) return result;

            return Result::SUCCESS;
        }


        PETAL_CHECK_COND(true, Result::VULKAN_DEVICE_CREATION_FAILED, m_logger, "No device found that met all requirements");
        return Result::SUCCESS;
    }

    Result RenderingDevice::QuerySupportedSurfaceFormats() {
        VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR,
            .pNext = nullptr,
            .surface = m_renderer.GetSurface()
        };
        glm::u32 numSurfaceFormats = 0;

        VkResult res = vkGetPhysicalDeviceSurfaceFormats2KHR(
            m_physicalDevice,
            &surfaceInfo,
            &numSurfaceFormats,
            nullptr
        );
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::VULKAN_DEVICE_CREATION_FAILED, m_logger, "Failed to query number of surface formats: {}", res);

        m_surfaceFormats.resize(numSurfaceFormats);
        for (auto &format : m_surfaceFormats) {
            format.sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR;
            format.pNext = nullptr;
        }
        res = vkGetPhysicalDeviceSurfaceFormats2KHR(
            m_physicalDevice,
            &surfaceInfo,
            &numSurfaceFormats,
            m_surfaceFormats.data()
        );
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::VULKAN_DEVICE_CREATION_FAILED, m_logger, "Failed to query surface formats: {}", res);

        return Result::SUCCESS;
    }

    Result RenderingDevice::QuerySupportedPresentModes() {
        glm::u32 numPresentModes = 0;

        VkResult res = vkGetPhysicalDeviceSurfacePresentModesKHR(
            m_physicalDevice,
            m_renderer.GetSurface(),
            &numPresentModes,
            nullptr
        );
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::VULKAN_DEVICE_CREATION_FAILED, m_logger, "Failed to query number of present modes: {}", res);

        m_presentModes.resize(numPresentModes);
        res = vkGetPhysicalDeviceSurfacePresentModesKHR(
            m_physicalDevice,
            m_renderer.GetSurface(),
            &numPresentModes,
            m_presentModes.data()
        );
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::VULKAN_DEVICE_CREATION_FAILED, m_logger, "Failed to query present modes: {}", res);

        return Result::SUCCESS;
    }

    Optional<std::vector<VkPhysicalDevice> > RenderingDevice::GetDevices() {
        glm::u32 numDevices = 0;

        VkResult res = vkEnumeratePhysicalDevices(m_renderingSystem.GetInstance(), &numDevices, nullptr);
        PETAL_CHECK_COND(
            res != VK_SUCCESS,
            Result::VULKAN_DEVICE_CREATION_FAILED,
            m_logger,
            "Failed to get number of physical devices: {}",
            res
        );

        std::vector<VkPhysicalDevice> devices(numDevices);
        res = vkEnumeratePhysicalDevices(m_renderingSystem.GetInstance(), &numDevices, devices.data());
        PETAL_CHECK_COND(
            res != VK_SUCCESS,
            Result::VULKAN_DEVICE_CREATION_FAILED,
            m_logger,
            "Failed to get number of physical devices: {}",
            res
        );

        return devices;
    }

    Result RenderingDevice::GetRequiredQueueFamilyIndices(
        VkPhysicalDevice physicalDevice,
        const std::vector<VkQueueFamilyProperties> &queueFamilies,
        const std::vector<QueueFamilyRequirements> &requirements
    ) {
        std::vector<std::vector<glm::u32> > indices(requirements.size());

        // what queue families meet what requirements?
        for (glm::u32 i = 0; i < requirements.size(); i++) {
            for (glm::u32 queueFamilyIndex = 0; queueFamilyIndex < queueFamilies.size(); queueFamilyIndex++) {
                if (DoesQueueFamilyMeetsRequirements(physicalDevice, queueFamilyIndex, queueFamilies[queueFamilyIndex], requirements[i])) {
                    indices[i].push_back(queueFamilyIndex);
                }
            }
        }

        m_requiredQueueFamiliesIndices = ConstraintMatching::ConstructSolution(indices);
        PETAL_CHECK_COND(
            m_requiredQueueFamiliesIndices.size() != requirements.size(),
            Result::VULKAN_DEVICE_CREATION_FAILED,
            m_logger,
            "Failed to apply constraint matching to meet queue family requirements.\nInput: {}\nOutput: {}",
            indices,
            m_requiredQueueFamiliesIndices
        );


        return Result::SUCCESS;
    }

    bool RenderingDevice::DoesQueueFamilyMeetsRequirements(
        VkPhysicalDevice physicalDevice,
        glm::u32 queueFamilyIndex,
        VkQueueFamilyProperties properties,
        QueueFamilyRequirements requirements
    ) const {
        // Queue is correct type(s)
        if ((properties.queueFlags & requirements.QueueType) != requirements.QueueType) return false;

        // Supports present
        if (requirements.SupportsPresenting) {
            VkBool32 supportsPresent = false;
            VkResult res = vkGetPhysicalDeviceSurfaceSupportKHR(
                physicalDevice,
                queueFamilyIndex,
                m_renderer.GetSurface(),
                &supportsPresent
            );
            if (res != VK_SUCCESS) {
                m_logger->Warn("Failed to check if queue family {} supports presenting: {}", queueFamilyIndex, res);
            }

            if (!supportsPresent) return false;
        }

        return true;
    }

    bool RenderingDevice::DoesDeviceMeetAllQueueFamilyRequirements(
        const std::vector<VkQueueFamilyProperties> &queueFamilies,
        const std::vector<QueueFamilyRequirements> &requirements
    ) const {
        std::vector<bool> metRequirements(requirements.size());

        // Check what requirements each queue family meets
        for (glm::u32 i = 0; i < requirements.size(); i++) {
            for (glm::u32 queueFamilyIndex = 0; queueFamilyIndex < queueFamilies.size(); queueFamilyIndex++) {
                if (metRequirements[i]) continue;
                metRequirements[i] = DoesQueueFamilyMeetsRequirements(
                    m_physicalDevice,
                    queueFamilyIndex,
                    queueFamilies[queueFamilyIndex],
                    requirements[i]
                );
            }
        }

        for (bool meetsRequirements : metRequirements) {
            if (!meetsRequirements) return false;
        }
        return true;
    }

    Optional<std::vector<VkQueueFamilyProperties> > RenderingDevice::GetQueueFamilyProperties(
        VkPhysicalDevice device
    ) {
        glm::u32 numberQueueFamilies = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &numberQueueFamilies, nullptr);

        PETAL_CHECK_COND(numberQueueFamilies == 0, Result::VULKAN_DEVICE_CREATION_FAILED, m_logger, "Device has no queue families");

        std::vector<VkQueueFamilyProperties> queueFamilies(numberQueueFamilies);

        // Queue family properties
        vkGetPhysicalDeviceQueueFamilyProperties(
            device,
            &numberQueueFamilies,
            queueFamilies.data()
        );

        return queueFamilies;
    }

    Result RenderingDevice::GetExtensions() {
        glm::u32 numExtensions;
        VkResult res = vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &numExtensions, nullptr);
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::VULKAN_DEVICE_CREATION_FAILED, m_logger, "Failed vkEnumerateDeviceExtensionProperties: {} (retrieving count)", res);

        std::vector<VkExtensionProperties> deviceExtensions(numExtensions);
        res = vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &numExtensions, deviceExtensions.data());
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::VULKAN_DEVICE_CREATION_FAILED, m_logger, "Failed vkEnumerateDeviceExtensionProperties: {} (count: {})", res, numExtensions);

        for (auto &extension : deviceExtensions) {
            Version version = Version::FromVulkanVersion(extension.specVersion);
            auto [it, inserted] = m_deviceExtensions.emplace(extension.extensionName, version);
            if (!inserted) {
                m_logger->Warn(
                    "Device reported duplicate extension {}. Version already in map: {}, current version: {}",
                    extension.extensionName,
                    it->second,
                    version
                );
            }
        }

        return Result::SUCCESS;
    }

    void RenderingDevice::GetDeviceFeatures(
        VkPhysicalDeviceFeatures2 &deviceFeatures,
        VkPhysicalDeviceDescriptorIndexingFeaturesEXT &indexingFeatures) const {
        indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
        indexingFeatures.pNext = nullptr;

        deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures.pNext = &indexingFeatures;

        vkGetPhysicalDeviceFeatures2(m_physicalDevice, &deviceFeatures);
    }

    Result RenderingDevice::CreateLogicalDevice(DeviceRequirements deviceRequirements) {
        std::vector deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        // Queue create infos
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(deviceRequirements.QueueFamilies.size());

        std::vector<std::vector<float> > queuePrioritiesList(deviceRequirements.QueueFamilies.size());
        for (glm::u32 i = 0; i < deviceRequirements.QueueFamilies.size(); i++) {
            queuePrioritiesList[i] = {1.0f};

            VkDeviceQueueCreateInfo queueInfo = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueFamilyIndex = m_requiredQueueFamiliesIndices[i],
                .queueCount = static_cast<glm::u32>(queuePrioritiesList[i].size()),
                .pQueuePriorities = queuePrioritiesList[i].data()
            };

            queueCreateInfos[i] = queueInfo;
        }

        // Get device features
        VkPhysicalDeviceFeatures2 queriedDeviceFeatures;
        VkPhysicalDeviceDescriptorIndexingFeaturesEXT queriedIndexingFeatures;
        GetDeviceFeatures(queriedDeviceFeatures, queriedIndexingFeatures);

        // Shader draw parameters
        static constexpr Version SHADER_DRAW_PARAMETERS_VERSION = {0, 1, 1, 0};

        VkPhysicalDeviceShaderDrawParametersFeatures shaderDrawParametersFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES,
            .pNext = nullptr,
            .shaderDrawParameters = VK_TRUE
        };

        if (m_renderingSystem.GetAPIVersion() < SHADER_DRAW_PARAMETERS_VERSION) {
            PETAL_CHECK_COND(
                !m_deviceExtensions.contains(VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME),
                Result::VULKAN_DEVICE_CREATION_FAILED,
                m_logger,
                "Shader draw parameters extension wasn't supported"
            );
            deviceExtensions.push_back(VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME);
        }

        // Dynamic rendering
        static constexpr Version DYNAMIC_RENDERING_VERSION = {0, 1, 3, 0};
        if (m_renderingSystem.GetAPIVersion() < DYNAMIC_RENDERING_VERSION) {
            PETAL_CHECK_COND(
                !m_deviceExtensions.contains(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME),
                Result::VULKAN_DEVICE_CREATION_FAILED,
                m_logger,
                "Dynamic rendering extension wasn't supported"
            );
            deviceExtensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        }

        VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
            .pNext = &shaderDrawParametersFeature,
            .dynamicRendering = VK_TRUE
        };

        // Descriptor indexing
        static constexpr Version DESCRIPTOR_INDEXING_VERSION = {0, 1, 2, 0};
        if (m_renderingSystem.GetAPIVersion() < DESCRIPTOR_INDEXING_VERSION) {
            PETAL_CHECK_COND(
                !m_deviceExtensions.contains(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME),
                Result::VULKAN_DEVICE_CREATION_FAILED,
                m_logger,
                "Descriptor indexing extension wasn't supported"
            );
            deviceExtensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
        }
        PETAL_CHECK_COND(
            !queriedIndexingFeatures.shaderStorageBufferArrayNonUniformIndexing,
            Result::VULKAN_DEVICE_CREATION_FAILED,
            m_logger,
            "indexingFeatures.shaderStorageBufferArrayNonUniformIndexing wasn't supported on the device features"
        );
        PETAL_CHECK_COND(
            !queriedIndexingFeatures.runtimeDescriptorArray,
            Result::VULKAN_DEVICE_CREATION_FAILED,
            m_logger,
            "indexingFeatures.runtimeDescriptorArray wasn't supported on the device features"
        );

        // Multi draw
        PETAL_CHECK_COND(
            !queriedDeviceFeatures.features.multiDrawIndirect,
            Result::VULKAN_DEVICE_CREATION_FAILED,
            m_logger,
            "deviceFeatures.features.multiDrawIndirect wasn't supported on the device features"
        );

        // Sync2
        VkPhysicalDeviceSynchronization2Features sync2 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES,
            .pNext = &dynamicRenderingFeature,
            .synchronization2 = VK_TRUE
        };

        // Requested indexing features
        VkPhysicalDeviceDescriptorIndexingFeaturesEXT requestedIndexingFeatures = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT,
            .pNext = &sync2
        };
        requestedIndexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
        requestedIndexingFeatures.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
        requestedIndexingFeatures.runtimeDescriptorArray = VK_TRUE;
        requestedIndexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;

        // Requested device features
        VkPhysicalDeviceFeatures2 requestedDeviceFeatures = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &requestedIndexingFeatures
        };
        requestedDeviceFeatures.features.multiDrawIndirect = VK_TRUE;
        requestedDeviceFeatures.features.drawIndirectFirstInstance = VK_TRUE;
        requestedDeviceFeatures.features.fillModeNonSolid = VK_TRUE;

        // Create device
        VkDeviceCreateInfo deviceCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &requestedDeviceFeatures,
            .flags = 0,
            .queueCreateInfoCount = static_cast<glm::u32>(queueCreateInfos.size()),
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = static_cast<glm::u32>(deviceExtensions.size()),
            .ppEnabledExtensionNames = deviceExtensions.data(),
            .pEnabledFeatures = nullptr
        };

        VkResult result = vkCreateDevice(
            m_physicalDevice,
            &deviceCreateInfo,
            nullptr,
            &m_device
        );
        PETAL_CHECK_COND(
            result != VK_SUCCESS,
            Result::VULKAN_DEVICE_CREATION_FAILED,
            m_logger,
            "Failed to create logical device: {}",
            result
        );

        return Result::SUCCESS;
    }

    Result RenderingDevice::FindRequestedGraphicsQueueFamily(const DeviceRequirements &deviceRequirements) {
        std::vector<glm::u32> graphicsQueueIndices;
        graphicsQueueIndices.reserve(deviceRequirements.QueueFamilies.size());
        for (glm::u32 i = 0; i < deviceRequirements.QueueFamilies.size(); i++) {
            bool isGraphicsQueueFamily = (deviceRequirements.QueueFamilies[i].QueueType & VK_QUEUE_GRAPHICS_BIT) != 0
                                         && deviceRequirements.QueueFamilies[i].SupportsPresenting;

            if (isGraphicsQueueFamily) {
                graphicsQueueIndices.push_back(i);
            }
        }

        PETAL_CHECK_COND(graphicsQueueIndices.empty(), Result::VULKAN_DEVICE_CREATION_FAILED, m_logger, "No graphics queues requested!");
        PETAL_CHECK_COND(graphicsQueueIndices.size() > 1, Result::VULKAN_DEVICE_CREATION_FAILED, m_logger, "Multiple graphics queues requested! Indices: {}", graphicsQueueIndices);

        m_indexOfGraphicsQueueFamily = graphicsQueueIndices.at(0);
        return Result::SUCCESS;
    }
} // Petal
