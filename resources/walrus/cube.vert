#version 450

layout(std140, push_constant) uniform Transform {
    mat4 matrix;
} transf;

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out vec2 texCoord;

void main() {
    gl_Position = transf.matrix * vec4(in_pos, 1.0);
    texCoord = in_uv;
}