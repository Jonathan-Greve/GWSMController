#pragma once
#include "ConnectionData.h"
#include "ImGuiStates.h"

class ShowUpdateOptionsPanel
{
public:
    ShowUpdateOptionsPanel()
    {
    }

    void draw(ImGuiStates& imgui_states, ConnectionData& connection_data)
    {
        if (!ImGui::Begin("Update Options"))
        {
            // Early out if the window is collapsed, as an optimization.
            ImGui::End();
            return;
        }

        for (const auto& connection_id : connection_data.get_connected_client_ids())
        {
            ImGui::Text("Connection: %s", connection_id.c_str());

            const auto update_options = connection_data.get_update_options_for_connection(connection_id);
            if (update_options != nullptr)
            {
                bool only_send_active_quest_description = update_options->only_send_active_quest_description();
                bool only_send_active_quest_objectives = update_options->only_send_active_quest_objectives();
                bool should_update_client_data = update_options->should_update_client_data();

                if (ImGui::Checkbox((connection_id + ": only send active quest description").c_str(), &only_send_active_quest_description) ||
                    ImGui::Checkbox((connection_id + ": only send active quest objectives").c_str(), &only_send_active_quest_objectives) ||
                    ImGui::Checkbox((connection_id + ": should update client data").c_str(), &should_update_client_data))
                {
                    connection_data.set_update_options_for_connection(
                        connection_id,
                        only_send_active_quest_description,
                        only_send_active_quest_objectives,
                        should_update_client_data);
                }
            }
        }

        ImGui::End();
    }
};

