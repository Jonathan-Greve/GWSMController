#include "pch.h"
#include "draw_ui.h"
#include "draw_dat_load_progress_bar.h"
#include "draw_gui_for_open_dat_file.h"

extern FileType selected_file_type;

void draw_ui(InitializationState initialization_state, int dat_files_to_read, int dat_total_files,
             DATManager& dat_manager, MapRenderer* map_renderer)
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
        if (initialization_state == InitializationState::Completed)
        {
            // UI logic goes here
        }
    }
}
