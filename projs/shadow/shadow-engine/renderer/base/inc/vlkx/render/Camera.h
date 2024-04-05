#pragma once

#include <optional>
#define GLM_FORCE_RADIAN
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <functional>
#include <memory>

namespace vlkx {
    class Camera {
    public:

        enum class Input {
            Up, Down, Left, Right
        };
        struct Movement {
            float moveSpeed = 10;
            float turnSpeed = 0.0005f;
            std::optional<glm::vec3> center;
        };

        /**
         * Camera configuration; with defaults.
         * Left and forward vectors are calculated from up, pos and target.
         */
        struct Config {
            float nearPlane = 0.1f;                     // The nearest a vertex can be to the camera before being clipped
            float farPlane = 100;                      // The furthest a vertex can be from the camera before clipped
            glm::vec3 upV{0, 1, 0};        // The vector pointing straight up from the camera
            glm::vec3 pos{0, 0, 0};        // The position of the camera in the world
            glm::vec3 target{1, 0, 0};     // The point the camera is looking at
        };

        Camera(const Camera &) = delete;

        Camera &operator=(const Camera &) = delete;

        virtual ~Camera() = default;

        Camera &move(const glm::vec3 &delta);

        Camera &setPos(const glm::vec3 &pos);

        Camera &up(const glm::vec3 &up);

        Camera &forward(const glm::vec3 &forward);

        glm::mat4 getViewMatrix() const;

        glm::mat4 getSkyboxView() const {
            return glm::mat4{glm::mat3{getViewMatrix()}};
        }

        virtual glm::mat4 getProjMatrix() const = 0;

        const glm::vec3& getPosition() const { return position; }
        const glm::vec3& getUp() const { return upVector; }
        const glm::vec3& getForward() const { return frontVector; }
        const glm::vec3& getRight() const { return rightVector; }

    protected:
        explicit Camera(const Config &conf) : nearPlane(conf.nearPlane), farPlane(conf.farPlane), position(conf.pos),
                                              upVector(glm::normalize(conf.upV)) {
            forward(conf.target - position);
        }

        const float nearPlane;
        const float farPlane;
    private:

        glm::vec3 position;
        glm::vec3 upVector;
        glm::vec3 frontVector;
        glm::vec3 rightVector;

    };

    class PerspectiveCamera : public Camera {
    public:

        struct Frustum {
            float fov;
            float aspect;
        };

        struct RT {
            glm::vec3 up;
            glm::vec3 forward;
            glm::vec3 right;
        };

        PerspectiveCamera(const Camera::Config &conf, const Frustum &frus) :
                Camera(conf), aspectRatio(frus.aspect), fov(frus.fov) {}

        PerspectiveCamera(const PerspectiveCamera &) = delete;

        PerspectiveCamera &operator=(const PerspectiveCamera &) = delete;

        PerspectiveCamera &fieldOfView(float newFov);

        RT getRT() const;

        glm::mat4 getProjMatrix() const override;

        float getFieldOfView() const { return fov; }

        float getAspect() const { return aspectRatio; }

    private:

        const float aspectRatio;
        float fov;
    };

    class OrthographicCamera : public Camera {
    public:
        struct OrthoConfig {
            float width;
            float aspect;
        };

        static OrthoConfig getFullscreenConfig() {
            return {2, 1};
        }

        OrthographicCamera(const Camera::Config &config, const OrthoConfig &ortho)
                : Camera(config), aspectRatio(ortho.aspect), width(ortho.width) {}

        OrthographicCamera(const OrthographicCamera &) = delete;

        OrthographicCamera &operator=(const OrthographicCamera &) = delete;

        OrthographicCamera &setWidth(float width);

        glm::mat4 getProjMatrix() const override;

        float getWidth() const { return width; }

    private:

        const float aspectRatio;
        float width;
    };

    template<typename Type>
    class UserCamera {
    public:
        UserCamera(const UserCamera &) = delete;

        UserCamera &operator=(const UserCamera &) = delete;

        virtual ~UserCamera() = default;

        void setInternal(std::function<void(Type *)> op);

        void setPos(const glm::dvec2 &pos) { cursorPos = pos; }

        void move(double x, double y);

        bool scroll(double delta, double min, double max);

        void press(Camera::Input key, float time);

        void active(bool active) { isActive = active; }

        const Type &getCamera() const { return *camera; }

        UserCamera(const Camera::Movement &movement, std::unique_ptr<Type> &&cam)
                : config(movement), camera(std::move(cam)) {
            reset();
        }

        void reset();

    private:


        const Camera::Movement config;
        bool isActive = false;

        std::unique_ptr<Type> camera;
        glm::dvec2 cursorPos;
        glm::vec3 refForward;
        glm::vec3 refLeft;

        float pitch;
        float yaw;
    };

    class UserPerspectiveCamera : public UserCamera<PerspectiveCamera> {
    public:
        static std::unique_ptr<UserPerspectiveCamera>
        create(const Camera::Movement &movement, const Camera::Config &config,
               const PerspectiveCamera::Frustum &frustum) {
            return std::make_unique<UserPerspectiveCamera>(movement,
                                                           std::make_unique<PerspectiveCamera>(config, frustum));
        }

    protected:
        using UserCamera<PerspectiveCamera>::UserCamera;
    };

    class UserOrthoCamera : public UserCamera<OrthographicCamera> {
    public:
        static std::unique_ptr<UserOrthoCamera> create(const Camera::Movement &movement, const Camera::Config &config,
                                                       const OrthographicCamera::OrthoConfig &ortho) {
            return std::make_unique<UserOrthoCamera>(movement, std::make_unique<OrthographicCamera>(config, ortho));
        }

    protected:
        using UserCamera<OrthographicCamera>::UserCamera;
    };
}