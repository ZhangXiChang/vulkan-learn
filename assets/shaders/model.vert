#version 450

layout(set = 0, binding = 10) uniform CameraSpaceLayout {
    mat4 ProjectionMat;//投影矩阵
    mat4 ViewMat;//视图空间矩阵
    mat4 InverseViewMat;//逆转视图矩阵
} CameraSpace;

layout(set = 0, binding = 12) uniform ModelSpaceLayout {
    mat4 ModelMat;//模型空间矩阵
} ModelSpace;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inColor;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outUV;
layout(location = 2) out vec3 outVertexPos;
layout(location = 3) out vec3 outNormalPos;

void main() {
    //顶点在视图中的位置
    vec4 VertexPos = ModelSpace.ModelMat * vec4(inPosition, 1.0);
    gl_Position = CameraSpace.ProjectionMat * CameraSpace.ViewMat * VertexPos;

    //顶点在世界中的位置
    outVertexPos = VertexPos.xyz;

    //法线在世界中的位置
    outNormalPos = normalize(mat3(ModelSpace.ModelMat) * inNormal);

    //输出
    outColor = inColor;
    outUV = inUV;
}
