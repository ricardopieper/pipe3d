#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

const float initialFoV = 45.0f;

class PerspectiveProjection {
private:
	float nearCutoff;
	float farCutoff;
	float zoomLevel = 0;
public:
	void SetCutoff(float near, float far);
	void ChangeZoom(float delta);
	glm::mat4 GetProjectionMatrix(int windowWidth, int windowHeight);
};