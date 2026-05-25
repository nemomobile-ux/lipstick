#version 440

layout(location = 0) in vec2 vTexCoord;
layout(location = 1) in vec2 vCorner;

layout(location = 0) out vec4 fragColor;

layout(binding = 1) uniform sampler2D qt_Texture;

layout(std140, binding = 0) uniform buf
{
    mat4 qt_Matrix;
    float qt_Opacity;
    vec2 radius;
};

void main()
{
    fragColor =
        texture(qt_Texture, vTexCoord)
        * smoothstep(radius.x,
                     radius.y,
                     dot(vCorner, vCorner))
        * qt_Opacity;
}
