#version 450

layout(set = 0, binding = 10) uniform CameraSpaceLayout {
    mat4 ProjectionMat;//投影矩阵
    mat4 ViewMat;//视图空间矩阵
    mat4 InverseViewMat;//逆转视图矩阵
} CameraSpace;

layout(set = 0, binding = 13) uniform SpotLightLayout {
    vec3 Position;//光点源位置
    float Intensity;//点光源强度
    vec4 Color;//点光源颜色
    float Size;//点光源大小
} SpotLight;

layout(location = 0) in vec2 inPosition;

layout(location = 0) out vec2 outOffset;

void main() {
    vec3 CameraRightWorld = vec3(CameraSpace.ViewMat[0][0], CameraSpace.ViewMat[1][0], CameraSpace.ViewMat[2][0]);
    vec3 CameraUpWorld = vec3(CameraSpace.ViewMat[0][1], CameraSpace.ViewMat[1][1], CameraSpace.ViewMat[2][1]);

    vec3 PositionWorld = SpotLight.Position + SpotLight.Size * 0.1 * inPosition.x * CameraRightWorld + SpotLight.Size * 0.1 * inPosition.y * CameraUpWorld;

    gl_Position = CameraSpace.ProjectionMat * CameraSpace.ViewMat * vec4(PositionWorld, 1);

    outOffset = inPosition;
}
