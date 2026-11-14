#version 440

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec2 texCoord;

layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out vec2 vPos;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float radius;
    vec2 itemSize;
};

void main()
{
    vTexCoord = texCoord;
    vPos = vertex.xy;
    gl_Position = qt_Matrix * vertex;
}
