#include "pch.h"
#include "ShowParties.h"

void ShowParties::built_unit_frame(const GWIPC::AgentLiving* agent_living, const GWIPC::Skillbar* skillbar,
                                   const std::array<GW_skill, 3432>& skills)
{
    // UnitFrame
    {
        // Get the available content region within the window
        //auto contentRegion = ImGui::GetContentRegionAvail();

        //// Calculate the size and position of the name label
        //auto nameLabelSize = ImVec2(contentRegion.x, 15);

        if (agent_living->name())
            ImGui::Text(agent_living->name()->c_str());

        //auto healthAndEnergyBarSize = ImVec2(contentRegion.x, 15);

        // Push the red color style for the health bar
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));

        if (agent_living->health())
            ImGui::ProgressBar(agent_living->health());

        // Pop the red color style for the health bar
        ImGui::PopStyleColor();

        // Calculate the size and position of the energy bar
        // Push the blue color style for the energy bar
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));

        // Draw the energy bar
        if (agent_living->energy())
            ImGui::ProgressBar(agent_living->energy());

        // Pop the blue color style for the energy bar
        ImGui::PopStyleColor();

        // Calculate the size of each image button based on the available content region and the number of buttons
        //ImVec2 buttonSize(contentRegion.x / 8, contentRegion.x / 8);

        float border_thickness = 1;
        float spacing_width = 1;

        // Draw the skill bar
        auto button_width =
          (ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x) / 8 -
          border_thickness * 2 - spacing_width + spacing_width / 8;

        if (skillbar && skillbar->skills())
        {
            for (int i = 0; i < 8; i++)
            {
                const auto gwipc_skill = skillbar->skills()->Get(i);
                if (gwipc_skill && gwipc_skill->skill_id() >= 0)
                {
                    const auto skill_id = gwipc_skill->skill_id();

                    auto skill = skills[skill_id];

                    // Unique Id
                    std::string button_id = std::format("##skill_image:{}:{}", i, (uint32_t)(skillbar));
                    ImVec2 buttonSize(button_width, button_width);
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
                                        ImVec2(border_thickness, border_thickness));
                    if (ImGui::ImageButton(button_id.c_str(), skill.skill_icon_texture, buttonSize))

                    {
                        // Handle button click
                    }
                    ImGui::PopStyleVar();
                    ImGui::SameLine(0, spacing_width);
                }
            }
        }
        ImGui::Separator();
    }
}

void ShowParties::operator()(ImGuiStates& imgui_states, ConnectionData& connection_data,
                             PartyManager& party_manager, const std::array<GW_skill, 3432>& skills)
{

    if (ImGui::Begin("Parties info", &imgui_states.is_party_window_open))
    {

        auto parties_client_names = party_manager.get_parties_client_names();

        // Left
        {
            ImGui::BeginChild("Parties", ImVec2(250, 0), true);
            if (parties_client_names.size() == 0)
            {
                imgui_states.selected_party = InstancePartyId(0, 0);
            }

            for (const auto& [party_id, client_names] : parties_client_names)
            {
                // FIXME: Good candidate to use ImGuiSelectableFlags_SelectOnNav
                char label[128];
                sprintf(label, "Id:(%u, %u)", party_id.first, party_id.second);
                if (ImGui::Selectable(label, imgui_states.selected_party == party_id) ||
                    imgui_states.selected_party == InstancePartyId(0, 0) ||
                    ! parties_client_names.contains(imgui_states.selected_party))
                    imgui_states.selected_party = party_id;
            }
            ImGui::EndChild();
        }
        ImGui::SameLine();

        // Right
        if (imgui_states.selected_party != InstancePartyId(0, 0))
        {
            const auto& selected_party_ids = parties_client_names[imgui_states.selected_party];

            ImGui::BeginGroup();
            ImGui::BeginChild(
              "item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
            ImGui::Text("Party id: (%u, %u)", imgui_states.selected_party.first,
                        imgui_states.selected_party.second);
            ImGui::Separator();
            if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
            {
                if (ImGui::BeginTabItem("Unit frames"))
                {
                    for (const auto& id : selected_party_ids)
                    {
                        const auto client_data = connection_data.get_client_data(id, client_data_buffer_);
                        if (client_data && client_data->character())
                        {
                            const auto character = client_data->character();

                            built_unit_frame(character->agent_living(), character->skillbar(), skills);
                        }
                    }

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Details"))
                {
                    ImGui::Text("ID: 0123456789");
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndChild();

            ImGui::EndGroup();
        }

        window_pos = ImGui::GetWindowPos();
        window_width = ImGui::GetWindowWidth();
        window_height = ImGui::GetWindowHeight();
    }

    ImGui::End();
}
