#pragma once
#include "ConnectionData.h"
#include <chrono>
#include <fstream>
#include <lz4.h>

class Replayer
{
public:
    Replayer(ConnectionData& connection_data)
        : connection_data_(connection_data)
    {
    }

    void start(const std::string& filename)
    {
        is_replaying_ = true;
        in_.open(filename, std::ios::binary);

        // Clear any previous data
        temp_data_.clear();
        uncompressed_data_.clear();
        read_position_ = 0;
        timestamps_.clear();

        // Start the replay timer
        start_time_ = std::chrono::high_resolution_clock::now();

        // Read and decompress data
        read_and_decompress_data();

        // Loop over frames to fill timestamps_ and find last_timestamp_
        size_t original_read_position = read_position_;
        while (can_read_frame())
        {
            auto frame = get_next_frame();
            timestamps_.push_back(frame.time);
        }
        read_position_ = original_read_position; // Restore the read position
    }

    void stop()
    {
        is_replaying_ = false;
        if (in_.is_open())
        {
            in_.close();
        }
    }

    void update()
    {
        if (is_replaying_)
        {
            if (can_read_frame())
            {
                auto next_frame = peek_next_frame();
                auto now = std::chrono::high_resolution_clock::now();
                auto elapsed_time = now - start_time_;

                if (next_frame.time <= start_time_ + elapsed_time)
                {
                    auto frame = get_next_frame();
                    connection_data_.set_client_data(frame.client_id, frame.buf);
                    current_time_ = frame.time;
                }
            }
            else
            {
                stop();
            }
        }
    }

    std::chrono::high_resolution_clock::time_point get_current_timestamp() const { return current_time_; }

    double get_elapsed_time_seconds() const
    {
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_);
        return elapsed_time.count() / 1000.0; // convert milliseconds to seconds
    }

    std::chrono::high_resolution_clock::time_point get_last_timestamp() const
    {
        return timestamps_[timestamps_.size() - 1];
    }
    std::chrono::high_resolution_clock::time_point get_first_timestamp() const { return timestamps_[0]; }

    double get_total_duration() const
    {
        auto duration = get_last_timestamp() - start_time_;
        return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() / 1000.0;
    }

    void set_frame_by_timestamp(std::chrono::high_resolution_clock::time_point timestamp)
    {
        auto nearest_it = std::upper_bound(timestamps_.begin(), timestamps_.end(), timestamp);
        if (nearest_it != timestamps_.begin())
            --nearest_it; // Find the largest element less than or equal to timestamp
        auto index = std::distance(timestamps_.begin(), nearest_it);
        read_position_ = index * (sizeof(long long) + sizeof(size_t) + GWIPC::CLIENTDATA_SIZE);
    }

    bool GetIsReplaying() { return is_replaying_; }

private:
    ConnectionData& connection_data_;
    bool is_replaying_ = false;
    std::ifstream in_;

    std::vector<char> temp_data_;
    std::vector<char> uncompressed_data_;
    size_t read_position_ = 0;

    struct Frame
    {
        std::string client_id;
        std::vector<uint8_t> buf;
        std::chrono::high_resolution_clock::time_point time;
    };

    std::chrono::high_resolution_clock::time_point start_time_;
    std::chrono::high_resolution_clock::time_point current_time_;

    std::vector<std::chrono::high_resolution_clock::time_point> timestamps_;

    Frame peek_next_frame()
    {
        auto original_read_position = read_position_;
        auto frame = get_next_frame();
        read_position_ = original_read_position; // Restore the read position
        return frame;
    }

    bool read_and_decompress_data()
    {
        // Read the uncompressed size
        size_t uncompressed_size;
        in_.read(reinterpret_cast<char*>(&uncompressed_size), sizeof(uncompressed_size));

        // Read the compressed data
        std::vector<char> compressed_data(std::istreambuf_iterator<char>(in_), {});

        // Decompress the data
        uncompressed_data_.resize(uncompressed_size);
        int decompressed_size = LZ4_decompress_safe(compressed_data.data(), uncompressed_data_.data(),
                                                    compressed_data.size(), uncompressed_size);

        if (decompressed_size <= 0)
        {
            return false;
        }
        else
        {
            uncompressed_data_.resize(decompressed_size);
        }

        return true;
    }

    bool can_read_frame() const
    {
        // Each frame needs to contain a timestamp (represented by a long long int),
        // the size of the client_id string (size_t), the client_id itself and the buffer.
        // If the remaining data is less than the size of timestamp + size of client_id, return false
        return read_position_ + sizeof(long long) + sizeof(size_t) <= uncompressed_data_.size();
    }

    Frame get_next_frame()
    {
        Frame frame;

        // Extract and erase timestamp
        long long duration;
        std::memcpy(&duration, uncompressed_data_.data() + read_position_, sizeof(duration));
        frame.time = std::chrono::high_resolution_clock::time_point(std::chrono::milliseconds(duration));
        read_position_ += sizeof(duration);

        // Extract and erase size of client_id
        size_t client_id_length;
        std::memcpy(&client_id_length, uncompressed_data_.data() + read_position_, sizeof(client_id_length));
        read_position_ += sizeof(client_id_length);

        // Extract and erase client_id
        frame.client_id = std::string(uncompressed_data_.data() + read_position_, client_id_length);
        read_position_ += client_id_length;

        // The remaining data in the frame is the buffer
        size_t buf_size = GWIPC::CLIENTDATA_SIZE;
        frame.buf = std::vector<uint8_t>(uncompressed_data_.data() + read_position_,
                                         uncompressed_data_.data() + read_position_ + buf_size);
        read_position_ += buf_size;

        return frame;
    }
};
