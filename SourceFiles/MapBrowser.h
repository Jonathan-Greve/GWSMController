//
// MapBrowser.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "InputManager.h"
#include "Camera.h"
#include "DATManager.h"
#include "MapRenderer.h"
#include "ConnectionData.h"
#include "PartyManager.h"
#include "ImGuiStates.h"
#include "Recorder.h"
#include "Replayer.h"

// A basic MapBrowser implementation that creates a D3D11 device and
// provides a MapBrowser loop.
class MapBrowser final : public DX::IDeviceNotify
{
public:
    MapBrowser(InputManager* input_manager) noexcept(false);
    ~MapBrowser() = default;

    MapBrowser(MapBrowser&&) = default;
    MapBrowser& operator=(MapBrowser&&) = default;

    MapBrowser(MapBrowser const&) = delete;
    MapBrowser& operator=(MapBrowser const&) = delete;

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic MapBrowser loop
    void Tick();

    // IDeviceNotify
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnDisplayChange();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize(int& width, int& height) const noexcept;

private:
    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    // Device resources.
    std::unique_ptr<DX::DeviceResources> m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer m_timer;

    // Input manager
    InputManager* m_input_manager;

    // dat file manager
    DATManager m_dat_manager;

    std::unique_ptr<MapRenderer> m_map_renderer;

    // Guild wars interaction
    ConnectionData m_connection_data;
    PartyManager m_party_manager;

    std::thread m_party_manager_thread;
    std::thread m_connection_data_thread;

    std::array<GW_skill, 3432> m_skills;

    ImGuiStates m_imgui_states;


    // Record and replay data
    Recorder m_recorder;
    Replayer m_replayer;
};
