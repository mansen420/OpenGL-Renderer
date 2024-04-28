#pragma once
#include "global_constants.h"
#include "engine_interface.h"
inline GLenum convert(engine::texture_filtering type)
{
    using namespace engine;
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
inline GLenum convert(engine::shader_type_option type)
{
    using namespace engine;
    switch (type)
    {
        case engine::FRAGMENT_SHADER:
            return GL_FRAGMENT_SHADER;
        case engine::VERTEX_SHADER:
            return GL_VERTEX_SHADER;
        case engine::GEOMETRY_SHADER:
            return GL_GEOMETRY_SHADER;
        default:
            return GL_NONE;
    }
}