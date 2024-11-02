#version 450

#pragma vscode_glsllint_stage : frag

layout(location = 0) in vec2 ipos;
layout(location = 0) out vec4 color;

layout(binding = 1) uniform UniformT {
    vec2 ligthPosition;
} uniformBuffer;

void main() {
    float distance = length(uniformBuffer.ligthPosition - ipos);
    color = vec4(cos(ipos.x * 3.4), 0.5, sin(ipos.y * 3.4), 1.0);
    color *= vec4(1.0 / distance) * 0.05;
}