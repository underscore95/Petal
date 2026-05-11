#include "TextureCreateInfo.h"

#define SETTINGS_NAME "TextureCreateInfo"
#include "CheckSettingsMacro.h"

namespace Petal {
    Result TextureCreateInfo::Validate(const std::shared_ptr<Logger>& logger) const {
        CHECK_SETTINGS(Size.x == 0, "Texture width is 0");
        CHECK_SETTINGS(Size.y == 0, "Texture height is 0");
        CHECK_SETTINGS(Size.z == 0, "Texture depth is 0");

        return Result::SUCCESS;
    }
} // Petal