#include "pch.h"
#include "GW_skill.h"
#include "get_filenames.h"
#include "LoadTextureFromFile.h"

void GW_skill::load_gw_skill_data(std::array<GW_skill, 3432>& skills, ID3D11Device1* p_device)
{

    // Load GW skill images to textures for rendering
    std::string path = "..\\cropped_skills";
    std::vector<std::string> filenames;

    get_filenames(path, filenames);
    auto avail_threads = std::thread::hardware_concurrency();

    // Divide the filenames into chunks
    std::vector<std::vector<std::string>> chunks(avail_threads);
    for (size_t i = 0; i < filenames.size(); i++)
    {
        chunks[i % avail_threads].push_back(filenames[i]);
    }

    // Create a thread for each chunk of filenames
    std::vector<std::thread> threads;
    for (const auto& chunk : chunks)
    {
        threads.emplace_back(
          [&]
          {
              for (const auto& file_name : chunk)
              {
                  // Get the integer at the end of the file_name string
                  auto skill_id = get_first_integer_in_string(file_name);

                  std::string file_path = std::format("{}/{}", path, file_name);
                  auto skill = GW_skill();

                  bool ret = LoadTextureFromFile(file_path.c_str(), &skill.skill_icon_texture, &skill.width,
                                                 &skill.height, p_device);

                  skills[skill_id] = skill;
              }
          });
    }

    // Wait for all threads to finish
    for (auto& thread : threads)
    {
        thread.join();
    }
}
