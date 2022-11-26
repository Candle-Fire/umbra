#version 460 core

layout(binding = 1) uniform samplerCube tex;

layout(location = 0) in vec3 tex_coord;
layout(location = 0) out vec4 frag_color;

void main() {
    frag_color = texture(tex, tex_coord);
}