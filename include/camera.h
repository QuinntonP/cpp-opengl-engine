#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

// Default camera values
constexpr float YAW         = -90.0f;
constexpr float PITCH       =  0.0f;
constexpr float SPEED       =  15.0f;
constexpr float SENSITIVITY =  0.1f;
constexpr float ZOOM        = 45.0f;

enum class CameraMovement {
    Forward,
    Backward,
    Left,
    Right,
    UP,
    DOWN
};

class Camera {
public:
    // Camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // Euler Angles
    float Yaw;
    float Pitch;

    // Options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    Camera(
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
        glm::vec3 up       = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw          = YAW,
        float pitch        = PITCH
    )
        : Position(position),
          WorldUp(up),
          Yaw(yaw),
          Pitch(pitch),
          MovementSpeed(SPEED),
          MouseSensitivity(SENSITIVITY),
          Zoom(ZOOM)
    {
        updateCameraVectors();
    }

    glm::mat4 GetViewMatrix() const {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void ProcessKeyboard(CameraMovement direction, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
        if (direction == CameraMovement::Forward)  Position += Front * velocity;
        if (direction == CameraMovement::Backward) Position -= Front * velocity;
        if (direction == CameraMovement::Left)     Position -= Right * velocity;
        if (direction == CameraMovement::Right)    Position += Right * velocity;
        if (direction == CameraMovement::UP)       Position += Up * velocity;
        if (direction == CameraMovement::DOWN)     Position -= Up * velocity;
    }

    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw   += xoffset;
        Pitch += yoffset;

        if (constrainPitch) {
            Pitch = std::clamp(Pitch, -89.0f, 89.0f);
        }

        updateCameraVectors();
    }

    void ProcessMouseScroll(float yoffset) {
        Zoom -= yoffset;
        Zoom = std::clamp(Zoom, 1.0f, 45.0f);
    }

private:
    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);

        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up    = glm::normalize(glm::cross(Right, Front));
    }
};
