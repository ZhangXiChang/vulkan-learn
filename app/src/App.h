#pragma once
#include "vk/Window.h"
#include "vk/Device.h"
#include "vk/Renderer.h"
#include "vk/Camera.h"
#include "vk/Gui.h"
#include "vk/DescriptorSetLayout.h"
#include "vk/Pipeline.h"
#include "vk/DescriptorSet.h"
#include "vk/ModelBuffer.h"
#include "vk/ShaderImage.h"
#include "vk/ShaderBuffer.h"

class App
{
public:
    struct Vertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec4 Color;
        glm::vec2 UV;
    };
    struct BillboardVertex
    {
        glm::vec2 Position;
    };

    struct CameraSpaceLayout
    {
        alignas(16) glm::mat4 ProjectionMat;
        alignas(16) glm::mat4 ViewMat;
        alignas(16) glm::mat4 InverseViewMat;
    };

    struct ModelSpaceLayout
    {
        alignas(16) glm::mat4 ModelMat;
    };

    struct SpotLightLayout
    {
        alignas(16) glm::vec3 Position;
        alignas(4) float Intensity;
        alignas(16) glm::vec4 Color;
        alignas(4) float Size;
    };

    struct IlluminationLayout
    {
        alignas(4) float AmbientLightIntensity;
        alignas(16) glm::vec4 AmbientLightColor;
        SpotLightLayout SpotLightS[10];
        alignas(4) int SpotLightCount;
    };

public:
    App();
    ~App();

private:
    // 窗口
    vk::Window::Ptr mWindow;
    // 设备
    vk::Device::Ptr mDevice;
    // 渲染器
    vk::Renderer::Ptr mRenderer;
    // ImGui
    vk::Gui::Ptr mGui;
    // 窗口循环
    bool mIsWindowClose = false;

    // 帧率
    uint32_t mFrameCount = 0;
    uint32_t mFrameRate = 0;
    float mFrameTime = 0;
    float mFrameStartTime = 0;

    // 描述符布局
    vk::DescriptorSetLayout::Ptr mDescriptorSetLayout;

    // 渲染管线
    vk::Pipeline::Ptr mModelPipeline;
    vk::Pipeline::Ptr mBillboardPipeline;

    // 相机
    vk::Camera::Ptr mCamera;

    // 平面模型
    vk::ModelBuffer::Ptr mModelBuffer1;
    vk::DescriptorSet::Ptr mDescriptorSet1;
    vk::ShaderImage::Ptr mTextureBuffer1;
    vk::ShaderBuffer::Ptr mModelSpaceBuffer1;

    // 人物模型
    std::vector<vk::ModelBuffer::Ptr> mModelBufferList2;
    std::vector<vk::DescriptorSet::Ptr> mDescriptorSetList2;
    std::vector<vk::ShaderImage::Ptr> mTextureBufferList2;
    vk::ShaderBuffer::Ptr mModelSpaceBuffer2;

    // 点光模型
    vk::ModelBuffer::Ptr mModelBuffer3;
    std::vector<vk::DescriptorSet::Ptr> mDescriptorSetList3;
    std::vector<vk::ShaderBuffer::Ptr> mSpotLightBufferList3;

    // 着色器缓冲区
    vk::ShaderBuffer::Ptr mCameraSpaceBuffer;
    vk::ShaderBuffer::Ptr mIlluminationBuffer;

    // 光数据
    std::vector<SpotLightLayout> mSpotLightList;

private:
    void Init();
    void CreateDescriptorSetLayout();
    void CreatePipeline();
    void CreateCamera();
    void CreateModelBuffer();
    void CreateShaderBuffer();
    void WriteShaderBuffer();

    bool IsMinimizeWindows();
    void ProcessEvent();
    void GuiDesign();
    void DrawOperations(uint32_t currentIndex);
    void CalculateFrameRate();

    // 处理顶点回调函数
    vk::ModelBuffer::ModelInfo<Vertex> ProcessMesh(aiMesh *mesh);

public:
    void loop();
};
