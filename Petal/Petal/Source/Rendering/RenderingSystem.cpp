#include "RenderingSystem.h"

namespace Petal {
    RenderingSystem::RenderingSystem(
        Engine &engine
    ) : m_engine(engine) {
        VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = ,
        .pApplicationInfo = ,
        .enabledLayerCount = ,
        .ppEnabledLayerNames = ,
        .enabledExtensionCount = ,
        .ppEnabledExtensionNames = }
        vkCreateInstance(&info, nullptr);
    }

    RenderingSystem::~RenderingSystem() {
    }
} // Petal
