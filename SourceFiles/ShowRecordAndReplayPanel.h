#pragma once
#include <filesystem>
#include "ImGuiStates.h"
#include "Recorder.h"

class ShowRecordAndReplayPanel
{
public:
    ShowRecordAndReplayPanel()
    {
        namespace fs = std::filesystem;
        fs::path exePath = fs::current_path();
        fs::path recordDir = exePath / "GWSM-Recordings";

        // Create the directory if it doesn't exist
        if (!fs::exists(recordDir))
        {
            fs::create_directory(recordDir);
        }

        recording_directory = recordDir.string();
    }

    void operator()(ImGuiStates& imgui_states, Recorder& recorder);

private:
    std::string recording_directory;
};
