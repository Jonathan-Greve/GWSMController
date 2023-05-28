#include "pch.h"
#include "ShowRecordAndReplayPanel.h"
#include <commdlg.h>

std::wstring openFileDialog()
{
    OPENFILENAME ofn;
    WCHAR file[260] = {0};

    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = NULL; // If you have a window to center over, put its HANDLE here
    ofn.lpstrFile = file;
    ofn.nMaxFile = sizeof(file);
    ofn.lpstrFilter = L"Recording\0*.gw_recording\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileName(&ofn))
    {
        return file;
    }
    return L"";
}

void ShowRecordAndReplayPanel::draw(ImGuiStates& imgui_states, Recorder& recorder, Replayer& replayer)
{
    ImGui::Begin("Recorder and Replayer");

    if (imgui_states.is_replaying || recorder.GetIsRecording())
    {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        ImGui::Button("Start Recording");
        ImGui::PopStyleVar();
    }
    else
    {
        if (ImGui::Button("Start Recording"))
        {
            recorder.start(recording_directory);
            imgui_states.is_recording = true;
        }
    }

    if (! recorder.GetIsRecording())
    {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        ImGui::Button("Stop Recording");
        ImGui::PopStyleVar();
    }
    else
    {
        if (ImGui::Button("Stop Recording"))
        {
            recorder.stop();
            imgui_states.is_recording = false;
        }
    }

    ImGui::Separator();
    // Replayer controls

    ImGui::Text("Selected File: %s", replay_filename.c_str());

    if (ImGui::Button("Select Replay File"))
    {
        std::wstring selected_file = openFileDialog();
        if (! selected_file.empty())
        {
            replay_filename = std::string(selected_file.begin(), selected_file.end());
        }
    }

    // Start replay button
    if (replay_filename.empty() || imgui_states.is_recording || replayer.GetIsReplaying())
    {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        ImGui::Button("Start Replaying");
        ImGui::PopStyleVar();
    }
    else
    {
        if (ImGui::Button("Start Replaying"))
        {
            replayer.start(replay_filename);
            imgui_states.is_replaying = true;
        }
    }

    // Display the current timestamp and elapsed time
    if (replayer.GetIsReplaying())
    {
        auto current_timestamp = replayer.get_current_timestamp();
        auto elapsed_time = static_cast<float>(replayer.get_elapsed_time_seconds());
        auto total_duration = static_cast<float>(replayer.get_total_duration());

        ImGui::Text("Current timestamp: %f",
                    std::chrono::duration<float, std::milli>(current_timestamp.time_since_epoch()).count());
        ImGui::Text("Elapsed time: %f s", elapsed_time);
        ImGui::Text("Total duration : %f s", total_duration);
    }

    // Seek bar
    if (replayer.GetIsReplaying())
    {
        float duration = static_cast<float>(replayer.get_total_duration());
        float elapsed_time = static_cast<float>(replayer.get_elapsed_time_seconds());
        if (ImGui::SliderFloat("Replay position", &elapsed_time, 0.0f, duration))
        {
            // Seek to the new position when the slider is moved
            auto first_timestamp = replayer.get_first_timestamp();
            auto new_timestamp =
              first_timestamp + std::chrono::milliseconds(static_cast<int>(elapsed_time * 1000));
            replayer.set_frame_by_timestamp(new_timestamp);
        }
    }

    // Stop replay button
    if (! replayer.GetIsReplaying())
    {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        ImGui::Button("Stop Replaying");
        ImGui::PopStyleVar();
    }
    else
    {
        if (ImGui::Button("Stop Replaying"))
        {
            replayer.stop();
            imgui_states.is_replaying = false;
        }
    }

    ImGui::End();
}
