//
// MapBrowser.cpp
//

#include "pch.h"
#include "MapBrowser.h"
#include "draw_gui_for_open_dat_file.h"
#include "draw_dat_load_progress_bar.h"
#include "draw_ui.h"

extern void ExitMapBrowser() noexcept;
extern bool party_members_changed;

using namespace DirectX;

using Microsoft::WRL::ComPtr;

MapBrowser::MapBrowser(InputManager* input_manager) noexcept(false)
    : m_input_manager(input_manager)
    , m_recorder(m_connection_data)
    , m_replayer(m_connection_data)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    // TODO: Provide parameters for swapchain format, depth/stencil format, and backbuffer count.
    //   Add DX::DeviceResources::c_AllowTearing to opt-in to variable rate displays.
    //   Add DX::DeviceResources::c_EnableHDR for HDR10 display.
    m_deviceResources->RegisterDeviceNotify(this);
}

// Initialize the Direct3D resources required to run.
void MapBrowser::Initialize(HWND window, int width, int height)
{
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    m_map_renderer = std::make_unique<MapRenderer>(m_deviceResources->GetD3DDevice(),
                                                   m_deviceResources->GetD3DDeviceContext(), m_input_manager);
    m_map_renderer->Initialize(m_deviceResources->GetScreenViewport().Width,
                               m_deviceResources->GetScreenViewport().Height);

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();

    // Modify the style to have rounded corners
    style.WindowRounding = 5.0f;
    style.ChildRounding = 5.0f;
    style.FrameRounding = 5.0f;
    style.GrabRounding = 5.0f;

    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(m_deviceResources->GetD3DDevice(), m_deviceResources->GetD3DDeviceContext());

    // Init ConnectionData
    m_connection_data_thread = std::thread(&ConnectionData::run, &m_connection_data);

    // Init Party Manager
    m_party_manager_thread = std::thread(&PartyManager::run, &m_party_manager, std::ref(m_connection_data));

    GW_skill::load_gw_skill_data(m_skills, m_deviceResources->GetD3DDevice());
}

#pragma region Frame Update
// Executes the basic render loop.
void MapBrowser::Tick()
{
    m_timer.Tick([&]() { Update(m_timer); });

    Render();
}

// Updates the world.
void MapBrowser::Update(DX::StepTimer const& timer)
{
    if (gw_dat_path_set && m_dat_manager.m_initialization_state == InitializationState::NotStarted)
    {
        bool succeeded = m_dat_manager.Init(gw_dat_path);
        if (! succeeded)
        {
            gw_dat_path_set = false;
            gw_dat_path = L"";
        }
    }

    float elapsedTime = float(timer.GetElapsedSeconds());

    auto parties_client_names = m_party_manager.get_parties_client_names();

    if (party_members_changed)
    {
        m_map_renderer->ClearAgents();
    }

    if (parties_client_names.size() > 0)
    {
        const auto it = parties_client_names.find(m_imgui_states.selected_party);
        if (it != parties_client_names.end())
        {
            const auto party_client_names = it->second;
            for (const auto& client_name : party_client_names)
            {
                static std::vector<uint8_t> client_data_buffer_ =
                  std::vector<uint8_t>(GWIPC::CLIENTDATA_SIZE);
                const auto* client_data = m_connection_data.get_client_data(client_name, client_data_buffer_);

                // Player
                if (client_data)
                {
                    if (client_data->character() && client_data->character()->agent_living())
                    {
                        bool is_party_leader = client_data->character()->agent_living()->party_slot() == 0;
                        if (is_party_leader)
                        {
                            m_map_renderer->UpdateAgentLiving(client_data->character()->agent_living(),
                                                              Color::DarkGreen);
                        }
                        else
                        {
                            m_map_renderer->UpdateAgentLiving(client_data->character()->agent_living(),
                                                              Color::Green);
                        }
                    }

                    // Enemies
                    const auto* const enemies = client_data->enemies();
                    if (enemies != nullptr)
                    {
                        for (int i = 0; i < enemies->size(); i++)
                        {
                            // Get enemy at index i
                            const GWIPC::Enemy* enemy = enemies->Get(i);

                            if (enemy->agent_living())
                            {
                                if (enemy->agent_living()->health() > 0)
                                {
                                    m_map_renderer->UpdateAgentLiving(enemy->agent_living(), Color::Red);
                                }
                                else
                                {
                                    m_map_renderer->UpdateAgentLiving(enemy->agent_living(), Color::White);
                                }
                            }
                        }
                    }

                    // Gadgets
                    const auto* const gadgets = client_data->gadgets();
                    if (gadgets != nullptr)
                    {
                        for (int i = 0; i < gadgets->size(); i++)
                        {
                            // Get enemy at index i
                            const GWIPC::AgentGadget* gadget = gadgets->Get(i);

                            if (gadget->agent())
                            {
                                m_map_renderer->UpdateAgent(gadget->agent(), Color::Blue);
                            }
                        }
                    }

                    // Items
                    const auto* const items = client_data->items();
                    if (items != nullptr)
                    {
                        for (int i = 0; i < items->size(); i++)
                        {
                            // Get enemy at index i
                            const GWIPC::AgentItem* item = items->Get(i);

                            if (item->agent())
                            {
                                m_map_renderer->UpdateAgent(item->agent(), Color::Orange);
                            }
                        }
                    }

                    if (client_data->party())
                    {
                        // Player members
                        const auto* const player_members = client_data->party()->player_members();
                        if (player_members != nullptr)
                        {
                            for (int i = 0; i < player_members->size(); i++)
                            {
                                // Get enemy at index i
                                const GWIPC::AgentLiving* player_member = player_members->Get(i);

                                bool is_party_leader = player_member->party_slot() == 0;
                                if (is_party_leader)
                                {
                                    m_map_renderer->UpdateAgentLiving(player_member, Color::DarkGreen);
                                }
                                else
                                {
                                    m_map_renderer->UpdateAgentLiving(player_member, Color::Green);
                                }
                            }
                        }

                        // Henchman members
                        const auto* const henchman_members = client_data->party()->henchman_members();
                        if (henchman_members != nullptr)
                        {
                            for (int i = 0; i < henchman_members->size(); i++)
                            {
                                // Get enemy at index i
                                const GWIPC::AgentLiving* henchman_member = henchman_members->Get(i);
                                m_map_renderer->UpdateAgentLiving(henchman_member, Color::LightGreen);
                            }
                        }

                        // Hero members
                        const auto* const hero_members = client_data->party()->hero_members();
                        if (hero_members != nullptr)
                        {
                            for (int i = 0; i < hero_members->size(); i++)
                            {
                                // Get enemy at index i
                                const GWIPC::Hero* hero_member = hero_members->Get(i);
                                if (hero_member->agent_living() != nullptr)
                                    m_map_renderer->UpdateAgentLiving(hero_member->agent_living(),
                                                                      Color::Pink);
                            }
                        }

                        // Extra NPC members
                        const auto* const extra_npc_members = client_data->party()->extra_npc_members();
                        if (extra_npc_members != nullptr)
                        {
                            for (int i = 0; i < extra_npc_members->size(); i++)
                            {
                                // Get enemy at index i
                                const GWIPC::AgentLiving* extra_npc_member = extra_npc_members->Get(i);
                                m_map_renderer->UpdateAgentLiving(extra_npc_member, Color::Purple);
                            }
                        }
                    }
                }
            }
        }
    }

    m_map_renderer->Update(elapsedTime);
    m_recorder.update();
    m_replayer.update();
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void MapBrowser::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");

    m_map_renderer->Render();

    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    draw_ui(m_dat_manager.m_initialization_state, m_dat_manager.get_num_files_type_read(),
            m_dat_manager.get_num_files(), m_dat_manager, m_map_renderer.get(), m_imgui_states,
            m_connection_data, m_party_manager, m_skills, m_recorder, m_replayer);

    static bool show_demo_window = false;
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    // Dear ImGui Render
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    m_deviceResources->PIXEndEvent();

    // Show the new frame.
    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void MapBrowser::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto const viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void MapBrowser::OnActivated()
{
    // TODO: MapBrowser is becoming active window.
}

void MapBrowser::OnDeactivated()
{
    // TODO: MapBrowser is becoming background window.
}

void MapBrowser::OnSuspending()
{
    // TODO: MapBrowser is being power-suspended (or minimized).
}

void MapBrowser::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: MapBrowser is being power-resumed (or returning from minimize).
}

void MapBrowser::OnWindowMoved()
{
    auto const r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void MapBrowser::OnDisplayChange() { m_deviceResources->UpdateColorSpace(); }

void MapBrowser::OnWindowSizeChanged(int width, int height)
{
    if (! m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    // TODO: MapBrowser window is being resized.
    m_map_renderer->OnViewPortChanged(width, height);
}

// Properties
void MapBrowser::GetDefaultSize(int& width, int& height) const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 1600;
    height = 900;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void MapBrowser::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    // TODO: Initialize device dependent objects here (independent of window size).
    device;
}

// Allocate all memory resources that change on a window SizeChanged event.
void MapBrowser::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.
}

void MapBrowser::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.
}

void MapBrowser::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
