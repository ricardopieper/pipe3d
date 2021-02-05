#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>

class OrthoProjection {
private:
	float left; 
	float right; 
	float bottom; 
	float top; 
	float zNear; 
	float zFar;

public:
	OrthoProjection(float left, float right, float bottom, float top, float zNear, float zFar);

	glm::mat4 GetProjectionMatrix();
};