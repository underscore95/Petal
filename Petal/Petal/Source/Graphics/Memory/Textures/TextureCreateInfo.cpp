#include "TextureCreateInfo.h"

#define SETTINGS_NAME "TextureCreateInfo"
#include "CheckSettingsMacro.h"

namespace Petal {
    Result TextureCreateInfo::Validate(const std::shared_ptr<Logger> &logger) const {
        CHECK_SETTINGS(Size.x == 0, "Texture width is 0");
        CHECK_SETTINGS(Size.y == 0, "Texture height is 0");
        CHECK_SETTINGS(Size.z == 0, "Texture depth is 0");

        if (ImageType == VK_IMAGE_TYPE_1D) {
            CHECK_SETTINGS(Size.y != 1, "1D texture but height is {}", Size.y);
            CHECK_SETTINGS(Size.z != 1, "1D texture but width is {}", Size.z);
            CHECK_SETTINGS(
                ViewType != VK_IMAGE_VIEW_TYPE_1D &&ViewType != VK_IMAGE_VIEW_TYPE_1D_ARRAY,
                "1D texture but image view type is {}", ViewType
            );
        } else if (ImageType == VK_IMAGE_TYPE_2D) {
            CHECK_SETTINGS(Size.z != 1, "2D texture but width is {}", Size.z);
            CHECK_SETTINGS(
                ViewType != VK_IMAGE_VIEW_TYPE_2D && ViewType != VK_IMAGE_VIEW_TYPE_2D_ARRAY && ViewType != VK_IMAGE_VIEW_TYPE_CUBE && ViewType != VK_IMAGE_VIEW_TYPE_CUBE_ARRAY,
                "2D texture but image view type is {}", ViewType
            );
        } else {
            CHECK_SETTINGS(
                ViewType != VK_IMAGE_VIEW_TYPE_3D,
                "3D texture but image view type is {}", ViewType
            );
        }

        if (ViewType == VK_IMAGE_VIEW_TYPE_CUBE || ViewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY) {
            CHECK_SETTINGS(Size.x != Size.y, "Image view type is {} but width ({}) != height ({})", ViewType, Size.x, Size.y);
        }

        return Result::SUCCESS;
    }
} // Petal
