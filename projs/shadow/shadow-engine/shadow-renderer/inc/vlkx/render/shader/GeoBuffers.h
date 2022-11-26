#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include <vlkx/render/Geometry.h>
#include <vlkx/vulkan/Tools.h>

// Contains memory and objects required to store information about geometry.
class GeoBuffers {
public:

	GeoBuffers();
	~GeoBuffers();

	// Program and virtual memory for vertex data.
	std::vector<Geo::Vertex> vertices;
	VkTools::ManagedBuffer vertexBuffer;

	// Program and virtual memory for indices data.
	std::vector<uint32_t> indices;
    VkTools::ManagedBuffer indexBuffer;

	// Virtual memory for uniforms - translation matrices, etc.
    VkTools::ManagedBuffer uniformBuffer;

	void createBuffers(Geo::MeshType type);

	void destroy();

private:

	void createVertexBuffer();
	void createIndexBuffer();
	void createUniformBuffer();

};