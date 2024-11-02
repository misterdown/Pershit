#version 450

#pragma vscode_glsllint_stage : vert

layout(binding = 0) uniform UniformT {
    vec2 cameraPosition;
    vec2 cameraScale;
} uniformBuffer;

layout(location = 0) in vec2 ipos;
layout(location = 0) out vec2 opos;
void main() {
    gl_Position = vec4((ipos  - uniformBuffer.cameraPosition) / uniformBuffer.cameraScale, 0.0f, 1.0f);
    opos = ipos;
}