#pragma once

#include "Common.h"

namespace Petal {
    enum class ShaderType {
        VERTEX,
        FRAGMENT,
        COUNT
    };

    class ShaderTypes {
        ShaderTypes() = delete;

    public:
        struct EnumData {
            VkShaderStageFlags VulkanShaderStage;
        };

    public:
        static VkShaderStageFlags CombineVulkanFlags(const std::vector<ShaderType> &shaderTypes);
        static const EnumData& GetData(ShaderType shaderType);

    private:
        static const std::array<EnumData, static_cast<size_t>(ShaderType::COUNT)> Data;
    };
}

PETAL_MAKE_ENUM_FORMATTABLE(Petal::ShaderType);