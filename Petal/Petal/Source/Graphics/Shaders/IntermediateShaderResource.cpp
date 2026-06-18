#include "IntermediateShaderResource.h"

#include "Graphics/Internal/ITexture.h"
#include "Graphics/Internal/RenderTarget.h"

bool Petal::IntermediateShaderResource::FragmentShaderInfo::MatchesRenderTarget(
    const RenderTarget &renderTarget,
    OptionalRef<const std::shared_ptr<Logger>> logger
) const {
    assert(logger.IsEmpty() || *logger);

    if (renderTarget.Colors.size() != Outputs.size()) {
        if (logger.HasValue()) {
            (*logger)->Warn("RenderTarget has {} color attachments, but the fragment shader has {} outputs", renderTarget.Colors.size(), Outputs.size());
        }
        return false;
    }

    for (size_t i = 0; i < Outputs.size(); i++) {
        VkFormat format = renderTarget.Colors[i].Texture->GetFormat();
        if (IsValidFormat(format, Outputs[i].Type, Outputs[i].NumElements)) continue;

        if (logger.HasValue()) {
            (*logger)->Warn(
                "RenderTarget color attachment at index {} is in format {} but the fragment shader output at this index is {}x {} (Output Name: {}))",
                i, format, Outputs[i].NumElements, Outputs[i].Type, Outputs[i].Name
            );
        }
        return false;
    }

    return true;
}

bool Petal::IntermediateShaderResource::IsValidFormat(VkFormat format, ScalarType type, glm::u32 numElements) {
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
