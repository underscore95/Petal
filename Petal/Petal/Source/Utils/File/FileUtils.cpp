#include "FileUtils.h"

namespace Petal {
    std::string FileUtils::Read(
        const std::filesystem::path &path,
        Optional<std::shared_ptr<Logger> > logger
    ) {
        std::ifstream stream(path, std::ios::binary | std::ios::ate);
        if (!stream) {
            if (logger.HasValue()) (*logger)->Error("Failed to open file for reading: {}", path);
            return "";
        }

        std::streamsize size = stream.tellg();
        std::string out(size, '\0');

        stream.seekg(0, std::ios::beg);
        if (!stream.read(out.data(), size)) {
            if (logger.HasValue()) (*logger)->Error("Error occurred while reading: {}", path);
            return "";
        }

        return out;
    }
} // Petal
