#version 450

layout(set = 0, binding = 13) uniform SpotLightLayout {
    vec3 Position;//光点源位置
    float Intensity;//点光源强度
    vec4 Color;//点光源颜色
    float Size;//点光源大小
} SpotLight;

layout(location = 0) in vec2 inOffset;

layout(location = 0) out vec4 outColor;

void main() {
    float dis = sqrt(dot(inOffset, inOffset));
    if(dis >= 1) {
        discard;
    }
    outColor = SpotLight.Color;
}
