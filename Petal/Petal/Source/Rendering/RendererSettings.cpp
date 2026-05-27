#include "RendererSettings.h"

#define SETTINGS_NAME "RendererSettings"
#include "CheckSettingsMacro.h"

namespace Petal {
    Result RendererSettings::Validate(const std::shared_ptr<Logger> &logger) {
        return Result::SUCCESS;
    }
} // Petal
