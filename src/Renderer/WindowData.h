#pragma once

#include "RenderingPipeline.h"
#include "Framebuffer.h"

struct WindowData {
    PerspectiveProjection* perspectiveProjection;
    RenderingPipeline* renderingPipeline;
};