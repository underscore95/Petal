#include "ShaderType.h"

namespace Petal {
    const std::array<ShaderTypes::EnumData, static_cast<size_t>(ShaderType::COUNT)> ShaderTypes::Data = {
        EnumData{VK_SHADER_STAGE_VERTEX_BIT},
        EnumData{VK_SHADER_STAGE_FRAGMENT_BIT}
    };

    VkShaderStageFlags ShaderTypes::CombineVulkanFlags(const std::vector<ShaderType> &shaderTypes) {
        VkShaderStageFlags flags = 0;
        for (ShaderType type : shaderTypes) {
            flags |= GetData(type).VulkanShaderStage;
        }
        return flags;
    }

    const ShaderTypes::EnumData &ShaderTypes::GetData(ShaderType shaderType) {
        return Data[static_cast<size_t>(shaderType)];
    }
}