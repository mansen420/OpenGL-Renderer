#pragma once

#include "glad/glad.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "global_constants.h"

//render settings
extern bool DEPTH_CLR_ENBLD, COLOR_CLR_ENBLD;
extern bool DEPTH_TEST_ENBLD;
extern glm::vec4 CLR_COLOR;

namespace renderer
{
    void clear_buffers();
    void render_scene();
    void init();
}