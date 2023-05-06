#pragma once

struct ImGuiStates
{
    bool is_party_window_open = true;
    InstancePartyId selected_party = InstancePartyId(0, 0);
};
