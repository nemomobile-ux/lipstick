#version 440

layout(location = 0) in vec2 vTexCoord;

layout(location = 0) out vec4 fragColor;

layout(binding = 1) uniform sampler2D qt_Texture;

layout(std140, binding = 0) uniform ubuf
{
    mat4 qt_Matrix;
    float qt_Opacity;
};

void main()
{
    fragColor =
        texture(qt_Texture, vTexCoord)
        * qt_Opacity;
}
