#pragma once

struct VertexCompare
{
    bool operator()(const GWIPC::Vec3* lhs, const GWIPC::Vec3* rhs) const
    {
        if (lhs->x() != rhs->x())
        {
            return lhs->x() < rhs->x();
        }
        if (lhs->y() != rhs->y())
        {
            return lhs->y() < rhs->y();
        }
        return lhs->z() < rhs->z();
    }

    bool operator()(const GWIPC::Vec3 lhs, const GWIPC::Vec3 rhs) const
    {
        if (lhs.x() != rhs.x())
        {
            return lhs.x() < rhs.x();
        }
        if (lhs.y() != rhs.y())
        {
            return lhs.y() < rhs.y();
        }
        return lhs.z() < rhs.z();
    }
};

struct TriangleFace
{
    uint32_t vertex_0_index;
    uint32_t vertex_1_index;
    uint32_t vertex_2_index;
};

struct MapBounds
{
    float x_min;
    float x_max;
    float y_min;
    float y_max;
    float z_min;
    float z_max;
    float z_avg;

    GWIPC::Vec3 center_point;
};

struct NavMesh
{
    std::vector<uint32_t> layers;
    std::map<uint32_t, uint32_t> null1s;
    std::vector<GWIPC::Vec3> vertices;
    std::vector<TriangleFace> faces;

    const int* get_indices() { return (int*)faces.data(); }
    const float* get_vertices() { return (float*)vertices.data(); }

    MapBounds map_bounds;

private:
    std::vector<float> verts;
};

class NavMeshManager
{
public:
    const NavMesh& get_navmesh(const uint32_t map_id, const std::string& file_path);

private:
    std::map<uint32_t, std::vector<uint8_t>> cached_nav_mesh_data;

    // The highest map_id is < 900. We waste some space for quick lookup.
    NavMesh nav_meshes[900]{};
    void try_cache_nav_mesh(const std::string& nav_mesh_filepath);

    void build_mesh(const GWIPC::NavMesh* const gwipc_nav_mesh, NavMesh& nav_mesh_out);
};
