#include "PerspectiveProjection.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

void PerspectiveProjection::ChangeZoom(float delta)
{
	zoomLevel += delta;
}

void PerspectiveProjection::SetCutoff(float near, float far)
{
	this->nearCutoff = near;
	this->farCutoff = far;
}

glm::mat4 PerspectiveProjection::GetProjectionMatrix(int windowWidth, int windowHeight)
{
	float fov = initialFoV - 5 * zoomLevel;
	return glm::perspective(glm::radians(fov), (float)windowWidth / (float)windowHeight, nearCutoff, farCutoff);
}

