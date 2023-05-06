#pragma once
#include "ConnectionData.h"

extern bool is_shutting_down;

class InstanceParty
{
public:
    InstanceParty(const ConnectionData& connection_data,
                  const std::unordered_set<std::string>& client_names_in_party)
        : m_connection_data(connection_data)
        , m_client_names_in_party(client_names_in_party)
    {
    }

    void run()
    {
        while (! is_shutting_down && m_client_names_in_party.size() > 0)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

private:
    const ConnectionData& m_connection_data;
    const std::unordered_set<std::string>& m_client_names_in_party;
};
