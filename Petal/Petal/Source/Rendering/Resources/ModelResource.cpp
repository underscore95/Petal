#include "ModelResource.h"
#include "MeshResource.h"

namespace Petal {
    ModelResource::ModelResource(
        std::shared_ptr<Logger> logger,
        const Model &model,
        std::vector<std::unique_ptr<MeshResource> > &&uploadedMeshes
    ) : m_logger(logger),
        m_uploadedMeshes(std::move(uploadedMeshes)) {
    }

    const std::vector<std::unique_ptr<MeshResource> > &ModelResource::GetMeshes() const {
        return m_uploadedMeshes;
    }
} // Petal
