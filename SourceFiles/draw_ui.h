#pragma once
#include "MapBrowser.h"

void draw_ui(InitializationState initialization_state, int dat_files_to_read, int dat_total_files,
             DATManager& dat_manager, MapRenderer* map_renderer, ImGuiStates& imgui_states,
             ConnectionData& connection_data, PartyManager& party_manager,
             const std::array<GW_skill, 3432>& skills, Recorder& recorder);
