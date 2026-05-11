#include "Image.h"

#include "ImageLoaderSettings.h"
#include "stb_image.h"

namespace Petal {
    Image::Image(
        const std::shared_ptr<Logger> &logger,
        const std::filesystem::path &path,
        const ImageLoaderSettings &settings,
        Result &resultOut
    ) : m_logger(logger) {
        resultOut = LoadImage(path, settings);
    }

    Image::~Image() {
        if (m_data) stbi_image_free(m_data);
    }

    const void *Image::GetData() const {
        assert(m_data);
        return m_data;
    }

    glm::uvec2 Image::GetDimensions() const {
        return m_size;
    }

    glm::u32 Image::GetSize() const {
        return m_size.x * m_size.y * STBI_rgb_alpha;
    }

    Result Image::LoadImage(
        const std::filesystem::path &path,
        const ImageLoaderSettings &settings
    ) {
        PETAL_CHECK_COND(std::filesystem::is_directory(path), Result::PETAL_FILE_DOES_NOT_EXIST, m_logger, "Attempted to load directory {} as an image", path);
        PETAL_CHECK_COND(!std::filesystem::exists(path), Result::PETAL_FILE_DOES_NOT_EXIST, m_logger, "Image file {} does not exist", path);

        stbi_set_flip_vertically_on_load(settings.FlipVertically);
        std::string file = path.string();

        glm::i32 width, height;
        glm::i32 numChannels = 0; // number of channels originally in the image, but we pad to 4 since GPUs often do that
        m_data = stbi_load(file.c_str(), &width, &height, &numChannels, STBI_rgb_alpha);
        m_size = {width, height};

        PETAL_CHECK_COND(!m_data, Result::PETAL_IMAGE_LOADING_FAILED, m_logger, "Failed to load {}", path);

        return Result::SUCCESS;
    }
} // Petal
