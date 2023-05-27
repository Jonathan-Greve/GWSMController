#include "pch.h"
#include "ShowRecordAndReplayPanel.h"

void ShowRecordAndReplayPanel::operator()(ImGuiStates& imgui_states, Recorder& recorder)
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

    // Replayer mock controls
    if (imgui_states.is_recording)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        ImGui::Button("Start Replaying");
        ImGui::PopStyleVar();
    }
    else
    {
        if (ImGui::Button("Start Replaying"))
        {
            // Not implemented yet.
            imgui_states.is_replaying = true;
        }
    }

    if (ImGui::Button("Stop Replaying"))
    {
        // Not implemented yet.
        imgui_states.is_replaying = false;
    }

    ImGui::End();
}
