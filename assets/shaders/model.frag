#version 450

layout(set = 0, binding = 0) uniform sampler2D Image;

layout(set = 0, binding = 10) uniform CameraSpaceLayout {
    mat4 ProjectionMat;//投影矩阵
    mat4 ViewMat;//视图空间矩阵
    mat4 InverseViewMat;//逆转视图矩阵
} CameraSpace;

struct SpotLightLayout {
    vec3 Position;//光点源位置
    float Intensity;//点光源强度
    vec4 Color;//点光源颜色
    float Size;//点光源大小
};

layout(set = 0, binding = 11) uniform IlluminationLayout {
    float AmbientLightIntensity;//环境光强度
    vec4 AmbientLightColor;//环境光颜色
    SpotLightLayout SpotLightS[10];//点光源
    int SpotLightCount;//点光源数
} Illumination;

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inVertexPos;
layout(location = 3) in vec3 inNormalPos;

layout(location = 0) out vec4 outColor;

void main() {
    //纹理
    vec4 Texture = texture(Image, inUV);

    //环境光
    vec3 AmbientLight = Illumination.AmbientLightColor.xyz * Illumination.AmbientLightIntensity;
    //归一化法线
    vec3 SurfaceNormal = normalize(inNormalPos);
    //反射光
    vec3 SpecularLight = vec3(0.0);

    vec3 CameraPosWorld = CameraSpace.InverseViewMat[3].xyz;
    vec3 ViewDirection = normalize(CameraPosWorld - inVertexPos);

    //点光源
    for(int i = 0; i < Illumination.SpotLightCount; i++) {
        //光程
        vec3 OpticalPath = Illumination.SpotLightS[i].Position - inVertexPos;
        //衰减
        float OpticalPathDecayFactor = 1.0 / dot(OpticalPath, OpticalPath);
        OpticalPath = normalize(OpticalPath);
        //漫反射
        float DiffuseReflection = max(dot(SurfaceNormal, OpticalPath), 0.0);
        //光源
        vec3 SpotLight = Illumination.SpotLightS[i].Color.xyz * Illumination.SpotLightS[i].Intensity * OpticalPathDecayFactor;
        //结算
        AmbientLight += SpotLight * DiffuseReflection;

        //镜面反射
        vec3 HalfAngle = normalize(OpticalPath + ViewDirection);
        float BlinnTerm = dot(SurfaceNormal, HalfAngle);
        BlinnTerm = clamp(BlinnTerm, 0, 1);
        BlinnTerm = pow(BlinnTerm, 512.0);
        SpecularLight += SpotLight * BlinnTerm;
    }

    //输出
    outColor = vec4(AmbientLight * inColor.rgb + SpecularLight * inColor.rgb, inColor.a) * Texture;
}
