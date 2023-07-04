#include "pch.h"
#include "draw_ui.h"
#include "draw_dat_load_progress_bar.h"
#include "draw_gui_for_open_dat_file.h"
#include "ShowClientsConnected.h"
#include "ShowParties.h"
#include "Recorder.h"
#include "Replayer.h"
#include "ShowRecordAndReplayPanel.h"
#include "ShowUpdateOptionsPanel.h"

extern FileType selected_file_type;

auto show_record_and_replay = ShowRecordAndReplayPanel();
auto show_update_options_panel = ShowUpdateOptionsPanel();

void draw_ui(InitializationState initialization_state, int dat_files_to_read, int dat_total_files,
             DATManager& dat_manager, MapRenderer* map_renderer, ImGuiStates& imgui_states,
             ConnectionData& connection_data, PartyManager& party_manager,
             const std::array<GW_skill, 3432>& skills, Recorder& recorder, Replayer& replayer)
{
    if (! gw_dat_path_set)
    {
        draw_gui_for_open_dat_file();
    }
    else
    {
        if (initialization_state == InitializationState::Started)
        {
            draw_dat_load_progress_bar(dat_files_to_read, dat_total_files);
        }
        if (initialization_state == InitializationState::Completed && dat_manager.get_hash_index().size() > 0)
        {
            ShowClientsConnected()(imgui_states, connection_data, skills);
            ShowParties()(imgui_states, connection_data, party_manager, skills, dat_manager, map_renderer);
            show_record_and_replay.draw(imgui_states, recorder, replayer);
            show_update_options_panel.draw(imgui_states, connection_data);
        }
    }
}
