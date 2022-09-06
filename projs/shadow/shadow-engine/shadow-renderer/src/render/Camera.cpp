#include <vlkx/render/Camera.h>

using namespace vlkx;

Camera& Camera::move(const glm::vec3 &delta) {
    position += delta;
    return *this;
}

Camera &Camera::setPos(const glm::vec3 &pos) {
    position = pos;
    return *this;
}

Camera &Camera::up(const glm::vec3 &up) {
    upVector = glm::normalize(up);
    return *this;
}

Camera &Camera::forward(const glm::vec3 &forward) {
    frontVector = glm::normalize(forward);
    rightVector = glm::normalize(glm::cross(frontVector, upVector));
    return *this;
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, position + frontVector, upVector);
}

PerspectiveCamera &PerspectiveCamera::fieldOfView(float newFov) {
    fov = newFov;
    return *this;
}

PerspectiveCamera::RT PerspectiveCamera::getRT() const {
    const glm::vec3 upVec = glm::normalize(glm::cross(getRight(), getForward()));
    const float fovTan = glm::tan(glm::radians(fov));
    return { upVec * fovTan, getForward(), getRight() * fovTan * aspectRatio };
}

glm::mat4 PerspectiveCamera::getProjMatrix() const {
    glm::mat4 proj = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
    proj[1][1] = -proj[1][1];
    return proj;
}

OrthographicCamera &OrthographicCamera::setWidth(float vWidth) {
    width = vWidth;
    return *this;
}

glm::mat4 OrthographicCamera::getProjMatrix() const {
    const float height = width / aspectRatio;
    const auto halfSize = glm::vec2 { width, height } / 2.0f;
    return glm::ortho(-halfSize.x, halfSize.x, -halfSize.y, halfSize.y, nearPlane, farPlane);
}

template <typename Type>
void UserCamera<Type>::setInternal(std::function<void(Type *)> op) {
    op(camera.get());
    reset();
}

template <typename Type>
void UserCamera<Type>::move(double x, double y) {
    if (!isActive) return;

    const auto offsetX = static_cast<float>(x * config.turnSpeed);
    const auto offsetY = static_cast<float>(y * config.turnSpeed);

    pitch = glm::clamp(pitch - offsetY, glm::radians(-89.9f), glm::radians(89.9f));
    yaw = glm::mod(yaw - offsetX, glm::radians(360.0f));
    camera->forward( { glm::cos(pitch) * glm::cos(yaw), glm::sin(pitch), glm::cos(pitch) * glm::sin(yaw) });
}

template <typename Type>
bool UserCamera<Type>::scroll(double delta, double min, double max) {
    if (!isActive) return false;

    if constexpr (std::is_same_v<Type, PerspectiveCamera>) {
        auto newFov = (float) glm::clamp(camera->getFieldOfView() + delta, min, max);
        if (newFov != camera->getFieldOfView()) {
            camera->fieldOfView(newFov);
            return true;
        }
    } else if constexpr (std::is_same_v<Type, OrthographicCamera>) {
        const auto newWidth = (float) glm::clamp(camera->getWidth() + delta, min, max);
        if (newWidth != camera->getWidth()) {
            camera->setWidth(newWidth);
            return true;
        }
    } else {
        static_assert("Unhandled Camera Type");
    }

    return false;
}

template <typename Type>
void UserCamera<Type>::press(Camera::Input key, float time) {
    using Key = Camera::Input;
    if (!isActive) return;

    if (!config.center.has_value()) {
        const float distance = time * config.moveSpeed;
        switch (key) {
            case Key::Up:
                camera->move(+camera->getForward() * distance); break;
            case Key::Down:
                camera->move(-camera->getForward() * distance); break;
            case Key::Left:
                camera->move(-camera->getRight() * distance); break;
            case Key::Right:
                camera->move(+camera->getRight() * distance); break;
        }
    } else {
        reset();
    }
}

template <typename Type>
void UserCamera<Type>::reset() {
    refForward = camera->getForward();
    refLeft = -camera->getRight();
    pitch = yaw = 0;
}

template class vlkx::UserCamera<PerspectiveCamera>;
template class vlkx::UserCamera<OrthographicCamera>;