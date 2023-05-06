#pragma once
#include "PartyManager.h"
#include "ConnectionData.h"
#include "ImGuiStates.h"

struct Character
{
    std::string name;
    float health;
    float mana;
    std::array<std::string, 8> skills;
};

// Show the number of clients connected and sharing data.
// Also show their email addresses and uptime.
class ShowClientsConnected
{
    static inline std::vector<uint8_t> client_data_buffer_ = std::vector<uint8_t>(GWIPC::CLIENTDATA_SIZE);

public:
    void operator()(ImGuiStates& imgui_states, ConnectionData& connection_data,
                    const std::array<GW_skill, 3432>& skills);
};
