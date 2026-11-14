#version 440

layout(location = 0) in vec2 vTexCoord;
layout(location = 1) in vec2 vPos;

layout(location = 0) out vec4 fragColor;

layout(binding = 1) uniform sampler2D source;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float radius;
    vec2 itemSize;
};

float roundedBoxSDF(vec2 p, vec2 b, float r)
{
    float rr = min(r, min(b.x, b.y));
    vec2 q = abs(p) - b + vec2(rr);
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - rr;
}

void main()
{
    vec4 c = texture(source, vTexCoord) * qt_Opacity;
    // Compute position from texture coordinates to keep the rounded-rect mask
    // aligned with the textured quad (avoids vertex-space coordinate issues).
    vec2 halfSize = itemSize * 0.5;
    vec2 p = vTexCoord * itemSize - halfSize;
    float d = roundedBoxSDF(p, halfSize, radius);
    float alpha = 1.0 - smoothstep(-0.75, 0.75, d);
    fragColor = c * alpha;
}
