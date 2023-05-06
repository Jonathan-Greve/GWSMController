#include "pch.h"
#include "ShowClientsConnected.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

void ShowClientsConnected::operator()(ImGuiStates& imgui_states, ConnectionData& connection_data,
                                      const std::array<GW_skill, 3432>& skills)
{
    {
        const auto connected_client_ids = connection_data.get_connected_client_ids();

        ImGui::Begin("Connected clients info");
        // Display the number of connections
        ImGui::Text("Number of connections:");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.2, 1, 0.3, 1), std::to_string(connected_client_ids.size()).c_str());

        // Display a list of character names, character classes, locations, party IDs, and party sizes
        ImGui::Columns(8, "connection_columns");
        ImGui::Separator();
        ImGui::Text("Game state");
        ImGui::NextColumn();
        ImGui::Text("Email");
        ImGui::NextColumn();
        ImGui::Text("Character Name");
        ImGui::NextColumn();
        ImGui::Text("Agent ID");
        ImGui::NextColumn();
        ImGui::Text("Character Class");
        ImGui::NextColumn();
        ImGui::Text("Location");
        ImGui::NextColumn();
        ImGui::Text("Party ID");
        ImGui::NextColumn();
        ImGui::Text("Party Size");
        ImGui::NextColumn();
        ImGui::Separator();

        for (const auto& id : connected_client_ids)
        {
            auto client_data = connection_data.get_client_data(id, client_data_buffer_);
            if (client_data)
            {
                auto game_state_name = GWIPC::EnumNamesGameState()[client_data->game_state()];
                if (game_state_name)
                {
                    ImGui::Text("%s", game_state_name);
                }
                ImGui::NextColumn();

                ImGui::Text("%s", id.c_str());
                ImGui::NextColumn();

                if (client_data->character() && client_data->character()->agent_living() &&
                    client_data->character()->agent_living()->agent())
                {
                    auto name = client_data->character()->agent_living()->name();
                    ImGui::Text("%s", name->c_str());
                    ImGui::NextColumn();

                    ImGui::Text("%u", client_data->character()->agent_living()->agent()->agent_id());
                    ImGui::NextColumn();

                    auto primary = client_data->character()->agent_living()->primary_profession();
                    auto primary_name = GWIPC::EnumNamesProfession()[primary];
                    auto secondary = client_data->character()->agent_living()->secondary_profession();
                    auto secondary_name = GWIPC::EnumNamesProfession()[secondary];
                    ImGui::Text("%s/%s", primary_name, secondary_name);
                    ImGui::NextColumn();
                }
                else
                {
                    ImGui::NextColumn();
                    ImGui::NextColumn();
                    ImGui::NextColumn();
                }

                int instance_id = -1;
                if (client_data->instance())
                {
                    auto map_id = client_data->instance()->map_id();
                    ImGui::Text("%s", GW::Constants::NAME_FROM_ID[map_id]);

                    instance_id = client_data->instance()->instance_id();
                }
                ImGui::NextColumn();

                uint32_t num_hero_members = 0;
                uint32_t num_player_members = 0;
                uint32_t num_henchman_members = 0;
                if (client_data->party())
                {
                    auto party_id = client_data->party()->party_id();
                    ImGui::Text("(%u, %u)", instance_id, party_id);

                    auto hero_members = client_data->party()->hero_members();
                    if (hero_members)
                    {
                        num_hero_members += hero_members->size();
                    }

                    auto player_members = client_data->party()->player_members();
                    if (player_members)
                    {
                        num_player_members += player_members->size();
                    }

                    auto henchman_members = client_data->party()->henchman_members();
                    if (henchman_members)
                    {
                        num_henchman_members += henchman_members->size();
                    }
                }
                ImGui::NextColumn();

                ImGui::Text("%d(A)|%d(P)|%d(H)|%d(h)",
                            num_hero_members + num_player_members + num_henchman_members, num_player_members,
                            num_hero_members, num_henchman_members);
                ImGui::NextColumn();
            }
        }
        ImGui::Columns(1);
        ImGui::End();
    }
}
