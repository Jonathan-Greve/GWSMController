#pragma once
extern bool is_shutting_down;

extern bool is_replaying_ext;

class ConnectionData
{
public:
    ConnectionData()
    {
        // Signal event so we always read the connection data
        // the first time update() is called.
        SetEvent(m_connections_shared_memory.get_event_handle());
    }

    void terminate()
    {
        // update() might be waiting on the event so release it.
        SetEvent(m_connections_shared_memory.get_event_handle());

        for (auto& [_, sm] : client_sm)
            sm->terminate();
        m_connections_shared_memory.terminate();
    }

    // Update data. Blocking until connection data changes.
    // We use is_replaying to skip a mutex lock when replaying a recording.
    // This is because the replayer holds the mutex while replaying to avoid clients connecting or disconnecting.
    // It restores the client connection state after replaying and releases the mutex. But that means we should
    // skip waiting for the mutex while replaying.
    void run()
    {
        while (! is_shutting_down)
        {
            // Wait for event to be signaled
            if (! is_replaying_ext)
            {
                auto wait_result =
                  WaitForSingleObject(m_connections_shared_memory.get_event_handle(), INFINITE);
                if (is_shutting_down || wait_result != WAIT_OBJECT_0)
                    return;
            }

            // Then lock mutex before accessing shared memory data (or skip if replaying)

            if (! is_replaying_ext)
            {
                WaitForSingleObject(m_connections_shared_memory.get_mutex_handle(), INFINITE);
            }

            auto connection_ids = m_connections_shared_memory.get_connections_ids();
            ResetEvent(m_connections_shared_memory.get_event_handle());

            if (! is_replaying_ext)
            {
                ReleaseMutex(m_connections_shared_memory.get_mutex_handle());
            }

            std::set<std::string> new_connection_ids(connection_ids.begin(), connection_ids.end());

            // Get set difference between new and locally cached connection names.
            std::vector<std::string> diff;
            std::set_symmetric_difference(new_connection_ids.begin(), new_connection_ids.end(),
                                          m_connection_ids.begin(), m_connection_ids.end(),
                                          std::inserter(diff, diff.begin()));

            for (const auto& id : diff)
            {
                // Remove dropped connections and remove shared memory
                if (! new_connection_ids.contains(id))
                {
                    {
                        std::scoped_lock lock(connection_ids_mutex_);
                        m_connection_ids.erase(id);
                    }

                    const auto it = client_sm.find(id);
                    if (it != client_sm.end())
                    {
                        it->second->terminate();
                        client_sm.erase(it);
                    }

                    const auto it_update = update_options_managers.find(id);
                    if (it_update != update_options_managers.end())
                    {
                        it_update->second->terminate();
                        update_options_managers.erase(it_update);
                    }
                }
                // Add new connections and create_or_open shared memory for client
                else
                {
                    {
                        std::scoped_lock lock(connection_ids_mutex_);
                        m_connection_ids.insert(id);
                    }
                    auto pClientData = std::make_unique<GWIPC::SharedMemory>(id, GWIPC::CLIENTDATA_SIZE);
                    client_sm.try_emplace(id, std::move(pClientData));

                    auto pUpdateOptionsManager = std::make_unique<GWIPC::UpdateOptionsManager>(id);
                    update_options_managers.try_emplace(id, std::move(pUpdateOptionsManager));
                }
            }
            // Assert that our locally cached set of connection match the one in shared memory
            assert(m_connection_ids == new_connection_ids);
        }
    }

    const std::set<std::string> get_connected_client_ids()
    {
        std::scoped_lock lock(connection_ids_mutex_);
        return m_connection_ids;
    };

    const GWIPC::ClientData* get_client_data(std::string connection_id, std::vector<uint8_t>& buf)
    {
        const auto sm = get_client_shared_memory(connection_id);
        if (sm)
        {
            auto sm_data = sm->get_data();
            if (sm_data)
            {
                GWIPC::SharedMemoryLock lock(*sm);
                auto data_info = sm->get_data_info();
                if (data_info)
                {
                    memcpy(buf.data(), sm_data, data_info->data_size);
                    auto client_data = GWIPC::GetClientData(buf.data());

                    return client_data;
                }
            }
        }

        return nullptr;
    }

    const GWIPC::ClientData* get_client_data(std::string connection_id, std::vector<uint8_t>& buf,
                                             int* size_out)
    {
        const auto sm = get_client_shared_memory(connection_id);
        if (sm)
        {
            auto sm_data = sm->get_data();
            if (sm_data)
            {
                GWIPC::SharedMemoryLock lock(*sm);
                auto data_info = sm->get_data_info();
                if (data_info)
                {
                    memcpy(buf.data(), sm_data, data_info->data_size);
                    auto client_data = GWIPC::GetClientData(buf.data());

                    *size_out = data_info->data_size;
                    return client_data;
                }
            }
        }

        return nullptr;
    }

    void set_client_data(const std::string& connection_id, const std::vector<uint8_t>& buf)
    {
        auto sm = get_client_shared_memory(connection_id);
        if (sm)
        {
            GWIPC::SharedMemoryLock lock(*sm);
            sm->write_data(buf.data(), buf.size());
        }
    }

    const GWIPC::UpdateOptions* get_update_options_for_connection(const std::string& connection_id)
    {
        const auto it = update_options_managers.find(connection_id);
        if (it != update_options_managers.end())
        {
            return it->second->get_update_options();
        }

        return nullptr;
    }

    void set_update_options_for_connection(const std::string& connection_id,
        bool only_send_active_quest_description,
        bool only_send_active_quest_objectives,
        bool should_update_client_data)
    {
        auto it = update_options_managers.find(connection_id);
        if (it != update_options_managers.end())
        {
            it->second->update(
                only_send_active_quest_description,
                only_send_active_quest_objectives,
                should_update_client_data);
        }
    }


private:
    std::mutex connection_ids_mutex_;

    GWIPC::ConnectionManager m_connections_shared_memory;

    // Hold the shared memory objects containing the client data
    std::unordered_map<std::string, std::unique_ptr<GWIPC::SharedMemory>> client_sm;

    std::unordered_map<std::string, std::unique_ptr<GWIPC::UpdateOptionsManager>> update_options_managers;

    // Contains the email addresses of the connected clients.
    std::set<std::string> m_connection_ids;

    GWIPC::SharedMemory* get_client_shared_memory(std::string connection_id)
    {
        const auto it = client_sm.find(connection_id);
        if (it != client_sm.end())
        {
            return it->second.get();
        }

        return nullptr;
    }
};
