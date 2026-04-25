#pragma once

#include "Common.h"

namespace Petal {
    class FileUtils {
    public:
        // Read contents of file into a string
        // If fails, returns an empty string and logs an error if a logger is passed in
        static std::string Read(
            const std::filesystem::path &path,
            Optional<std::shared_ptr<Logger> > logger = Optional<std::shared_ptr<Logger> >::Empty()
        );
    };
} // Petal
