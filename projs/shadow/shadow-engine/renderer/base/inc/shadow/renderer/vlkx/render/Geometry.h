#pragma once

#include <vulkan/vulkan.h>
#include <array>
#include <vector>

#define GLM_FORCE_RADIAN
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Global namespace for all classes to do with geometry in a level.
namespace Geo {

	// The core components of a given mesh.
	enum MeshType {
		Triangle, // A construction of triangles
		Quad,	  // A construction of quads
		Cube,     // A single Cube.
		Sphere	  // A single Sphere.
	};

	// Contains standard uniforms for shader files.
	struct UniformBufferObject {
		glm::mat4 model; // Model transform matrix.
		glm::mat4 view;  // View matrix.
		glm::mat4 proj;  // Projection matrix.
	};

	// All of the metadata involved with a vertex.
	struct VertexAll {
		glm::vec3 position; // XYZ coordinates of the vertex's position.
		glm::vec3 normal;   // Unit vector pointing away from the outer surface of the vertex.
		glm::vec2 texture;  // The u/v coordinates of this vertex in the bound texture.

		// How fast should vertex data be read from RAM?
		static VkVertexInputBindingDescription getBindingDesc() {
			VkVertexInputBindingDescription desc = {};
			desc.binding = 0;
			desc.stride = sizeof(VertexAll);
			desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return desc;
		}

		// How should vertexes be handled?
		static std::vector<VkVertexInputAttributeDescription> getAttributeDesc() {
			return {
                    { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, static_cast<uint32_t>(offsetof(VertexAll, position)) },
                    { 0, 1, VK_FORMAT_R32G32B32_SFLOAT, static_cast<uint32_t>(offsetof(VertexAll, normal)) },
                    { 0, 2, VK_FORMAT_R32G32_SFLOAT, static_cast<uint32_t>(offsetof(VertexAll, texture)) }
            };
		}
	};

    // All of the metadata involved with a vertex.
    struct VertexColor {
        glm::vec3 position; // XYZ coordinates of the vertex's position.
        glm::vec3 color;    // The color of the vertex.

        // How fast should vertex data be read from RAM?
        static VkVertexInputBindingDescription getBindingDesc() {
            VkVertexInputBindingDescription desc = {};
            desc.binding = 0;
            desc.stride = sizeof(VertexColor);
            desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return desc;
        }

        // How should vertexes be handled?
        static std::vector<VkVertexInputAttributeDescription> getAttributeDesc() {
            return {
                    { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, static_cast<uint32_t>(offsetof(VertexColor, position)) },
                    { 0, 1, VK_FORMAT_R32G32B32_SFLOAT, static_cast<uint32_t>(offsetof(VertexColor, color)) }
            };
        }
    };

	// Contains data about a given Mesh.
	class Mesh {
	public:
		// Pre-load the data for a triangle into the given buffers.
		static void setTriData(std::vector<VertexAll>& vertices, std::vector<uint32_t>& indices);
		// Pre-load the data for a quad into the given buffers.
		static void setQuadData(std::vector<VertexAll>& vertices, std::vector<uint32_t>& indices);
		// Pre-load the data for a cube into the given buffers.
		static void setCubeData(std::vector<VertexAll>& vertices, std::vector<uint32_t>& indices);
		// Pre-load the data for a sphere into the given buffers.
		static void setSphereData(std::vector<VertexAll>& vertices, std::vector<uint32_t>& indices);
	};


}

