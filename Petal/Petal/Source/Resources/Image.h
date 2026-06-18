#pragma once

#include "Common.h"

namespace Petal {
    struct ImageLoaderSettings;

    class Image {
    public:
        Image(
            const std::shared_ptr<Logger> &logger,
            const std::filesystem::path &path,
            const ImageLoaderSettings &settings,
            Result &resultOut
        );

        ~Image();

    public:
        const void *GetData() const;

        glm::uvec2 GetDimensions() const;

        glm::u32 GetSize() const;

    private:
        Result LoadImage(
            const std::filesystem::path &path,
            const ImageLoaderSettings &settings
        );

    private:
        std::shared_ptr<Logger> m_logger;
        void *m_data;
        glm::ivec2 m_size;
    };
} // Petal
