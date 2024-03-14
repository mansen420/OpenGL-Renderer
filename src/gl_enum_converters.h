#pragma once
#include "global_constants.h"
#include "engine_interface.h"
inline GLenum convert(renderer::texture_filtering type)
{
    using namespace renderer;
    switch (type)
    {
    case LINEAR:
        return GL_LINEAR;
    case NEAREST:
        return GL_NEAREST;
    default:
        return GL_NONE;
    }
}
inline GLenum convert(renderer::shader_type_option type)
{
    using namespace renderer;
    switch (type)
    {
        case renderer::FRAGMENT_SHADER:
            return GL_FRAGMENT_SHADER;
        case renderer::VERTEX_SHADER:
            return GL_VERTEX_SHADER;
        case renderer::GEOMETRY_SHADER:
            return GL_GEOMETRY_SHADER;
        default:
            return GL_NONE;
    }
}