#include "App.h"

App::App()
{
    Init();
    CreateDescriptorSetLayout();
    CreatePipeline();
    CreateCamera();
    CreateModelBuffer();
    CreateShaderBuffer();
    WriteShaderBuffer();
}
App::~App()
{
}
void App::Init()
{
    vk::Window::Init();
    SDL_Rect Rect = vk::Window::GetDisplayBound(0);
    mWindow = vk::Window::MakePtr("VulkanEngine", 1600, 900, false, true);
    mDevice = vk::Device::MakePtr(mWindow);
    mRenderer = vk::Renderer::MakePtr(mDevice);
    // mGui = vk::Gui::MakePtr(mDevice, mWindow);
}
void App::CreateDescriptorSetLayout()
{
    // 模型描述符布局
    {
        // 相机空间缓冲区描述
        VkDescriptorSetLayoutBinding CameraSpaceDescriptorSetLayoutBinding{};
        CameraSpaceDescriptorSetLayoutBinding.binding = 10;
        CameraSpaceDescriptorSetLayoutBinding.descriptorCount = 1;
        CameraSpaceDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        CameraSpaceDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        // 光照缓冲区描述
        VkDescriptorSetLayoutBinding IlluminationDescriptorSetLayoutBinding{};
        IlluminationDescriptorSetLayoutBinding.binding = 11;
        IlluminationDescriptorSetLayoutBinding.descriptorCount = 1;
        IlluminationDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        IlluminationDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        // 模型空间缓冲区描述
        VkDescriptorSetLayoutBinding ModelSpaceDescriptorSetLayoutBinding{};
        ModelSpaceDescriptorSetLayoutBinding.binding = 12;
        ModelSpaceDescriptorSetLayoutBinding.descriptorCount = 1;
        ModelSpaceDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ModelSpaceDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        // 光缓冲区描述
        VkDescriptorSetLayoutBinding SpotLightDescriptorSetLayoutBinding{};
        SpotLightDescriptorSetLayoutBinding.binding = 13;
        SpotLightDescriptorSetLayoutBinding.descriptorCount = 1;
        SpotLightDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        SpotLightDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        // 纹理描述
        VkDescriptorSetLayoutBinding TextureDescriptorSetLayoutBinding{};
        TextureDescriptorSetLayoutBinding.binding = 0;
        TextureDescriptorSetLayoutBinding.descriptorCount = 1;
        TextureDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        TextureDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        // 创建描述符布局
        mDescriptorSetLayout = vk::DescriptorSetLayout::MakePtr(mDevice, 100,
                                                                {
                                                                    CameraSpaceDescriptorSetLayoutBinding,
                                                                    IlluminationDescriptorSetLayoutBinding,
                                                                    ModelSpaceDescriptorSetLayoutBinding,
                                                                    SpotLightDescriptorSetLayoutBinding,
                                                                    TextureDescriptorSetLayoutBinding,
                                                                },
                                                                nullptr);
        //
    }
}
void App::CreatePipeline()
{
    // 创建模型渲染管线
    {
        vk::ShaderModule::Ptr ModelVertexModule = vk::ShaderModule::MakePtr(mDevice, VK_SHADER_STAGE_VERTEX_BIT, "./assets/shaders/model.vert.spv");
        vk::ShaderModule::Ptr ModelFragmentModule = vk::ShaderModule::MakePtr(mDevice, VK_SHADER_STAGE_FRAGMENT_BIT, "./assets/shaders/model.frag.spv");

        VkVertexInputBindingDescription VertexInputBindingDescription{};
        VertexInputBindingDescription.binding = 0;
        VertexInputBindingDescription.stride = sizeof(Vertex);
        VertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkVertexInputAttributeDescription PositionAttributeDescription{};
        PositionAttributeDescription.binding = 0;
        PositionAttributeDescription.location = 0;
        PositionAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
        PositionAttributeDescription.offset = offsetof(Vertex, Position);
        VkVertexInputAttributeDescription NormalAttributeDescription{};
        NormalAttributeDescription.binding = 0;
        NormalAttributeDescription.location = 1;
        NormalAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
        NormalAttributeDescription.offset = offsetof(Vertex, Normal);
        VkVertexInputAttributeDescription ColorAttributeDescription{};
        ColorAttributeDescription.binding = 0;
        ColorAttributeDescription.location = 2;
        ColorAttributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        ColorAttributeDescription.offset = offsetof(Vertex, Color);
        VkVertexInputAttributeDescription TexCoordAttributeDescription{};
        TexCoordAttributeDescription.binding = 0;
        TexCoordAttributeDescription.location = 3;
        TexCoordAttributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
        TexCoordAttributeDescription.offset = offsetof(Vertex, UV);

        vk::Pipeline::PipelineInfo ModelPipelineInfo{};
        ModelPipelineInfo.ShaderModuleList = {ModelVertexModule, ModelFragmentModule};
        ModelPipelineInfo.VertexInputBindingDescription = VertexInputBindingDescription;
        ModelPipelineInfo.VertexInputAttributeDescriptionList = {
            PositionAttributeDescription,
            NormalAttributeDescription,
            ColorAttributeDescription,
            TexCoordAttributeDescription,
        };
        mModelPipeline = vk::Pipeline::MakePtr(mDevice, mDescriptorSetLayout, ModelPipelineInfo);
    }
    // 创建广告牌渲染管线
    {
        vk::ShaderModule::Ptr ModelVertexModule = vk::ShaderModule::MakePtr(mDevice, VK_SHADER_STAGE_VERTEX_BIT, "./assets/shaders/billboard.vert.spv");
        vk::ShaderModule::Ptr ModelFragmentModule = vk::ShaderModule::MakePtr(mDevice, VK_SHADER_STAGE_FRAGMENT_BIT, "./assets/shaders/billboard.frag.spv");

        VkVertexInputBindingDescription VertexInputBindingDescription{};
        VertexInputBindingDescription.binding = 0;
        VertexInputBindingDescription.stride = sizeof(BillboardVertex);
        VertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkVertexInputAttributeDescription PositionAttributeDescription{};
        PositionAttributeDescription.binding = 0;
        PositionAttributeDescription.location = 0;
        PositionAttributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
        PositionAttributeDescription.offset = offsetof(BillboardVertex, Position);

        vk::Pipeline::PipelineInfo ModelPipelineInfo{};
        ModelPipelineInfo.ShaderModuleList = {ModelVertexModule, ModelFragmentModule};
        ModelPipelineInfo.VertexInputBindingDescription = VertexInputBindingDescription;
        ModelPipelineInfo.VertexInputAttributeDescriptionList = {
            PositionAttributeDescription,
        };
        mBillboardPipeline = vk::Pipeline::MakePtr(mDevice, mDescriptorSetLayout, ModelPipelineInfo);
    }
}
void App::CreateCamera()
{
    // 创建相机
    mCamera = vk::Camera::MakePtr(mDevice, 0.0f, -20.0f, 0.0f, -4.0f, 2.5f, 50.0f, 0.01f, 1000.0f);
}
void App::CreateModelBuffer()
{
    // 加载平面模型
    {
        Assimp::Importer AssimpImporter;
        const aiScene *Scene = AssimpImporter.ReadFile("./assets/models/pingmian.obj",
                                                       aiProcess_ValidateDataStructure |
                                                           aiProcess_ImproveCacheLocality |
                                                           aiProcess_RemoveRedundantMaterials |
                                                           aiProcess_FindInvalidData);
        if (Scene == nullptr || Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
        {
            printf(AssimpImporter.GetErrorString());
            printf("\n");
            return;
        }
        std::vector<vk::ModelBuffer::ModelInfo<Vertex>> modelInfoList;
        vk::ModelBuffer::ProcessNode<Vertex>(Scene, Scene->mRootNode, &modelInfoList, std::bind(&App::ProcessMesh, this, std::placeholders::_1));
        // 创建模型缓冲区
        uint64_t VertexDataSize = modelInfoList[0].Vertex.size() * sizeof(modelInfoList[0].Vertex[0]);
        uint64_t IndexDataSize = modelInfoList[0].VertexIndex.size() * sizeof(modelInfoList[0].VertexIndex[0]);
        mModelBuffer1 = vk::ModelBuffer::MakePtr(mDevice, modelInfoList[0].Vertex.data(), VertexDataSize,
                                                 modelInfoList[0].VertexIndex.data(), IndexDataSize, modelInfoList[0].VertexIndex.size());
        //
        // 创建描述符
        mDescriptorSet1 = vk::DescriptorSet::MakePtr(mDevice, mDescriptorSetLayout);
        // 纹理
        vk::Image::ImageInfo TextureInfo = vk::Image::OpenImageFile("./assets/images/pingmian.png");
        mTextureBuffer1 = vk::ShaderImage::MakePtr(mDevice, TextureInfo.Width, TextureInfo.Height, false);
        mTextureBuffer1->AllWriteData(TextureInfo.Data);
        mTextureBuffer1->WriteDescriptorSet({mDescriptorSet1}, 0);
        TextureInfo.Free();
        // 变换矩阵
        mModelSpaceBuffer1 = vk::ShaderBuffer::MakePtr(mDevice, sizeof(ModelSpaceLayout), true);
        mModelSpaceBuffer1->WriteDescriptorSet({mDescriptorSet1}, 12);
        ModelSpaceLayout ModelSpace{};
        ModelSpace.ModelMat = glm::mat4(1.0f);
        ModelSpace.ModelMat = glm::scale(ModelSpace.ModelMat, glm::vec3(3.0f));
        mModelSpaceBuffer1->AllWriteData(&ModelSpace);
    }
    // 加载人物模型
    {
        Assimp::Importer AssimpImporter;
        const aiScene *Scene = AssimpImporter.ReadFile("./assets/models/xiaoluoli/xiaoluoli.obj",
                                                       aiProcess_ValidateDataStructure |
                                                           aiProcess_ImproveCacheLocality |
                                                           aiProcess_RemoveRedundantMaterials |
                                                           aiProcess_FindInvalidData);
        if (Scene == nullptr || Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
        {
            printf(AssimpImporter.GetErrorString());
            printf("\n");
            return;
        }
        std::vector<vk::ModelBuffer::ModelInfo<Vertex>> modelInfoList;
        vk::ModelBuffer::ProcessNode<Vertex>(Scene, Scene->mRootNode, &modelInfoList, std::bind(&App::ProcessMesh, this, std::placeholders::_1));
        std::vector<std::string> TextureFileList{
            "./assets/models/xiaoluoli/shenti.jpg",
            "./assets/models/xiaoluoli/tou.jpg",
            "./assets/models/xiaoluoli/toufa.jpg",
            "./assets/models/xiaoluoli/yifu.jpg",
        };
        mModelBufferList2.resize(modelInfoList.size());
        mDescriptorSetList2.resize(modelInfoList.size());
        mTextureBufferList2.resize(modelInfoList.size());
        for (size_t i = 0; i < modelInfoList.size(); i++)
        {
            // 创建模型缓冲区
            uint64_t VertexDataSize = modelInfoList[i].Vertex.size() * sizeof(modelInfoList[i].Vertex[i]);
            uint64_t IndexDataSize = modelInfoList[i].VertexIndex.size() * sizeof(modelInfoList[i].VertexIndex[i]);
            mModelBufferList2[i] = vk::ModelBuffer::MakePtr(mDevice, modelInfoList[i].Vertex.data(), VertexDataSize,
                                                            modelInfoList[i].VertexIndex.data(), IndexDataSize, modelInfoList[i].VertexIndex.size());
            //
            // 创建描述符
            mDescriptorSetList2[i] = vk::DescriptorSet::MakePtr(mDevice, mDescriptorSetLayout);
            // 创建纹理
            bool IsHave = false;
            for (size_t j = 0; j < TextureFileList.size(); j++)
            {
                vk::Image::ImageInfo TextureInfo = vk::Image::OpenImageFile(TextureFileList[j]);
                if (TextureInfo.Name == modelInfoList[i].ModelName)
                {
                    mTextureBufferList2[i] = vk::ShaderImage::MakePtr(mDevice, TextureInfo.Width, TextureInfo.Height, false);
                    mTextureBufferList2[i]->AllWriteData(TextureInfo.Data);
                    mTextureBufferList2[i]->WriteDescriptorSet({mDescriptorSetList2[i]}, 0);
                    TextureInfo.Free();
                    IsHave = true;
                }
            }
            if (!IsHave)
            {
                vk::Image::ImageInfo TextureInfo = vk::Image::OpenImageFile("./assets/images/pingmian.png");
                mTextureBufferList2[i] = vk::ShaderImage::MakePtr(mDevice, TextureInfo.Width, TextureInfo.Height, false);
                mTextureBufferList2[i]->AllWriteData(TextureInfo.Data);
                mTextureBufferList2[i]->WriteDescriptorSet({mDescriptorSetList2[i]}, 0);
                TextureInfo.Free();
            }
        }
        // 变换矩阵
        mModelSpaceBuffer2 = vk::ShaderBuffer::MakePtr(mDevice, sizeof(ModelSpaceLayout), true);
        mModelSpaceBuffer2->WriteDescriptorSet(mDescriptorSetList2, 12);
        ModelSpaceLayout ModelSpace{};
        ModelSpace.ModelMat = glm::mat4(1.0f);
        mModelSpaceBuffer2->AllWriteData(&ModelSpace);
    }
    // 加载广告牌模型
    {
        // 广告牌顶点数据
        vk::ModelBuffer::ModelInfo<BillboardVertex> BillboardModelInfo{};
        BillboardModelInfo.Vertex = {
            {{-1.0f, 1.0f}},
            {{-1.0f, -1.0f}},
            {{1.0f, -1.0f}},
            {{1.0f, 1.0f}},
        };
        BillboardModelInfo.VertexIndex = {0, 1, 2, 2, 3, 0};
        // 创建模型缓冲区
        uint64_t VertexDataSize = BillboardModelInfo.Vertex.size() * sizeof(BillboardModelInfo.Vertex[0]);
        uint64_t IndexDataSize = BillboardModelInfo.VertexIndex.size() * sizeof(BillboardModelInfo.VertexIndex[0]);
        mModelBuffer3 = vk::ModelBuffer::MakePtr(mDevice, BillboardModelInfo.Vertex.data(), VertexDataSize,
                                                 BillboardModelInfo.VertexIndex.data(), IndexDataSize, BillboardModelInfo.VertexIndex.size());
        //
        std::vector<glm::vec4> SpotLightColorList{
            {1.0f, 0.0f, 0.0f, 1.0f},
            {0.0f, 1.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, 1.0f, 1.0f},
            {0.0f, 1.0f, 1.0f, 1.0f},
            {1.0f, 0.0f, 1.0f, 1.0f},
            {1.0f, 1.0f, 0.0f, 1.0f},
        };
        mSpotLightList.resize(SpotLightColorList.size());
        for (size_t i = 0; i < SpotLightColorList.size(); i++)
        {
            glm::mat4 RotateLight = glm::rotate(glm::mat4(1.0f), (i * glm::two_pi<float>()) / mSpotLightList.size(), {0.0f, 0.0f, 1.0f});
            mSpotLightList[i].Position = glm::vec3(RotateLight * glm::vec4(1.5f, 1.5f, 1.5f, 1.0f));
            mSpotLightList[i].Intensity = 1.0f;
            mSpotLightList[i].Color = SpotLightColorList[i];
            mSpotLightList[i].Size = 1.0f;
        }
        mDescriptorSetList3.resize(mSpotLightList.size());
        mSpotLightBufferList3.resize(mSpotLightList.size());
        for (size_t i = 0; i < mSpotLightList.size(); i++)
        {
            // 描述符
            mDescriptorSetList3[i] = vk::DescriptorSet::MakePtr(mDevice, mDescriptorSetLayout);
            // 着色器缓冲区
            mSpotLightBufferList3[i] = vk::ShaderBuffer::MakePtr(mDevice, sizeof(SpotLightLayout), true);
            mSpotLightBufferList3[i]->WriteDescriptorSet({mDescriptorSetList3[i]}, 13);
            mSpotLightBufferList3[i]->AllWriteData(&mSpotLightList[i]);
        }
    }
}
void App::CreateShaderBuffer()
{
    // 创建着色器缓冲区
    mCameraSpaceBuffer = vk::ShaderBuffer::MakePtr(mDevice, sizeof(CameraSpaceLayout), true);
    mIlluminationBuffer = vk::ShaderBuffer::MakePtr(mDevice, sizeof(IlluminationLayout), true);

    mCameraSpaceBuffer->WriteDescriptorSet({mDescriptorSet1}, 10);
    mCameraSpaceBuffer->WriteDescriptorSet(mDescriptorSetList2, 10);
    mCameraSpaceBuffer->WriteDescriptorSet(mDescriptorSetList3, 10);

    mIlluminationBuffer->WriteDescriptorSet({mDescriptorSet1}, 11);
    mIlluminationBuffer->WriteDescriptorSet(mDescriptorSetList2, 11);
}
void App::WriteShaderBuffer()
{
}

bool App::IsMinimizeWindows()
{
    int FrameWidth, FrameHeight;
    mWindow->GetFrameBufferSize(&FrameWidth, &FrameHeight);
    if (FrameWidth == 0 || FrameHeight == 0)
    {
        return true;
    }
    return false;
}
void App::CalculateFrameRate()
{
    static std::chrono::steady_clock::time_point FrameStartTime = std::chrono::steady_clock::now();
    static std::chrono::steady_clock::time_point StartFrameTime = std::chrono::steady_clock::now();
    static std::chrono::steady_clock::time_point StartTime = std::chrono::steady_clock::now();

    // 当前时间
    std::chrono::steady_clock::time_point CurrentTime = std::chrono::steady_clock::now();
    // 开始时间
    mFrameStartTime = std::chrono::duration<float, std::chrono::seconds::period>(CurrentTime - FrameStartTime).count();
    // 帧时间
    mFrameTime = std::chrono::duration<float, std::chrono::seconds::period>(CurrentTime - StartFrameTime).count();
    StartFrameTime = CurrentTime;
    // 帧率
    float TimeDifference = std::chrono::duration<float, std::chrono::seconds::period>(CurrentTime - StartTime).count();
    if (TimeDifference >= 1.0f)
    {
        StartTime = CurrentTime;
        mFrameRate = mFrameCount;
        mFrameCount = 0;
    }
    mFrameCount++;
}
void App::loop()
{
    // 窗口循环
    while (!mIsWindowClose)
    {
        // 事件处理
        ProcessEvent();
        // 计算帧率
        CalculateFrameRate();
        // 判断窗口是否最小化
        if (IsMinimizeWindows())
        {
            continue;
        }
        // 相机输入
        mCamera->InputTick();
        // GUI渲染
        // vk::Gui::Render(mWindow, std::bind(&App::GuiDesign, this));
        // 渲染
        mRenderer->Render(std::bind(&App::DrawOperations, this, std::placeholders::_1));
    }
    // 等待设备空闲
    mDevice->DeviceWaitIdle();
}

void App::ProcessEvent()
{
    // 事件循环
    SDL_Event Event;
    while (vk::Window::PollEvent(&Event))
    {
        // Gui事件
        // vk::Gui::ProcessEvent(&Event);
        switch (Event.type)
        {
        case SDL_WINDOWEVENT: // 窗口事件
        {
            switch (Event.window.event)
            {
            case SDL_WINDOWEVENT_CLOSE: // 窗口关闭
            {
                mIsWindowClose = true;
            }
            break;
            }
        }
        break;
        case SDL_KEYDOWN: // 按键按下事件
        {
            switch (Event.key.keysym.sym)
            {
            case SDLK_ESCAPE: // Esc
            {
                mIsWindowClose = true;
            }
            break;
            }
        }
        break;
        }
    }
}
void App::GuiDesign()
{
    // ImGui设计
    glm::vec2 CameraView = mCamera->GetView();
    glm::vec3 CameraPos = mCamera->GetPosition();
    ImGui::Begin("GUI");
    ImGui::Text("Welcome to BeiGua's project!");
    ImGui::Text(std::string("FrameRate: " + std::to_string(mFrameRate)).c_str());
    ImGui::Text(std::string("Yaw: " + std::to_string(CameraView.x) + "," + "Pitch: " + std::to_string(CameraView.y)).c_str());
    ImGui::Text(std::string("X: " + std::to_string(CameraPos.x) + ",Y: " + std::to_string(CameraPos.y) + ",Z: " + std::to_string(CameraPos.z)).c_str());
    ImGui::End();
}
void App::DrawOperations(uint32_t currentIndex)
{
    // 更新相机空间缓冲区
    CameraSpaceLayout CameraSpace{};
    CameraSpace.ProjectionMat = mCamera->GetProjectionMat();
    CameraSpace.ViewMat = mCamera->GetViewMat();
    CameraSpace.InverseViewMat = mCamera->GetInverseViewMat();
    mCameraSpaceBuffer->WriteData(currentIndex, &CameraSpace);

    // 更新光照缓冲区
    IlluminationLayout Illumination{};
    Illumination.AmbientLightIntensity = 0.001f;
    Illumination.AmbientLightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    Illumination.SpotLightCount = mSpotLightList.size();
    for (size_t i = 0; i < mSpotLightList.size(); i++)
    {
        Illumination.SpotLightS[i] = mSpotLightList[i];
    }
    mIlluminationBuffer->WriteData(currentIndex, &Illumination);

    // 绘制
    mRenderer->Draw(mModelBuffer1, mDescriptorSet1, mModelPipeline);
    for (size_t i = 0; i < mModelBufferList2.size(); i++)
    {
        mRenderer->Draw(mModelBufferList2[i], mDescriptorSetList2[i], mModelPipeline);
    }
    for (size_t i = 0; i < mDescriptorSetList3.size(); i++)
    {
        mRenderer->Draw(mModelBuffer3, mDescriptorSetList3[i], mBillboardPipeline);
    }

    // ImGui绘制
    // mRenderer->DrawGUI(mGui);

    // 更新点光源缓冲区
    for (size_t i = 0; i < mSpotLightBufferList3.size(); i++)
    {
        glm::mat4 RotateLight = glm::rotate(glm::mat4(1.0f), mFrameTime, {0.0f, 0.0f, 1.0f});
        mSpotLightList[i].Position = glm::vec3(RotateLight * glm::vec4(mSpotLightList[i].Position, 1.0f));
        mSpotLightBufferList3[i]->WriteData(currentIndex, &mSpotLightList[i]);
    }
}

vk::ModelBuffer::ModelInfo<App::Vertex> App::ProcessMesh(aiMesh *mesh)
{
    vk::ModelBuffer::ModelInfo<Vertex> modelInfo{};
    modelInfo.ModelName = mesh->mName.C_Str();
    for (size_t i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex{};
        // 位置
        if (mesh->HasPositions())
        {
            vertex.Position.x = mesh->mVertices[i].x;
            vertex.Position.y = mesh->mVertices[i].y;
            vertex.Position.z = mesh->mVertices[i].z;
        }
        // 法线
        if (mesh->HasNormals())
        {
            vertex.Normal.x = mesh->mNormals[i].x;
            vertex.Normal.y = mesh->mNormals[i].y;
            vertex.Normal.z = mesh->mNormals[i].z;
        }
        // 颜色
        vertex.Color = glm::vec4(1.0f);
        if (mesh->GetNumColorChannels() == 4)
        {
            vertex.Color.r = mesh->mColors[i]->r;
            vertex.Color.g = mesh->mColors[i]->g;
            vertex.Color.b = mesh->mColors[i]->b;
            vertex.Color.a = mesh->mColors[i]->a;
        }
        // UV
        if (mesh->GetNumUVChannels() == 1)
        {
            vertex.UV.x = mesh->mTextureCoords[0][i].x;
            vertex.UV.y = 1 - mesh->mTextureCoords[0][i].y;
        }
        modelInfo.Vertex.push_back(vertex);
    }
    // 顶点索引
    for (size_t i = 0; i < mesh->mNumFaces; i++)
    {
        for (size_t j = 0; j < mesh->mFaces[i].mNumIndices; j++)
        {
            modelInfo.VertexIndex.push_back(mesh->mFaces[i].mIndices[j]);
        }
    }
    return modelInfo;
}
