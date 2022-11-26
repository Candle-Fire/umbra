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
	struct Vertex {
		glm::vec3 position; // XYZ coordinates of the vertex's position.
		glm::vec3 normal;   // Unit vector pointing away from the outer surface of the vertex.
		glm::vec3 color;    // The color of the vertex.
		glm::vec2 texture;  // The u/v coordinates of this vertex in the bound texture.

		// How fast should vertex data be read from RAM?
		static VkVertexInputBindingDescription getBindingDesc() {
			VkVertexInputBindingDescription desc = {};
			desc.binding = 0;
			desc.stride = sizeof(Vertex);
			desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return desc;
		}

		// How should vertexes be handled?
		static std::array<VkVertexInputAttributeDescription, 4> getAttributeDesc() {
			std::array<VkVertexInputAttributeDescription, 4> descs = {};

			// Attribute 0; position. Location 0, 3x 32-bit float.
			descs[0].binding = 0;
			descs[0].location = 0;
			descs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			descs[0].offset = offsetof(Vertex, position);

			// Attribute 1; normal. Location 1, 3x 32-bit float.
			descs[1].binding = 0;
			descs[1].location = 1;
			descs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			descs[1].offset = offsetof(Vertex, normal);

			// Attribute 2; color. Location 2, 3x 32-bit float.
			descs[2].binding = 0;
			descs[2].location = 2;
			descs[2].format = VK_FORMAT_R32G32B32_SFLOAT;
			descs[2].offset = offsetof(Vertex, color);

			// Attribute 3; texture. Location 3, 2x 32-bit float.
			descs[3].binding = 0;
			descs[3].location = 3;
			descs[3].format = VK_FORMAT_R32G32_SFLOAT;
			descs[3].offset = offsetof(Vertex, texture);

			return descs;
		}
	};

	// Contains data about a given Mesh.
	class Mesh {
	public:
		// Pre-load the data for a triangle into the given buffers.
		static void setTriData(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
		// Pre-load the data for a quad into the given buffers.
		static void setQuadData(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
		// Pre-load the data for a cube into the given buffers.
		static void setCubeData(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
		// Pre-load the data for a sphere into the given buffers.
		static void setSphereData(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	};


}

