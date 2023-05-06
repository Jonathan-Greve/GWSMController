#include "pch.h"
#include "NavMeshManager.h"
#include <iostream>
#include <fstream>
#include <filesystem>

const NavMesh& NavMeshManager::get_navmesh(const uint32_t map_id, const std::string& file_path)
{
    // Read nav_mesh from file if not cached
    if (nav_meshes[map_id].vertices.size() == 0)
    {
        if (! cached_nav_mesh_data.contains(map_id))
        {
            const auto nav_mesh_filepath = file_path;
            try_cache_nav_mesh(nav_mesh_filepath);
        }

        // Draw nav_mesh if cached successfully
        const auto cached_nav_mesh_it = cached_nav_mesh_data.find(map_id);
        if (cached_nav_mesh_it != cached_nav_mesh_data.end())
        {
            const auto nav_mesh = GWIPC::GetNavMesh(cached_nav_mesh_it->second.data());
            if (nav_mesh)
            {
                build_mesh(nav_mesh, nav_meshes[map_id]);
            }
        }
    }

    return nav_meshes[map_id];
}

void NavMeshManager::try_cache_nav_mesh(const std::string& nav_mesh_filepath)
{
    HANDLE mutex = CreateMutex(NULL, false, L"NavMeshMutex");
    if (mutex != NULL)
    {

        DWORD wait_result = WaitForSingleObject(mutex, 0);
        if (wait_result == WAIT_OBJECT_0)
        {
            std::ifstream navmesh_ifstream(nav_mesh_filepath, std::ios_base::binary);
            if (navmesh_ifstream.is_open())
            {
                const auto file_size = std::filesystem::file_size(nav_mesh_filepath);
                const auto map_id = get_first_integer_in_string(nav_mesh_filepath);
                auto insert_result = cached_nav_mesh_data.insert({map_id, std::vector<uint8_t>(file_size)});

                navmesh_ifstream.read(reinterpret_cast<char*>(insert_result.first->second.data()), file_size);
                navmesh_ifstream.close();
            }
        }
    }
}

void NavMeshManager::build_mesh(const GWIPC::NavMesh* const gwipc_nav_mesh, NavMesh& nav_mesh_out)
{
    nav_mesh_out.vertices.clear();
    nav_mesh_out.faces.clear();

    auto nav_mesh_dim = gwipc_nav_mesh->dimensions();
    auto min_x = nav_mesh_dim->min_x();
    auto max_x = nav_mesh_dim->max_x();
    auto min_y = nav_mesh_dim->min_y();
    auto max_y = nav_mesh_dim->max_y();
    auto min_z = nav_mesh_dim->min_z();
    auto max_z = nav_mesh_dim->max_z();
    auto x_diff = max_x - min_x;
    auto z_diff = max_z - min_z;

    auto map_bounds = MapBounds();
    map_bounds.x_max = max_x;
    map_bounds.x_min = min_x;
    map_bounds.y_max = max_y;
    map_bounds.y_min = min_y;
    map_bounds.z_max = max_z;
    map_bounds.z_min = min_z;
    map_bounds.center_point = GWIPC::Vec3((max_x + min_x) / 2, (max_y + min_y) / 2, (max_z + min_z) / 2);

    nav_mesh_out.map_bounds = map_bounds;

    const auto gwipc_trapzoids = gwipc_nav_mesh->trapezoids();
    for (int i = 0; i < 100; i++)
    {
        nav_mesh_out.null1s.insert({i, 0});
    }

    if (gwipc_trapzoids)
    {
        // Vertex -> global_vertex_id.
        std::map<const GWIPC::Vec3*, uint32_t, VertexCompare> already_added_vertices;
        uint32_t current_global_vertex_id = 0;

        for (int i = 0; i < gwipc_trapzoids->size(); i++)
        {
            const auto trapezoid = gwipc_trapzoids->Get(i);
            if (trapezoid)
            {
                uint32_t BL_global_index;
                uint32_t BR_global_index;
                uint32_t TL_global_index;
                uint32_t TR_global_index;

                int adj_Count = 0;
                if (trapezoid->adjacent_trapezoid_ids()->down_id() >= 0)
                    adj_Count++;
                if (trapezoid->adjacent_trapezoid_ids()->left_id() >= 0)
                    adj_Count++;
                if (trapezoid->adjacent_trapezoid_ids()->right_id() >= 0)
                    adj_Count++;
                if (trapezoid->adjacent_trapezoid_ids()->up_id() >= 0)
                    adj_Count++;

                if (adj_Count == 1)
                {
                    nav_mesh_out.null1s[trapezoid->z_plane()]++;
                }

                auto bl = trapezoid->bottom_left();
                auto br = trapezoid->bottom_right();
                auto tl = trapezoid->top_left();
                auto tr = trapezoid->top_right();
                assert(bl && br && tl && tr, "missing nav_mesh vertex.");

                // BL
                auto it = already_added_vertices.find(bl);
                if (it != already_added_vertices.end())
                {
                    BL_global_index = it->second;
                }
                else
                {
                    already_added_vertices.insert({bl, current_global_vertex_id});
                    BL_global_index = current_global_vertex_id++;
                    nav_mesh_out.vertices.push_back(*bl);
                    nav_mesh_out.layers.push_back(trapezoid->z_plane());
                }

                // BR
                it = already_added_vertices.find(br);
                if (it != already_added_vertices.end())
                {
                    BR_global_index = it->second;
                }
                else
                {
                    already_added_vertices.insert({br, current_global_vertex_id});
                    BR_global_index = current_global_vertex_id++;
                    nav_mesh_out.vertices.push_back(*br);
                    nav_mesh_out.layers.push_back(trapezoid->z_plane());
                }

                // TL
                it = already_added_vertices.find(tl);
                if (it != already_added_vertices.end())
                {
                    TL_global_index = it->second;
                }
                else
                {
                    already_added_vertices.insert({tl, current_global_vertex_id});
                    TL_global_index = current_global_vertex_id++;
                    nav_mesh_out.vertices.push_back(*tl);
                    nav_mesh_out.layers.push_back(trapezoid->z_plane());
                }

                // TR
                it = already_added_vertices.find(tr);
                if (it != already_added_vertices.end())
                {
                    TR_global_index = it->second;
                }
                else
                {
                    already_added_vertices.insert({tr, current_global_vertex_id});
                    TR_global_index = current_global_vertex_id++;
                    nav_mesh_out.vertices.push_back(*tr);
                    nav_mesh_out.layers.push_back(trapezoid->z_plane());
                }

                const auto face_1 = TriangleFace{BL_global_index, TL_global_index, BR_global_index};
                const auto face_2 = TriangleFace{TL_global_index, TR_global_index, BR_global_index};
                nav_mesh_out.faces.push_back(face_1);
                nav_mesh_out.faces.push_back(face_2);
            }
        }
    }
}
