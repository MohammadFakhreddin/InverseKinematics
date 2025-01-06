#include "VisualizationApp.hpp"

using namespace MFA;

//======================================================================================================================

VisualizationApp::VisualizationApp()
{
    _path = Path::Instance();

    LogicalDevice::InitParams params{.windowWidth = 800,
                                     .windowHeight = 800,
                                     .resizable = true,
                                     .fullScreen = false,
                                     .applicationName = "InverseKinematics"};

    _device = LogicalDevice::Instantiate(params);
    assert(_device->IsValid() == true);

    if (SDL_JoystickOpen(0) != nullptr)
        SDL_JoystickEventState(SDL_ENABLE);

    _device->SDL_EventSignal.Register([&](SDL_Event *event) -> void { OnSDL_Event(event); });

    _swapChainResource = std::make_shared<SwapChainRenderResource>();
    _depthResource = std::make_shared<DepthRenderResource>();
    _msaaResource = std::make_shared<MSSAA_RenderResource>();
    _displayRenderPass = std::make_shared<DisplayRenderPass>(_swapChainResource, _depthResource, _msaaResource);

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
}

//======================================================================================================================

VisualizationApp::~VisualizationApp()
{
    _ui.reset();
    _displayRenderPass.reset();
    _msaaResource.reset();
    _depthResource.reset();
    _swapChainResource.reset();
    _device.reset();
    _path.reset();
}

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
    _ui->Update();
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
}

//======================================================================================================================

void VisualizationApp::OnSDL_Event(SDL_Event *event)
{
    if (UI::Instance != nullptr && UI::Instance->HasFocus() == true)
    {
        return;
    }

    {// Keyboard

    }

    {// Joystick

    }
}

//======================================================================================================================

void VisualizationApp::OnUI(float deltaTimeSec)
{
    {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // Backgrounds
        colors[ImGuiCol_WindowBg]             = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
        colors[ImGuiCol_ChildBg]              = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
        colors[ImGuiCol_PopupBg]              = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);

        // Borders
        colors[ImGuiCol_Border]               = ImVec4(0.30f, 0.30f, 0.35f, 1.00f);
        colors[ImGuiCol_BorderShadow]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

        // Headers
        colors[ImGuiCol_Header]               = ImVec4(0.25f, 0.50f, 0.75f, 0.85f);
        colors[ImGuiCol_HeaderHovered]        = ImVec4(0.30f, 0.55f, 0.80f, 0.95f);
        colors[ImGuiCol_HeaderActive]         = ImVec4(0.35f, 0.60f, 0.85f, 1.00f);

        // Buttons
        colors[ImGuiCol_Button]               = ImVec4(0.20f, 0.45f, 0.70f, 0.80f);
        colors[ImGuiCol_ButtonHovered]        = ImVec4(0.25f, 0.50f, 0.75f, 0.90f);
        colors[ImGuiCol_ButtonActive]         = ImVec4(0.30f, 0.55f, 0.80f, 1.00f);

        // Text and Highlights
        colors[ImGuiCol_Text]                 = ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // Bright white text
        colors[ImGuiCol_TextDisabled]         = ImVec4(0.50f, 0.50f, 0.55f, 1.00f); // Muted gray for disabled
        colors[ImGuiCol_TextSelectedBg]       = ImVec4(0.30f, 0.60f, 0.90f, 0.90f); // Text selection

        // Frame Backgrounds
        colors[ImGuiCol_FrameBg]              = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
        colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.20f, 0.50f, 0.80f, 0.70f);
        colors[ImGuiCol_FrameBgActive]        = ImVec4(0.25f, 0.55f, 0.85f, 0.85f);

        // Tabs
        colors[ImGuiCol_Tab]                  = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
        colors[ImGuiCol_TabHovered]           = ImVec4(0.25f, 0.55f, 0.80f, 0.90f);
        colors[ImGuiCol_TabActive]            = ImVec4(0.30f, 0.60f, 0.85f, 1.00f);
        colors[ImGuiCol_TabUnfocused]         = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive]   = ImVec4(0.20f, 0.50f, 0.75f, 1.00f);

        // Title Bars
        colors[ImGuiCol_TitleBg]              = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
        colors[ImGuiCol_TitleBgActive]        = ImVec4(0.25f, 0.55f, 0.80f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.05f, 0.05f, 0.07f, 0.50f);

        // Scrollbars
        colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
        colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.25f, 0.50f, 0.75f, 0.80f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30f, 0.55f, 0.80f, 0.90f);
        colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.35f, 0.60f, 0.85f, 1.00f);

        // Style Tweaks
        style.FrameRounding = 5.0f;
        style.WindowRounding = 5.0f;
        style.GrabRounding = 4.0f;
        style.ScrollbarRounding = 5.0f;
        style.ChildRounding = 5.0f;
        style.TabRounding = 4.0f;
    }
    _ui->DisplayDockSpace();
    _ui->BeginWindow("Parameters");
    ImGui::PushFont(_defaultFont);
    ImGui::Text("This is a text");
    ImGui::PopFont();
    ImGui::PushFont(_boldFont);
    ImGui::Text("This is a bold text");
    ImGui::PopFont();
    _ui->EndWindow();
    _ui->BeginWindow("Scene");
    _ui->EndWindow();
}

//======================================================================================================================
