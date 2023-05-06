#pragma once
#include <filesystem>
#include <iostream>

// Function to store filenames in a vector using a relative path
void get_filenames(const std::string& path, std::vector<std::string>& filenames)
{
    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(path))
        {
            // Skip hidden files and directories
            if (entry.path().filename().string()[0] == '.')
                continue;
            filenames.push_back(entry.path().filename().string());
        }
    }
    catch (std::filesystem::filesystem_error& e)
    {
        // Could not open directory
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
