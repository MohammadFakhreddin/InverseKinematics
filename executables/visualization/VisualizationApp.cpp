#include "VisualizationApp.hpp"

#include "ShapeGenerator.hpp"
#include "camera/ArcballCamera.hpp"

#include <glm/glm.hpp>

#include <array>

using namespace MFA;

//======================================================================================================================

VisualizationApp::VisualizationApp()
{
    _path = Path::Instance();
    _device = LogicalDevice::Instance;

    if (SDL_JoystickOpen(0) != nullptr)
        SDL_JoystickEventState(SDL_ENABLE);

    _device->SDL_EventSignal.Register([&](SDL_Event *event) -> void { OnSDL_Event(event); });

    _swapChainResource = std::make_shared<SwapChainRenderResource>();
    _depthResource = std::make_shared<DepthRenderResource>();
    _msaaResource = std::make_shared<MSSAA_RenderResource>();
    _displayRenderPass = std::make_shared<DisplayRenderPass>(_swapChainResource, _depthResource, _msaaResource);

    _sampler = RB::CreateSampler(_device->GetVkDevice(), RB::CreateSamplerParams{});

    _ui = std::make_shared<UI>(_displayRenderPass, UI::Params {
        .lightMode = false,
        .fontCallback = [this](ImGuiIO & io)->void
        {
            {// Default font
                auto const fontPath = Path::Instance()->Get("fonts/JetBrains-Mono/JetBrainsMonoNL-Regular.ttf");
                MFA_ASSERT(std::filesystem::exists(fontPath));
                _defaultFont = io.Fonts->AddFontFromFileTTF(
                    fontPath.c_str(),
                    20.0f
                );
                MFA_ASSERT(_defaultFont != nullptr);
            }
            {// Bold font
                auto const fontPath = Path::Instance()->Get("fonts/JetBrains-Mono/JetBrainsMono-Bold.ttf");
                MFA_ASSERT(std::filesystem::exists(fontPath));
                _boldFont = io.Fonts->AddFontFromFileTTF(
                    fontPath.c_str(),
                    20.0f
                );
                MFA_ASSERT(_boldFont != nullptr);
            }
        }
    });
    _ui->UpdateSignal.Register([this]() -> void { OnUI(Time::DeltaTimeSec()); });

    _device->ResizeEventSignal2.Register([this]() -> void { Resize(); });

    PrepareSceneRenderPass();

    {
        // Shape pipeline and buffers
        // Light buffer
        auto lightBufferGroup = RB::CreateHostVisibleUniformBuffer(
            _device->GetVkDevice(),
            _device->GetPhysicalDevice(),
            sizeof(ShapePipeline::LightSource),
            _device->GetMaxFramePerFlight()
        );
        _lightBufferTracker = std::make_shared<HostVisibleBufferTracker>(lightBufferGroup);
        ShapePipeline::LightSource light {
            .direction = _lightDirection,
            .ambientStrength = _ambientStrength,
            .color = _lightColor * _lightIntensity,
        };
        _lightBufferTracker->SetData(Alias{light});

        // Camera buffer
        auto cameraBufferGroup = RB::CreateHostVisibleUniformBuffer(
            _device->GetVkDevice(),
            _device->GetPhysicalDevice(),
            sizeof(ShapePipeline::Camera),
            _device->GetMaxFramePerFlight()
        );
        _cameraBufferTracker = std::make_shared<HostVisibleBufferTracker>(cameraBufferGroup);
        // Shape pipeline
        ShapePipeline::Params params{};
        params.cullModeFlags = VK_CULL_MODE_NONE;
        _shapePipeline = std::make_shared<ShapePipeline>(_sceneRenderPass->GetRenderPass(), params);
    }

    {// Cylinder renderer
        auto [
            vertices,
            indices,
            normals
        ] = ShapeGenerator::Cylinder(0.5f, 1, 100);
        _cylinderShapeRenderer = std::make_unique<ShapeRenderer>(
            _shapePipeline,
            _cameraBufferTracker->HostVisibleBuffer(),
            _lightBufferTracker->HostVisibleBuffer(),
            (int)vertices.size(),
            vertices.data(),
            normals.data(),
            (int)indices.size(),
            indices.data()
        );
    }

    {// Sphere renderer
        auto [vertices, indices, normals] = ShapeGenerator::Sphere(0.5f, 40, 40);
        _sphereShapeRenderer = std::make_unique<ShapeRenderer>(
            _shapePipeline,
            _cameraBufferTracker->HostVisibleBuffer(),
            _lightBufferTracker->HostVisibleBuffer(),
            (int)vertices.size(),
            vertices.data(),
            normals.data(),
            (int)indices.size(),
            indices.data()
        );
    }

    {// Grid renderer
        _gridPipeline = std::make_shared<GridPipeline>(_sceneRenderPass->GetRenderPass());
        _gridRenderer = std::make_unique<GridRenderer>(_gridPipeline);
    }

    {// Camera
        _camera = std::make_unique<MFA::ArcballCamera>(
            [this]()->VkExtent2D
            {
                return _sceneWindowSize;
            },
            [this]()->bool{return _sceneWindowFocused;},
            glm::vec3{},
            -Math::ForwardVec3
        );
        _camera->SetfovDeg(40.0f);
        _camera->SetLocalPosition(glm::vec3{20.0f, 20.0f, 20.0f});
        _camera->SetfarPlane(1000.0f);
        _camera->SetnearPlane(0.010f);
        _camera->SetmaxDistance(100.0f);
    }
}

//======================================================================================================================

VisualizationApp::~VisualizationApp() = default;

//======================================================================================================================

void VisualizationApp::Run()
{
    SDL_GL_SetSwapInterval(0);
    SDL_Event e;

    _time = Time::Instantiate(120, 30);

    bool shouldQuit = false;

    while (shouldQuit == false)
    {
        //Handle events
        while (SDL_PollEvent(&e) != 0)
        {
            //User requests quit
            if (e.type == SDL_QUIT)
            {
                shouldQuit = true;
            }
        }

        _device->Update();

        Update(Time::DeltaTimeSec());

        auto recordState = _device->AcquireRecordState(_swapChainResource->GetSwapChainImages().swapChain);
        if (recordState.isValid == true)
        {
            _activeImageIndex = static_cast<int>(recordState.imageIndex);
            Render(recordState);
        }

        _time->Update();
    }

    _time.reset();

    _device->DeviceWaitIdle();
}

//======================================================================================================================
void VisualizationApp::Update(float deltaTime)
{
    if (_sceneWindowResized == true)
    {
        PrepareSceneRenderPass();
        _sceneWindowResized = false;
        return;
    }

    _camera->Update(deltaTime);
    // TODO: Something is wrong. I have to update the matrices otherwise it wont work correctly!
    // if (_camera->IsDirty()))
    {
        ShapePipeline::Camera cameraData
        {
            .viewProjection = _camera->ViewProjection(),
            .position = _camera->GlobalPosition()
        };
        _cameraBufferTracker->SetData(Alias{cameraData});
    }

    _ui->Update();

    for (int i = static_cast<int>(oldScenes.size()) - 1; i >= 0; i--)
    {
        oldScenes[i].remLifeTime -= 1;
        if (oldScenes[i].remLifeTime == 0)
        {
            for (auto const textureID : oldScenes[i].textureIDs)
            {
                UI::Instance->RemoveTexture(textureID);
            }
            oldScenes.erase(oldScenes.begin() + i);
        }
    }

    if (_ikEnabled == true && _hierarchy.empty() == false)
    {
        // Calculate IK here
        glm::vec3 const currentEndPoint = CalculateJointsLocation();
        Eigen::MatrixX<float> const J = Jacobian(currentEndPoint);
        Eigen::MatrixX<float> const JT = Eigen::Transpose(J);
        auto const dEGlm = _ikTargetPosition - currentEndPoint;
        Eigen::MatrixX<float> dE (3, 1);
        dE(0, 0) = dEGlm[0]; dE(1, 0) = dEGlm[1]; dE(2, 0) = dEGlm[2];
        // TODO: Add damping
        Eigen::MatrixX<float> const JTxJ = (JT * J);
        Eigen::MatrixX<float> const identity = Eigen::MatrixXf::Identity(JTxJ.rows(), JTxJ.cols());
        Eigen::MatrixX<float> const damping = _damping * identity;
        Eigen::MatrixX<float> const dTheta = Eigen::Inverse(JTxJ + damping) * JT * dE;
        for (int i = 0; i < (int)_hierarchy.size(); i++)
        {
            auto & joint = _hierarchy[i];
            if (joint.isLengthFixed == false)
            {
                joint.length = joint.length + dTheta(i * 3 + 0, 0);
            }
            if (joint.isX_AngleFixed == false)
            {
                joint.angle.x = joint.angle.x + dTheta(i * 3 + 1, 0);
            }
            if (joint.isY_AngleFixed == false)
            {
                joint.angle.y = joint.angle.y + dTheta(i * 3 + 2, 0);
            }
        }
        // MFA_LOG_INFO("dTheta.rows(): %d, dTheta.cols(): %d", dTheta.rows(), dTheta.cols());
        // for (int i = 0; i < dTheta.rows(); i++)
        // {
        //     printf("%f\n", dTheta(i, 0));
        // }
    }
}

//======================================================================================================================

void VisualizationApp::Render(MFA::RT::CommandRecordState &recordState)
{
    // device->BeginCommandBuffer(
    //     recordState,
    //     RT::CommandBufferType::Compute
    // );
    // device->EndCommandBuffer(recordState);

    _device->BeginCommandBuffer(
        recordState,
        RT::CommandBufferType::Graphic
    );

    _lightBufferTracker->Update(recordState);
    _cameraBufferTracker->Update(recordState);

    _sceneRenderPass->Begin(recordState, *_sceneFrameBuffer);

    // TODO: Connect to UI
    _gridRenderer->Draw(
        recordState,
        GridPipeline::PushConstants {.viewProjMat = _camera->ViewProjection()}
    );


    int colorIdx = 0;
    static std::array<glm::vec4, 3> colors
    {
        glm::vec4{0.5f, 0.0f, 0.0f, 1.0f},
        glm::vec4{0.0f, 0.5f, 0.0f, 1.0f},
        glm::vec4{0.0f, 0.0f, 0.5f, 1.0f},
    };

    glm::vec3 endPoint {};
    static constexpr float sphereScale = 1.5f;

    _sphereShapeRenderer->Queue(ShapePipeline::Instance
    {
        .model = glm::translate(glm::mat4(1), endPoint) * glm::scale(glm::mat4(1), glm::vec3(sphereScale)),
        .color = glm::vec4{0.5f, 0.5f, 0.0f, 1.0f},
        .specularStrength = _specularLightIntensity,
        .shininess = _shininess
    });

    CalculateJointsLocation();

    for (auto const & arm : _hierarchy)
    {
        glm::vec3 startPoint = endPoint;
        endPoint = arm.matrix * glm::vec4 {0.0f, 0.0f, 0.0f, 1.0f};

        auto const middlePoint = (startPoint + endPoint) * 0.5f;
        auto const vector = endPoint - startPoint;
        auto const length = glm::length(vector);
        auto const normal = vector / length;
        auto const tangent = glm::normalize(glm::cross(Math::RightVec3, normal));
        auto const biTangent1 = glm::normalize(glm::cross(normal, tangent));

        glm::mat4 model = glm::translate(glm::mat4(1), middlePoint) *
            Math::ChangeOfBasis(biTangent1, normal, tangent) *
            glm::scale(glm::mat4(1), glm::vec3{1, length, 1});

        ShapePipeline::Instance const instance
        {
            .model = model,
            .color = colors[colorIdx],
            .specularStrength = _specularLightIntensity,
            .shininess = _shininess
        };
        _cylinderShapeRenderer->Queue(instance);

        colorIdx = (colorIdx + 1) % colors.size();

        // TODO: Duplicate code
        _sphereShapeRenderer->Queue(ShapePipeline::Instance
        {
            .model = glm::translate(glm::mat4(1), endPoint) * glm::scale(glm::mat4(1), glm::vec3(sphereScale)),
            .color = glm::vec4{0.5f, 0.5f, 0.0f, 1.0f},
            .specularStrength = _specularLightIntensity,
            .shininess = _shininess
        });
    }

    _sphereShapeRenderer->Queue(ShapePipeline::Instance
    {
        .model = glm::translate(glm::mat4(1), _ikTargetPosition) *
            glm::scale(glm::mat4(1), glm::vec3(sphereScale)),
        .color = glm::vec4{0.5f, 0.0f, 0.5f, 1.0f},
        .specularStrength = _specularLightIntensity,
        .shininess = _shininess
    });

    _cylinderShapeRenderer->Render(recordState);
    _sphereShapeRenderer->Render(recordState);

    _sceneRenderPass->End(recordState);

    _displayRenderPass->Begin(recordState);

    _ui->Render(recordState, Time::DeltaTimeSec());

    _displayRenderPass->End(recordState);
    _device->EndCommandBuffer(recordState);

    _device->SubmitQueues(recordState);
    _device->Present(recordState, _swapChainResource->GetSwapChainImages().swapChain);
}

//======================================================================================================================

void VisualizationApp::Resize()
{
    _camera->SetProjectionDirty();
    _sceneWindowResized = true;
}

//======================================================================================================================

void VisualizationApp::Reload()
{
    auto const * device = LogicalDevice::Instance;
    RB::DeviceWaitIdle(device->GetVkDevice());

    _shapePipeline->Reload();
    _gridPipeline->Reload();
}

//======================================================================================================================

void VisualizationApp::OnSDL_Event(SDL_Event *event)
{
    // if (UI::Instance != nullptr && UI::Instance->HasFocus() == true)
    // {
    //     return;
    // }

    // Keyboard
    if (event->type == SDL_KEYDOWN)
    {
        if (event->key.keysym.sym == SDLK_F5)
        {
            Reload();
        }
    }

    {// Joystick

    }
}

//======================================================================================================================

void VisualizationApp::OnUI(float deltaTimeSec)
{
    ApplyUI_Style();

    _ui->DisplayDockSpace();

    DisplayParametersWindow();

    DisplaySceneWindow();
}

//======================================================================================================================

void VisualizationApp::PrepareSceneRenderPass()
{
    auto const maxImageCount = LogicalDevice::Instance->GetSwapChainImageCount();

    if (_sceneRenderResource != nullptr || _sceneRenderPass != nullptr || _sceneFrameBuffer != nullptr)
    {
        MFA_ASSERT(_sceneRenderResource != nullptr);
        MFA_ASSERT(_sceneRenderPass != nullptr);
        MFA_ASSERT(_sceneFrameBuffer != nullptr);

        oldScenes.emplace_back();
        OldScene & oldScene = oldScenes.back();
        oldScene.sceneRenderResource = _sceneRenderResource;
        oldScene.sceneFrameBuffer = _sceneFrameBuffer;
        oldScene.textureIDs = _sceneTextureID_List;
        oldScene.remLifeTime = static_cast<int>(maxImageCount) + 1;
        oldScenes.emplace_back(oldScene);
        _sceneTextureID_List.clear();
    }

    auto const surfaceFormat = LogicalDevice::Instance->GetSurfaceFormat().format;
    auto const depthFormat = LogicalDevice::Instance->GetDepthFormat();
    auto const sampleCount = LogicalDevice::Instance->GetMaxSampleCount();

    _sceneRenderResource = std::make_shared<SceneRenderResource>(
        _sceneWindowSize,
        surfaceFormat,
        depthFormat,
        sampleCount
    );
    if (_sceneRenderPass == nullptr)
    {
        _sceneRenderPass = std::make_unique<SceneRenderPass>(surfaceFormat, depthFormat, sampleCount);
    }
    _sceneFrameBuffer = std::make_shared<SceneFrameBuffer>(_sceneRenderResource, _sceneRenderPass->GetRenderPass());
    _sceneTextureID_List.resize(maxImageCount);

    for (int imageIndex = 0; imageIndex < maxImageCount; imageIndex++)
    {
        auto & sceneTextureID = _sceneTextureID_List[imageIndex];
        sceneTextureID = _ui->AddTexture(_sampler->sampler, _sceneRenderResource->ColorImage(imageIndex).imageView->imageView);
    }
}

//======================================================================================================================

void VisualizationApp::ApplyUI_Style()
{
    ImGuiStyle &style = ImGui::GetStyle();
    ImVec4 *colors = style.Colors;

    // Backgrounds
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);

    // Borders
    colors[ImGuiCol_Border] = ImVec4(0.30f, 0.30f, 0.35f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Headers
    colors[ImGuiCol_Header] = ImVec4(0.25f, 0.50f, 0.75f, 0.85f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.55f, 0.80f, 0.95f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.35f, 0.60f, 0.85f, 1.00f);

    // Buttons
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.45f, 0.70f, 0.80f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.50f, 0.75f, 0.90f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.30f, 0.55f, 0.80f, 1.00f);

    // Text and Highlights
    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // Bright white text
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.55f, 1.00f); // Muted gray for disabled
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.30f, 0.60f, 0.90f, 0.90f); // Text selection

    // Frame Backgrounds
    colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.50f, 0.80f, 0.70f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.55f, 0.85f, 0.85f);

    // Tabs
    colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.25f, 0.55f, 0.80f, 0.90f);
    colors[ImGuiCol_TabActive] = ImVec4(0.30f, 0.60f, 0.85f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.50f, 0.75f, 1.00f);

    // Title Bars
    colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.25f, 0.55f, 0.80f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.05f, 0.05f, 0.07f, 0.50f);

    // Scrollbars
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.25f, 0.50f, 0.75f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30f, 0.55f, 0.80f, 0.90f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.35f, 0.60f, 0.85f, 1.00f);

    // Style Tweaks
    style.FrameRounding = 5.0f;
    style.WindowRounding = 5.0f;
    style.GrabRounding = 4.0f;
    style.ScrollbarRounding = 5.0f;
    style.ChildRounding = 5.0f;
    style.TabRounding = 4.0f;
}

//======================================================================================================================

void VisualizationApp::DisplaySceneWindow()
{
    _ui->BeginWindow("Scene");
    auto sceneWindowSize = ImGui::GetWindowSize();
    _sceneWindowFocused = ImGui::IsWindowFocused() && ImGui::IsWindowDocked();
    sceneWindowSize.y -= 45.0f;
    sceneWindowSize.x -= 15.0f;
    if (_sceneWindowSize.width != sceneWindowSize.x || _sceneWindowSize.height != sceneWindowSize.y)
    {
        _sceneWindowSize.width = std::max(sceneWindowSize.x, 1.0f);
        _sceneWindowSize.height = std::max(sceneWindowSize.y, 1.0f);
        Resize();
    }
    if (_activeImageIndex < _sceneTextureID_List.size())
    {
        ImGui::Image(_sceneTextureID_List[_activeImageIndex], sceneWindowSize);
    }
    _ui->EndWindow();
}

//======================================================================================================================

void VisualizationApp::DisplayParametersWindow()
{
    _ui->BeginWindow("Lighting");

    if (ImGui::InputFloat3("Light direction", reinterpret_cast<float *>(&_lightDirection)))
    {
        auto *light = (ShapePipeline::LightSource *)_lightBufferTracker->Data();
        auto const magnitude2 = glm::length2(_lightDirection);
        if (magnitude2 > 0.0f)
        {
            light->direction = _lightDirection / std::sqrt(magnitude2);
        }
    }
    if (ImGui::ColorPicker4("Light color", reinterpret_cast<float *>(&_lightColor)))
    {
        auto *light = (ShapePipeline::LightSource *)_lightBufferTracker->Data();
        light->color = _lightColor * _lightIntensity;
    }
    if (ImGui::SliderFloat("Ambient intensity", &_ambientStrength, 0.0f, 1.0f))
    {
        auto *light = (ShapePipeline::LightSource *)_lightBufferTracker->Data();
        light->ambientStrength = _ambientStrength;
    }
    ImGui::SliderFloat("Specularity", &_specularLightIntensity, 0.0f, 10.0f);
    ImGui::InputInt("Shininess", &_shininess, 1, 256);

    _ui->EndWindow();

    _ui->BeginWindow("Hierarchy");

    ImGui::SeparatorText("IK-Target");
    ImGui::SliderFloat3("IK Target", reinterpret_cast<float *>(&_ikTargetPosition), -10.0f, 10.0f);
    ImGui::Checkbox("Enable IK", &_ikEnabled);
    ImGui::SliderFloat("Damping", &_damping, 0.001f, 1.0f);

    ImGui::SeparatorText("Joints");

    for (int i = 0; i < _hierarchy.size(); i++)
    {
        char name[32];
        sprintf(name, "Joint %d", i);
        if (ImGui::TreeNode(name))
        {
            auto &joint = _hierarchy[i];
            ImGui::SliderFloat("Length", &joint.length, 0.0, 10.0f);
            ImGui::SliderFloat2("Angle", reinterpret_cast<float *>(&joint.angle), -180.0f, 180.0f);
            ImGui::Checkbox("Is length fixed", &joint.isLengthFixed);
            ImGui::Checkbox("Is angle X fixed", &joint.isX_AngleFixed);
            ImGui::Checkbox("Is angle y fixed", &joint.isY_AngleFixed);
            ImGui::TreePop();
        }
    }

    ImVec2 buttonSize = ImVec2(32, 0); // width 120, height auto
    float windowWidth = ImGui::GetWindowSize().x;
    float buttonX = (windowWidth - buttonSize.x * 2) * 0.5f;
    ImGui::SetCursorPosX(buttonX);
    if (ImGui::Button("+", buttonSize))
    {
        _hierarchy.emplace_back();
    }
    ImGui::SameLine();
    if (ImGui::Button("-", buttonSize))
    {
        _hierarchy.pop_back();
    }

    _ui->EndWindow();
}

//======================================================================================================================

glm::vec3 VisualizationApp::CalculateJointsLocation()
{
    glm::mat4 matrix = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), Math::RightVec3);

    for (auto & arm : _hierarchy)
    {
        auto const rotateX = glm::rotate(glm::mat4(1), glm::radians(arm.angle.x), Math::RightVec3);
        auto const rotateY = glm::rotate(glm::mat4(1), glm::radians(arm.angle.y), Math::UpVec3);
        auto const translate = glm::translate(glm::mat4(1), Math::UpVec3 * arm.length);
        matrix *= rotateY * rotateX * translate;

        arm.matrix = matrix;
    }

    return matrix * glm::vec4{0.0, 0.0, 0.0, 1.0};
}

//======================================================================================================================

Eigen::MatrixX<float> VisualizationApp::Jacobian(glm::vec3 const & currentEndPoint)
{
    // The epsilon here determines the convergence rate and thus the speed
    static constexpr float lengthEpsilon = 0.025f;
    static constexpr float angleEpsilon = 0.5f;

    // 3 degrees of freedom per joint (Length + x, y angles)
    Eigen::MatrixX<float> Jacobian(3, _hierarchy.size() * 3);
    for (int armIdx = 0; armIdx < _hierarchy.size(); armIdx++)
    {
        auto & joint = _hierarchy[armIdx];

        {// Length
            glm::vec3 prevEndPoint = currentEndPoint;
            glm::vec3 nextEndPoint = currentEndPoint;
            if (joint.isLengthFixed == false)
            {
                float const jointLength = joint.length;
                joint.length = jointLength - lengthEpsilon;
                prevEndPoint = CalculateJointsLocation();
                joint.length = jointLength + lengthEpsilon;
                nextEndPoint = CalculateJointsLocation();
                joint.length = jointLength;
            }
            auto const difference = (nextEndPoint - prevEndPoint) / 2.0f * lengthEpsilon;
            Jacobian(0, 3 * armIdx + 0) = difference.x;
            Jacobian(1, 3 * armIdx + 0) = difference.y;
            Jacobian(2, 3 * armIdx + 0) = difference.z;
        }

        {// X angle
            glm::vec3 prevEndPoint = currentEndPoint;
            glm::vec3 nextEndPoint = currentEndPoint;
            if (joint.isX_AngleFixed == false)
            {
                float const angleX = joint.angle.x;
                joint.angle.x = angleX - angleEpsilon;
                prevEndPoint = CalculateJointsLocation();
                joint.angle.x = angleX + angleEpsilon;
                nextEndPoint = CalculateJointsLocation();
                joint.angle.x = angleX;
            }
            auto const difference = (nextEndPoint - prevEndPoint) / 2.0f * angleEpsilon;
            Jacobian(0, 3 * armIdx + 1) = difference.x;
            Jacobian(1, 3 * armIdx + 1) = difference.y;
            Jacobian(2, 3 * armIdx + 1) = difference.z;
        }

        {// Y angle
            glm::vec3 prevEndPoint = currentEndPoint;
            glm::vec3 nextEndPoint = currentEndPoint;
            if (joint.isY_AngleFixed == false)
            {
                float const angleY = joint.angle.y;
                joint.angle.y = angleY - angleEpsilon;
                prevEndPoint = CalculateJointsLocation();
                joint.angle.y = angleY + angleEpsilon;
                nextEndPoint = CalculateJointsLocation();
                joint.angle.y = angleY;
            }
            auto const difference = (nextEndPoint - prevEndPoint) / 2.0f * angleEpsilon;
            Jacobian(0, 3 * armIdx + 2) = difference.x;
            Jacobian(1, 3 * armIdx + 2) = difference.y;
            Jacobian(2, 3 * armIdx + 2) = difference.z;
        }
    }
    return Jacobian;
}

//======================================================================================================================
