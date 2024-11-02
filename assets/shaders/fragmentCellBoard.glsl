#version 450

#pragma vscode_glsllint_stage : frag

layout(location = 0) in vec2 ipos;
layout(location = 0) out vec4 color;

void main() {
    vec2 cellBoardDiff = ipos * 6.0f - floor(ipos * 6.0f);
    if (cellBoardDiff.x < 0.95 && cellBoardDiff.y < 0.95)
        discard;
    vec3 layer1 = vec3(
        cos(ipos.x),
        cos(ipos.y),
        cos(ipos.y + ipos.x)
    );
    vec3 layer2 = vec3(
        cos(ipos.x * 3.4),
        0.5,
        sin(ipos.y * 3.4)
    );
    vec3 total = (layer1 + layer2) / 2.0f;
    total *= total;
    total /= vec3(2.0f);
    total += vec3(0.05f);
    color = vec4(total, 1.0);
}