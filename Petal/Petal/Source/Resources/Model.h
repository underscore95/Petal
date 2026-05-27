#pragma once
#include "Graphics/VertexType.h"

namespace Petal {
    class MeshBuilder;

    class Model {
    public:
        struct Section {
            std::unique_ptr<MeshBuilder> Mesh;
            std::filesystem::path DiffuseTexture;
        };

        struct Settings {
            VertexType VertexType;
        };

    public:
        Model(
            const std::shared_ptr<Logger> &logger,
            const std::filesystem::path &path,
            const Settings &settings,
            Result &resultOut
        );

    public:
        const std::vector<Section> &GetSections() const;

    private:
        Result LoadModel(
            const std::shared_ptr<Logger> &logger,
            const std::filesystem::path &path,
            const Settings &settings
        );

    private:
        std::vector<Section> m_sections;
    };
} // Petal
