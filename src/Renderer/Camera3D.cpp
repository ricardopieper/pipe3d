#include "Camera3D.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>

#include <iostream>

glm::mat4 Camera3D::GetViewMatrix() {
    glm::vec3 right = getRight();
    glm::vec3 direction = getDirection();
    glm::vec3 up = glm::cross(right, direction);
    return this->LookAt(this->Position + direction, up);
};

glm::mat4 Camera3D::LookAt(glm::vec3 at, glm::vec3 up) {
    glm::mat4 cameraLookAt = glm::lookAt(
        this->Position,
        at,
        up
    );
    return cameraLookAt;
};


glm::vec3 Camera3D::getDirection()
{
    return glm::vec3(
        cos(verticalAngle) * sin(horizontalAngle),
        sin(verticalAngle),
        cos(verticalAngle) * cos(horizontalAngle)
    );
}

glm::vec3 Camera3D::getRight()
{
    return glm::vec3(
        sin(horizontalAngle - 3.14f / 2.0f),
        0,
        cos(horizontalAngle - 3.14f / 2.0f)
    );
}

void Camera3D::MoveForward(float factor)
{
    this->Position += getDirection() * factor;
}

void Camera3D::MoveBackward(float factor)
{
    this->Position -= getDirection() * factor;
}

void Camera3D::MoveRight(float factor)
{
    this->Position += getRight() * factor;
}

void Camera3D::MoveLeft(float factor)
{
    this->Position -= getRight() * factor;
}

void Camera3D::ChangeVerticalAngle(float delta)
{
    this->verticalAngle += delta;
}

void Camera3D::ChangeHorizontalAngle(float delta)
{
    this->horizontalAngle += delta;
}

void Camera3D::ResetAngle()
{
    this->horizontalAngle = 3.14f;
    this->verticalAngle = 0;
}