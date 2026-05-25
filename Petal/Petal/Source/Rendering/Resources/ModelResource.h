#pragma once

#include "Common.h"

namespace Petal {
    class Model;
    class MeshResource;

    class ModelResource {
    public:
        ModelResource(
            std::shared_ptr<Logger> logger,
            const Model &model,
            std::vector<std::unique_ptr<MeshResource> > &&uploadedMeshes
        );

    public:
        const std::vector<std::unique_ptr<MeshResource> > &GetMeshes() const;

    private:
        std::shared_ptr<Logger> m_logger;
        std::vector<std::unique_ptr<MeshResource> > m_uploadedMeshes;
    };
} // Petal
