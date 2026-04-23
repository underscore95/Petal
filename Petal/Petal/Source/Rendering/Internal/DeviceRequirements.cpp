#include "DeviceRequirements.h"

namespace Petal {
    DeviceRequirements DeviceRequirements::DEFAULT() {
        return {
            .APIVersion = DEFAULT_API_VERSION,
            .QueueFamilies = {
                {
                    .QueueType = VK_QUEUE_GRAPHICS_BIT,
                    .SupportsPresenting = true
                }
            }
        };
    }
}
