#pragma once
#include "ShaderType.h"
#include "slang.h"
#include "Common.h"
#include "Graphics/VertexType.h"
#include "Graphics/Resources/ShaderResource.h"

namespace Petal {
    // Represents a compiled shader that has yet to be tied to a specific graphics API
    struct IntermediateShaderResource {
        enum class ScalarType {
            FLOAT, INT, UNSIGNED_INT
        };

        struct ShaderStage {
            std::string EntryFunctionName;
            glm::u32 EntryPointIndex;
            Slang::ComPtr<slang::IBlob> SPIRV;
        };

        struct FragmentOutput {
            std::string Name;
            glm::u32 NumElements;
            ScalarType Type;
        };

        struct FragmentShaderInfo {
            std::vector<FragmentOutput> Outputs;
            // True if any of the FragmentOutput names are blank
            bool ContainsUnnamedOutputs;
        };

        std::vector<ShaderResource> Resources;
        std::unordered_map<ShaderType, ShaderStage> ShaderTypes;

        // This will be set unless no vertex shader exists in the shader
        // it contains information on vertex size and attributes
        // The vertex is assumed to be the sum of all VARYING_INPUT parameters to the vertex shader
        Optional<VertexType> VertexType = Result::PETAL_OPTIONAL_EMPTY;

        // List of fragment shader outputs
        // This will be set unless no fragment shader exists in the shader
        Optional<FragmentShaderInfo> FragmentShader = Result::PETAL_OPTIONAL_EMPTY;

        // Check if the VkFormat matches the scalar type and num elements
        static bool IsValidFormat(VkFormat format, ScalarType type, glm::u32 numElements) {
            switch (type) {
                case ScalarType::FLOAT:
                    switch (numElements) {
                        case 1:
                            return format == VK_FORMAT_R32_SFLOAT ||
                                   format == VK_FORMAT_R16_SFLOAT ||
                                   format == VK_FORMAT_R8_UNORM ||
                                   format == VK_FORMAT_R8_SNORM ||
                                   format == VK_FORMAT_R16_UNORM ||
                                   format == VK_FORMAT_R16_SNORM;

                        case 2:
                            return format == VK_FORMAT_R32G32_SFLOAT ||
                                   format == VK_FORMAT_R16G16_SFLOAT ||
                                   format == VK_FORMAT_R8G8_UNORM ||
                                   format == VK_FORMAT_R8G8_SNORM ||
                                   format == VK_FORMAT_R16G16_UNORM ||
                                   format == VK_FORMAT_R16G16_SNORM;

                        case 3:
                            return format == VK_FORMAT_R32G32B32_SFLOAT;

                        case 4:
                            return format == VK_FORMAT_R32G32B32A32_SFLOAT ||
                                   format == VK_FORMAT_R16G16B16A16_SFLOAT ||
                                   format == VK_FORMAT_R8G8B8A8_UNORM ||
                                   format == VK_FORMAT_R8G8B8A8_SNORM ||
                                   format == VK_FORMAT_B8G8R8A8_UNORM ||
                                   format == VK_FORMAT_R8G8B8A8_SRGB ||
                                   format == VK_FORMAT_B8G8R8A8_SRGB ||
                                   format == VK_FORMAT_A2R10G10B10_UNORM_PACK32 ||
                                   format == VK_FORMAT_A2B10G10R10_UNORM_PACK32;
                        default:
                            assert(false);
                    }
                    break;

                case ScalarType::INT:
                    switch (numElements) {
                        case 1:
                            return format == VK_FORMAT_R32_SINT ||
                                   format == VK_FORMAT_R16_SINT ||
                                   format == VK_FORMAT_R8_SINT;

                        case 2:
                            return format == VK_FORMAT_R32G32_SINT ||
                                   format == VK_FORMAT_R16G16_SINT ||
                                   format == VK_FORMAT_R8G8_SINT;

                        case 3:
                            return format == VK_FORMAT_R32G32B32_SINT;

                        case 4:
                            return format == VK_FORMAT_R32G32B32A32_SINT ||
                                   format == VK_FORMAT_R16G16B16A16_SINT ||
                                   format == VK_FORMAT_R8G8B8A8_SINT;
                        default:
                            assert(false);
                    }
                    break;

                case ScalarType::UNSIGNED_INT:
                    switch (numElements) {
                        case 1:
                            return format == VK_FORMAT_R32_UINT ||
                                   format == VK_FORMAT_R16_UINT ||
                                   format == VK_FORMAT_R8_UINT;

                        case 2:
                            return format == VK_FORMAT_R32G32_UINT ||
                                   format == VK_FORMAT_R16G16_UINT ||
                                   format == VK_FORMAT_R8G8_UINT;

                        case 3:
                            return format == VK_FORMAT_R32G32B32_UINT;

                        case 4:
                            return format == VK_FORMAT_R32G32B32A32_UINT ||
                                   format == VK_FORMAT_R16G16B16A16_UINT ||
                                   format == VK_FORMAT_R8G8B8A8_UINT;
                        default:
                            assert(false);
                    }
                    break;

                default:
                    assert(false && "Unsupported VkFormat");
            }

            return false;
        }
    };
} // Petal

PETAL_MAKE_ENUM_FORMATTABLE(Petal::IntermediateShaderResource::ScalarType);