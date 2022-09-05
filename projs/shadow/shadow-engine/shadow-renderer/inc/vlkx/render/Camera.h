#pragma once

#define GLM_FORCE_RADIAN
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:

	// Set up the camera.
	void init(float fov, float width, float height, float near, float far);

	// Move the camera.
	void setPosition(glm::vec3 positionIn);

	glm::mat4 getViewMatrix() { return viewMatrix; };
	glm::mat4 getProjectionMatrix() { return projectionMatrix; }

private:
	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;
	glm::vec3 position;
};