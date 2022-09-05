#pragma once

#include <vlkx/render/shader/Pipeline.h>
#include <vlkx/render/shader/GeoBuffers.h>
#include <vlkx/render/shader/Descriptor.h>
#include <vlkx/render/Camera.h>

// Renders a single object.
class SingleRenderer {
public:

	void createSingleRenderer(Geo::MeshType type, glm::vec3 posIn, glm::vec3 scaleIn);

	void updateUniforms(Camera camera);

	void draw();

	void destroy();

	void setPosition(glm::vec3 newPos) { position = newPos; }
	void setRotation(glm::mat4 newRot) { rotation = newRot; }

    Descriptor getDescriptor() { return descriptor; }

	glm::mat4 getRotation() { return rotation; }
private:

	Pipeline pipeline;
	GeoBuffers buffers;
	Descriptor descriptor;

	glm::vec3 position;
	glm::vec3 scale;
	glm::mat4 rotation;
};