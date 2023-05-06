#pragma once
#include "InstanceParty.h"
#include "ConnectionData.h"

extern bool is_shutting_down;

class PartyManager
{
public:
    // Main function. Should run in its own thread.
    // Exists when the program terminates.
    void terminate()
    {
        for (auto& [_, thread] : party_id_to_party_thread)
        {
            thread.join();
        }
    }
    void run(ConnectionData& connection_data)
    {

        while (! is_shutting_down)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            auto connection_ids = connection_data.get_connected_client_ids();
            std::vector<std::string> diff;
            std::set_symmetric_difference(
              connection_ids.begin(), connection_ids.end(), managed_connected_client_ids_.begin(),
              managed_connected_client_ids_.end(), std::inserter(diff, diff.begin()));

            for (const auto& id : diff)
            {
                // Connection not yet managed by party manager
                if (connection_ids.contains(id))
                {
                    managed_connected_client_ids_.insert(id);
                }
                else
                {
                    managed_connected_client_ids_.erase(id);
                    remove_from_party(id);
                }
            }

            // PartyId might not be valid yet such as if the dll was injected on
            // the login screen, loading screen, character select menu etc.
            std::vector<std::string> to_delete;
            for (const auto id : managed_connected_client_ids_)
            {
                const auto client_data = connection_data.get_client_data(id, client_data_buffer_);
                if (client_data)
                {
                    const auto game_state = client_data->game_state();
                    if (game_state && game_state != GWIPC::GameState::GameState_InGame)
                    {
                        // We are not in-game (i.e. we are in loading screen, char select, etc.)
                        to_delete.push_back(id);
                        continue;
                    }

                    const auto instance = client_data->instance();
                    const auto party = client_data->party();

                    if (! instance || ! party)
                        continue;
                    InstancePartyId party_id = {instance->instance_id(), party->party_id()};
                    if (! is_party_id_valid(party_id))
                        continue;

                    // If the client is already in a party and it is different then remove them from the
                    // current party thread and add them to the thread for the clients current party.
                    // Otherwise continue without doing anything.
                    const auto curr_party_it = client_name_to_party_id.find(id);
                    if (curr_party_it != client_name_to_party_id.end())
                    {
                        if (curr_party_it->second == party_id)
                            continue;

                        // The client is currently in a different party so we must remove them.
                        remove_from_party(id);
                    }

                    // At this point the player is not in a party. So we must add them.
                    client_name_to_party_id.insert({id, party_id});
                    const auto it = party_id_to_client_names.find(party_id);
                    if (it == party_id_to_client_names.end())
                    {
                        party_id_to_client_names.insert({party_id, std::unordered_set<std::string>()});
                    }
                    party_id_to_client_names[party_id].insert(id);

                    // Create party thread if it does not already exist
                    const auto it2 = party_id_to_party_thread_objects.find(party_id);
                    if (it2 == party_id_to_party_thread_objects.end())
                    {
                        auto it3 =
                          party_id_to_party_thread_objects
                            .insert(
                              {party_id, InstanceParty(connection_data, party_id_to_client_names[party_id])})
                            .first;
                        auto new_party_thread = std::thread(&InstanceParty::run, &it3->second);
                        party_id_to_party_thread.insert({party_id, std::move(new_party_thread)});
                    }
                }
            }

            for (const auto& id : to_delete)
            {
                managed_connected_client_ids_.erase(id);
                remove_from_party(id);
            }
        }
    }

    void remove_from_party(const std::string& id)
    {
        const auto curr_party_it = client_name_to_party_id.find(id);
        if (curr_party_it != client_name_to_party_id.end())
        {
            const auto party_id = curr_party_it->second;

            party_id_to_client_names[party_id].erase(id);

            // If the party is empty then delete thread and data structures.
            if (party_id_to_client_names[party_id].size() == 0)
            {
                party_id_to_party_thread[party_id].join();
                party_id_to_client_names.erase(party_id);
                party_id_to_party_thread_objects.erase(party_id);
                party_id_to_party_thread.erase(party_id);
            }

            //
            client_name_to_party_id.erase(id);
        }
    }

    const std::map<InstancePartyId, std::unordered_set<std::string>> get_parties_client_names()
    {
        return party_id_to_client_names;
    }

private:
    std::vector<uint8_t> client_data_buffer_ = std::vector<uint8_t>(GWIPC::CLIENTDATA_SIZE);
    const DWORD timeout_duration_ms = 1000;

    std::set<std::string> managed_connected_client_ids_;

    std::map<InstancePartyId, InstanceParty> party_id_to_party_thread_objects;
    std::map<InstancePartyId, std::thread> party_id_to_party_thread;

    std::map<InstancePartyId, std::unordered_set<std::string>> party_id_to_client_names;
    std::unordered_map<std::string, InstancePartyId> client_name_to_party_id;

    bool is_party_id_valid(InstancePartyId party_id) { return party_id.first > 0 && party_id.second > 0; }
};
