#pragma once
#include "ConnectionData.h"
#include <chrono>
#include <lz4.h> // LZ4 header

class Recorder
{
public:
    Recorder(ConnectionData& connection_data)
        : connection_data_(connection_data)
    {
    }

    void start(const std::string& directory)
    {
        is_recording_ = true;
        filename_ = create_filename(directory);
        out_.open(filename_, std::ios::binary);

        // Clear any previous data
        temp_data_.clear();
        update_counter_ = 0;
        previous_bufs_.clear();
    }

    void stop()
    {
        is_recording_ = false;
        if (out_.is_open())
        {
            compress_and_write_data();
            out_.flush();
            out_.close();
        }
    }

    void update()
    {
        if (is_recording_)
        {
            auto now = std::chrono::high_resolution_clock::now();
            for (const auto& client_id : connection_data_.get_connected_client_ids())
            {
                std::vector<uint8_t> buf(GWIPC::CLIENTDATA_SIZE);
                int data_size = 0;
                const GWIPC::ClientData* client_data =
                  connection_data_.get_client_data(client_id, buf, &data_size);

                buf.resize(data_size);
                if (client_data && buf.size() > 0)
                {
                    handle_client_data(now, client_id, buf);
                }
            }
        }
    }

    bool GetIsRecording() { return is_recording_; }

private:
    ConnectionData& connection_data_;
    bool is_recording_ = false;
    std::string filename_;
    std::ofstream out_;

    std::unordered_map<std::string, std::vector<uint8_t>> previous_bufs_;
    std::vector<char> temp_data_;
    int update_counter_ = 0;

    std::string create_filename(const std::string& directory)
    {
        auto now = std::chrono::system_clock::now();
        std::time_t start_time = std::chrono::system_clock::to_time_t(now);
        std::tm* start_time_tm = std::localtime(&start_time);

        std::ostringstream filename_stream;
        filename_stream << directory << "/GWSM_recording_"
                        << std::put_time(start_time_tm, "%Y-%m-%d_%H-%M-%S") << ".gw_recording";
        return filename_stream.str();
    }

    void handle_client_data(const std::chrono::high_resolution_clock::time_point& now,
                            const std::string& client_id, const std::vector<uint8_t>& buf)
    {
        auto it = previous_bufs_.find(client_id);
        if (it == previous_bufs_.end() || it->second != buf) // if the client_id is new or the buf has changed
        {
            auto duration =
              std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
            insert_into_temp_data(reinterpret_cast<const char*>(&duration), sizeof(duration));

            // Write the length of the client_id string before the string itself
            auto client_id_length = client_id.size();
            insert_into_temp_data(reinterpret_cast<const char*>(&client_id_length), sizeof(client_id_length));
            insert_into_temp_data(client_id.c_str(), client_id_length);

            auto buf_size = buf.size();
            insert_into_temp_data(reinterpret_cast<const char*>(&buf_size), sizeof(buf_size));
            insert_into_temp_data(reinterpret_cast<const char*>(buf.data()), buf_size);
        }
        previous_bufs_[client_id] = buf; // store the buf for next comparison
    }

    void insert_into_temp_data(const char* start, size_t size)
    {
        temp_data_.insert(temp_data_.end(), start, start + size);
    }

    void compress_and_write_data()
    {
        // Compress data before writing
        const int max_dst_size = LZ4_compressBound(temp_data_.size());
        std::vector<char> compressed_temp_data(max_dst_size);
        int compressed_data_size = LZ4_compress_default(temp_data_.data(), compressed_temp_data.data(),
                                                        temp_data_.size(), max_dst_size);

        if (compressed_data_size > 0)
        {
            // Write uncompressed data size
            size_t temp_data_size = temp_data_.size();
            out_.write(reinterpret_cast<const char*>(&temp_data_size), sizeof(temp_data_size));

            // Write compressed data
            out_.write(compressed_temp_data.data(), compressed_data_size);
        }
        else
        {
            // Handle the error
        }
    }
};
