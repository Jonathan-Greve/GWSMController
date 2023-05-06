#pragma once

struct GW_skill
{
    ID3D11ShaderResourceView* skill_icon_texture;
    int width = 0;
    int height = 0;

    static void load_gw_skill_data(std::array<GW_skill, 3432>& skills, ID3D11Device1* p_device);
};
