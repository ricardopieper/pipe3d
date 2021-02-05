#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>

class Camera3D {
private:


	glm::vec3 getDirection();
	glm::vec3 getRight();

public:
	glm::vec3 Position;
	float horizontalAngle = 3.14f;
	float verticalAngle = 0.0f;

	void MoveForward(float factor);
	void MoveBackward(float factor);
	void MoveRight(float factor);
	void MoveLeft(float factor);

	void ChangeVerticalAngle(float delta);
	void ChangeHorizontalAngle(float delta);

	glm::mat4 GetViewMatrix();

	void ResetAngle();
};