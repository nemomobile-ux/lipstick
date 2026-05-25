#version 440

layout(location = 0) in vec4 qt_Vertex;
layout(location = 1) in vec2 qt_MultiTexCoord0;
layout(location = 2) in vec2 vertexCorner;

layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out vec2 vCorner;

layout(std140, binding = 0) uniform buf
{
    mat4 qt_Matrix;
    float qt_Opacity;
    vec2 radius;
};

void main()
{
    vTexCoord = qt_MultiTexCoord0;
    vCorner = vertexCorner;

    gl_Position = qt_Matrix * qt_Vertex;
}
