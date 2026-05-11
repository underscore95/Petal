#include "RendererSettings.h"

#define SETTINGS_NAME "RendererSettings"
#include "CheckSettingsMacro.h"

namespace Petal {
    Result RendererSettings::Validate(const std::shared_ptr<Logger> &logger) {
        CHECK_SETTINGS(VertexBufferShaderName.empty(), "VertexBufferShaderName is empty");
        CHECK_SETTINGS(IndexBufferShaderName.empty(), "IndexBufferShaderName is empty");

        return Result::SUCCESS;
    }
} // Petal
