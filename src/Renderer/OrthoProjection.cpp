#include "OrthoProjection.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

OrthoProjection::OrthoProjection(float left, float right, float bottom, float top, float zNear, float zFar)
	: left(left), right(right), bottom(bottom), top(top), zNear(zNear), zFar(zFar) {
	
}

glm::mat4 OrthoProjection::GetProjectionMatrix()
{
	return glm::ortho(left, right, bottom, top, zNear, zFar);	
}
