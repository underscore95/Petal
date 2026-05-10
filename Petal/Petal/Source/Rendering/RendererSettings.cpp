#include "RendererSettings.h"

#define CHECK_SETTINGS(cond, fmt, ...) \
    PETAL_CHECK_COND( \
        cond, \
        Result::PETAL_INVALID_SETTINGS, \
        logger, \
        "Invalid RendererSettings! {}", \
        std::format(fmt, ##__VA_ARGS__)                  \
    )

namespace Petal {
    Result RendererSettings::Validate(const std::shared_ptr<Logger> &logger) {
        CHECK_SETTINGS(VertexBufferShaderName.empty(), "VertexBufferShaderName is empty");
        CHECK_SETTINGS(IndexBufferShaderName.empty(), "IndexBufferShaderName is empty");

        return Result::SUCCESS;
    }
} // Petal
