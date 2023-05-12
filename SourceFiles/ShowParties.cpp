#include "pch.h"
#include "ShowParties.h"

void set_map_to_render(DATManager& dat_manager, int map_file_hash, MapRenderer* map_renderer)
{
    const auto hash_index = dat_manager.get_hash_index();

    const auto it = hash_index.find(map_file_hash);

    if (it != hash_index.end())
    {
        // Found a matching MFTEntry. The index can be obtained by subtracting mft.begin() from the iterator.
        int index = it->second.first;

        const auto selected_ffna_map_file = dat_manager.parse_ffna_map_file(index);

        if (selected_ffna_map_file.terrain_chunk.terrain_heightmap.size() > 0 &&
            selected_ffna_map_file.terrain_chunk.terrain_heightmap.size() ==
              selected_ffna_map_file.terrain_chunk.terrain_x_dims *
                selected_ffna_map_file.terrain_chunk.terrain_y_dims)
        {

            auto& terrain_texture_filenames = selected_ffna_map_file.terrain_texture_filenames.array;
            std::vector<DatTexture> terrain_dat_textures;
            for (int i = 0; i < terrain_texture_filenames.size(); i++)
            {
                auto decoded_filename =
                  decode_filename(selected_ffna_map_file.terrain_texture_filenames.array[i].filename.id0,
                                  selected_ffna_map_file.terrain_texture_filenames.array[i].filename.id1);
                auto mft_entry_it = hash_index.find(decoded_filename);
                if (mft_entry_it != hash_index.end())
                {
                    const DatTexture dat_texture =
                      dat_manager.parse_ffna_texture_file(mft_entry_it->second.first);
                    terrain_dat_textures.push_back(dat_texture);
                }
            }

            const auto dat_texture =
              map_renderer->GetTextureManager()->BuildTextureAtlas(terrain_dat_textures, 8, 8);
            int atlas_texture_id = 0;

            if (dat_texture.width > 0 && dat_texture.height > 0)
            {
                map_renderer->GetTextureManager()->CreateTextureFromRGBA(
                  dat_texture.width, dat_texture.height, dat_texture.rgba_data.data(), &atlas_texture_id,
                  it->second.second->Hash);
            }

            auto& terrain_texture_indices =
              selected_ffna_map_file.terrain_chunk.terrain_texture_indices_maybe;

            auto& terrain_texture_blend_weights =
              selected_ffna_map_file.terrain_chunk.terrain_texture_blend_weights_maybe;

            // Create terrain
            auto terrain = std::make_unique<Terrain>(selected_ffna_map_file.terrain_chunk.terrain_x_dims,
                                                     selected_ffna_map_file.terrain_chunk.terrain_y_dims,
                                                     selected_ffna_map_file.terrain_chunk.terrain_heightmap,
                                                     terrain_texture_indices, terrain_texture_blend_weights,
                                                     selected_ffna_map_file.map_info_chunk.map_bounds);
            map_renderer->SetTerrain(std::move(terrain), atlas_texture_id);
        }

        std::vector<FFNA_ModelFile> map_models;

        // Load models
        for (int i = 0; i < selected_ffna_map_file.prop_filenames_chunk.array.size(); i++)
        {
            auto decoded_filename =
              decode_filename(selected_ffna_map_file.prop_filenames_chunk.array[i].filename.id0,
                              selected_ffna_map_file.prop_filenames_chunk.array[i].filename.id1);
            auto mft_entry_it = hash_index.find(decoded_filename);
            if (mft_entry_it != hash_index.end())
            {
                auto type = mft_entry_it->second.second->type;
                if (type == FFNA_Type2)
                {
                    map_models.emplace_back(dat_manager.parse_ffna_model_file(mft_entry_it->second.first));
                }
            }
        }

        // Load models
        for (int i = 0; i < selected_ffna_map_file.more_filnames_chunk.array.size(); i++)
        {
            auto decoded_filename =
              decode_filename(selected_ffna_map_file.more_filnames_chunk.array[i].filename.id0,
                              selected_ffna_map_file.more_filnames_chunk.array[i].filename.id1);
            auto mft_entry_it = hash_index.find(decoded_filename);
            if (mft_entry_it != hash_index.end())
            {
                auto type = mft_entry_it->second.second->type;
                if (type == FFNA_Type2)
                {
                    auto map_model = dat_manager.parse_ffna_model_file(mft_entry_it->second.first);
                    map_models.emplace_back(map_model);
                }
            }
        }

        for (int i = 0; i < selected_ffna_map_file.props_info_chunk.prop_array.props_info.size(); i++)
        {
            PropInfo prop_info = selected_ffna_map_file.props_info_chunk.prop_array.props_info[i];

            if (prop_info.filename_index < map_models.size())
            {
                if (auto ffna_model_file_ptr = &map_models[prop_info.filename_index])
                {
                    if (ffna_model_file_ptr->parsed_correctly)
                    {
                        // Load geometry
                        std::vector<Mesh> prop_meshes;
                        for (int j = 0; j < ffna_model_file_ptr->geometry_chunk.models.size(); j++)
                        {
                            Mesh prop_mesh = ffna_model_file_ptr->GetMesh(j);
                            if ((prop_mesh.indices.size() % 3) == 0)
                            {
                                prop_meshes.push_back(prop_mesh);
                            }
                        }

                        // Load textures
                        std::vector<int> texture_ids;
                        for (int j = 0;
                             j < ffna_model_file_ptr->texture_filenames_chunk.texture_filenames.size(); j++)
                        {
                            auto texture_filename =
                              ffna_model_file_ptr->texture_filenames_chunk.texture_filenames[j];
                            auto decoded_filename =
                              decode_filename(texture_filename.id0, texture_filename.id1);

                            int texture_id =
                              map_renderer->GetTextureManager()->GetTextureIdByHash(decoded_filename);
                            if (texture_id >= 0)
                            {
                                texture_ids.push_back(texture_id);
                                continue;
                            }

                            auto mft_entry_it = hash_index.find(decoded_filename);
                            if (mft_entry_it != hash_index.end())
                            {
                                // Get texture from .dat
                                auto dat_texture =
                                  dat_manager.parse_ffna_texture_file(mft_entry_it->second.first);

                                // Create texture
                                auto HR = map_renderer->GetTextureManager()->CreateTextureFromRGBA(
                                  dat_texture.width, dat_texture.height, dat_texture.rgba_data.data(),
                                  &texture_id, decoded_filename);
                                texture_ids.push_back(texture_id);
                            }
                        }

                        // The number of textures might exceed 8 for a model since each submodel might use up to 8 separate textures.
                        // So for each submodel's Mesh we must make sure that the uv_indices[i] < 8 and tex_indices[i] < 8.
                        std::vector<std::vector<int>> per_mesh_tex_ids(prop_meshes.size());
                        for (int i = 0; i < prop_meshes.size(); i++)
                        {
                            std::vector<uint8_t> mesh_tex_indices;
                            for (int j = 0; j < prop_meshes[i].tex_indices.size(); j++)
                            {
                                int tex_index = prop_meshes[i].tex_indices[j];
                                per_mesh_tex_ids[i].push_back(texture_ids[tex_index]);

                                mesh_tex_indices.push_back(j);
                            }

                            prop_meshes[i].tex_indices = mesh_tex_indices;
                        }

                        std::vector<PerObjectCB> per_object_cbs;
                        per_object_cbs.resize(prop_meshes.size());
                        for (int j = 0; j < per_object_cbs.size(); j++)
                        {
                            DirectX::XMFLOAT3 translation(prop_info.x, prop_info.y, prop_info.z);
                            float sin_angle = prop_info.sin_angle;
                            float cos_angle = prop_info.cos_angle;
                            float rotation_angle = std::atan2(
                              sin_angle,
                              cos_angle); // Calculate the rotation angle from the sine and cosine values

                            float scale = prop_info.scaling_factor;

                            DirectX::XMMATRIX scaling_matrix = DirectX::XMMatrixScaling(scale, scale, scale);
                            DirectX::XMMATRIX rotation_matrix = DirectX::XMMatrixRotationY(rotation_angle);
                            DirectX::XMMATRIX translation_matrix =
                              DirectX::XMMatrixTranslationFromVector(XMLoadFloat3(&translation));

                            DirectX::XMMATRIX transform_matrix = DirectX::XMMatrixMultiply(
                              scaling_matrix, DirectX::XMMatrixMultiply(rotation_matrix, translation_matrix));

                            DirectX::XMStoreFloat4x4(&per_object_cbs[j].world, transform_matrix);

                            auto& prop_mesh = prop_meshes[j];
                            if (prop_mesh.uv_coord_indices.size() != prop_mesh.tex_indices.size() ||
                                prop_mesh.uv_coord_indices.size() >= MAX_NUM_TEX_INDICES)
                            {
                                return; // Failed, maybe throw here on handle error.
                            }

                            per_object_cbs[j].num_uv_texture_pairs = prop_mesh.uv_coord_indices.size();

                            for (int k = 0; k < prop_mesh.uv_coord_indices.size(); k++)
                            {
                                int index0 = k / 4;
                                int index1 = k % 4;

                                per_object_cbs[j].uv_indices[index0][index1] =
                                  (uint32_t)prop_mesh.uv_coord_indices[k];
                                per_object_cbs[j].texture_indices[index0][index1] =
                                  (uint32_t)prop_mesh.tex_indices[k];
                                per_object_cbs[j].blend_flags[index0][index1] =
                                  (uint32_t)prop_mesh.blend_flags[k];
                            }
                        }

                        auto mesh_ids = map_renderer->AddProp(prop_meshes, per_object_cbs, i);
                        for (int i = 0; i < mesh_ids.size(); i++)
                        {
                            int mesh_id = mesh_ids[i];
                            auto& mesh_texture_ids = per_mesh_tex_ids[i];

                            map_renderer->GetMeshManager()->SetTexturesForMesh(
                              mesh_id, map_renderer->GetTextureManager()->GetTextures(mesh_texture_ids), 0);
                        }
                    }
                }
            }
        }
    }
    else
    {
        // No matching MFTEntry found.
    }
}

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
                             PartyManager& party_manager, const std::array<GW_skill, 3432>& skills,
                             DATManager& dat_manager, MapRenderer* map_renderer)
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
                {
                    imgui_states.selected_party = party_id;
                    set_map_to_render(dat_manager, 0x1b97d, map_renderer);
                }
            }
            ImGui::EndChild();
        }
        ImGui::SameLine();

        // Right
        if (imgui_states.selected_party != InstancePartyId(0, 0))
        {
            const auto& selected_party_connection_ids = parties_client_names[imgui_states.selected_party];

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
                    for (const auto& id : selected_party_connection_ids)
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
