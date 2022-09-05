#include <vlkx/render/Camera.h>

void Camera::init(float fov, float width, float height, float near, float far) {

	// Initialise members
	position = glm::vec3(0, 0, 4);
	
	glm::vec3 cameraFront = glm::vec3(0, 0, 0);
	glm::vec3 cameraUp = glm::vec3(0, 1, 0);

	viewMatrix = glm::mat4(1);
	projectionMatrix = glm::mat4(1);

	projectionMatrix = glm::perspective(fov, width / height, near, far);
	viewMatrix = glm::lookAt(position, cameraFront, cameraUp);
}

void Camera::setPosition(glm::vec3 newPosition) {
	position = newPosition;
}