#pragma once

#include "PartyManager.h"
#include "ConnectionData.h"
#include "ImGuiStates.h"

class ShowParties
{
    static inline std::vector<uint8_t> client_data_buffer_ = std::vector<uint8_t>(GWIPC::CLIENTDATA_SIZE);

    void built_unit_frame(const GWIPC::AgentLiving* agent_living, const GWIPC::Skillbar* skillbar,
                          const std::array<GW_skill, 3432>& skills);

public:
    static inline ImVec2 window_pos;
    static inline float window_width;
    static inline float window_height;

    void operator()(ImGuiStates& imgui_states, ConnectionData& connection_data, PartyManager& party_manager,
                    const std::array<GW_skill, 3432>& skills, DATManager& dat_manager,
                    MapRenderer* map_renderer);
};
