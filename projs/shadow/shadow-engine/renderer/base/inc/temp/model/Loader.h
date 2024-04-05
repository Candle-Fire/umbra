#pragma once
#include <string>
#include <vector>
#include "vlkx/render/Geometry.h"

namespace vlkxtemp {

    struct Wavefront {
        Wavefront(std::string_view path, size_t base);
        Wavefront(const Wavefront&) = delete;

        Wavefront& operator=(const Wavefront&) = delete;

        std::vector<uint32_t> indices;
        std::vector<Geo::VertexAll> vertices;
    };

    class ModelLoader {
    public:
        enum class TextureType {
            Diffuse,
            Specular,
            Reflection,
            Cubemap,
            Count
        };

        struct TextureData {
            TextureData(TextureData&&) noexcept = default;
            TextureData& operator=(TextureData&&) noexcept = default;

            std::string path;
            TextureType type;
        };

        struct MeshData {
            MeshData() = default;
            MeshData(MeshData&&) noexcept = default;
            MeshData& operator=(MeshData&&) noexcept = default;

            std::vector<Geo::VertexAll> vertices;
            std::vector<uint32_t> indices;
            std::vector<TextureData> textures;
        };

        ModelLoader(const std::string& model, const std::string& textures);

        ModelLoader(const ModelLoader&) = delete;
        ModelLoader& operator=(const ModelLoader&) = delete;

        const std::vector<MeshData>& getMeshes() const { return meshes; }

    private:

        std::vector<MeshData> meshes;
    };
}