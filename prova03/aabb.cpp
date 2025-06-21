#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <memory>
#include <glm/glm.hpp>

// Axis-Aligned Bounding Box (AABB) structure
struct AABB
{
    glm::vec3 min_corner{}; // Minimum corner of the bounding box
    glm::vec3 max_corner{}; // Maximum corner of the bounding box

    AABB() {}

    // Constructor that initializes the bounding box with given min and max
    AABB(const glm::vec3& min, const glm::vec3& max):
        min_corner{min},
        max_corner{max}
    {}

    // Returns the axis (0=x, 1=y, 2=z) where the box is largest
    unsigned short getLargestAxis() const
    {
        glm::vec3 size = max_corner - min_corner;

        if (size.x >= size.y && size.x >= size.z)
            return 0;
        else if (size.y >= size.z)
            return 1;
        else
            return 2;
    }

    // Splits the AABB into two along a specified axis
    std::pair<AABB, AABB> split(unsigned axis) const
    {
        float mid_point = (min_corner[axis] + max_corner[axis]) * 0.5f;

        glm::vec3 first_max{max_corner};
        first_max[axis] = mid_point;

        glm::vec3 second_min{min_corner};
        second_min[axis] = mid_point;

        return {
            AABB(min_corner, first_max),
            AABB(second_min, max_corner)
        };
    }
};

// Structure representing a 3D mesh
struct Mesh
{
    using coordinate_t = std::vector<glm::vec3>; // List of vertex positions
    using triangles_t = std::vector<std::array<unsigned, 3>>; // List of triangle indices

    AABB aabb; // Bounding box for the mesh
    std::shared_ptr<const coordinate_t> coordinates; // Shared pointer to vertex positions
    triangles_t triangles; // Triangles made of 3 vertex indices

    // Constructor that takes coordinates (shared_ptr) and triangle data
    Mesh(std::shared_ptr<const coordinate_t> coords,
         const triangles_t& triangles):
        coordinates{std::move(coords)},
        triangles{triangles}
    {
        updateAABB(); // Automatically compute the AABB
    }

    void updateAABB(); // Updates the mesh's bounding box

    // Sorts triangles based on their centroid's position along a given axis
    void sortTrianglesByAxis(unsigned axis)
    {
        auto comparator = [&](const std::array<unsigned, 3>& a, const std::array<unsigned, 3>& b)
        {
            glm::vec3 centroid_a = (coordinates->at(a[0]) + coordinates->at(a[1]) + coordinates->at(a[2])) / 3.0f;
            glm::vec3 centroid_b = (coordinates->at(b[0]) + coordinates->at(b[1]) + coordinates->at(b[2])) / 3.0f;
            return centroid_a[axis] < centroid_b[axis];
        };

        std::sort(triangles.begin(), triangles.end(), comparator);
    }

    // Splits the mesh's triangle list into two halves based on spatial median of centroids
    std::pair<Mesh, Mesh> splitMesh();
};

// Computes the AABB that encloses all the mesh's vertices used by its triangles
void Mesh::updateAABB()
{
    if(triangles.empty()) return;

    glm::vec3 min = coordinates->at(triangles[0][0]);
    glm::vec3 max = min;

    for (const auto& tri : triangles)
    {
        for (auto idx : tri)
        {
            const glm::vec3& coord = coordinates->at(idx);
            min = glm::min(min, coord);
            max = glm::max(max, coord);
        }
    }

    aabb = {min, max};
}

// Splits the mesh into two submeshes based on the spatial median of triangle centroids along the largest axis
std::pair<Mesh, Mesh> Mesh::splitMesh()
{
    if(triangles.empty()) return {{coordinates, {}}, {coordinates, {}}};

    unsigned axis = aabb.getLargestAxis();

    // Compute centroid positions for all triangles
    std::vector<float> centroids;
    centroids.reserve(triangles.size());
    for(const auto& tri : triangles)
    {
        glm::vec3 centroid = (coordinates->at(tri[0]) + coordinates->at(tri[1]) + coordinates->at(tri[2])) / 3.0f;
        centroids.push_back(centroid[axis]);
    }

    // Compute median value (approximate)
    std::vector<float> sorted_centroids = centroids;
    size_t mid_index = sorted_centroids.size() / 2;
    std::nth_element(sorted_centroids.begin(), sorted_centroids.begin() + mid_index, sorted_centroids.end());
    float median = sorted_centroids[mid_index];

    // Split triangles into two groups based on median
    triangles_t first_t, second_t;
    for(size_t i=0; i<triangles.size(); ++i)
    {
        glm::vec3 centroid = (coordinates->at(triangles[i][0]) + coordinates->at(triangles[i][1]) + coordinates->at(triangles[i][2])) / 3.0f;
        if(centroid[axis] <= median)
            first_t.push_back(triangles[i]);
        else
            second_t.push_back(triangles[i]);
    }

    // Handle case where one side is empty (split by count instead)
    if(first_t.empty() || second_t.empty())
    {
        size_t half = triangles.size() / 2;
        first_t.assign(triangles.begin(), triangles.begin() + half);
        second_t.assign(triangles.begin() + half, triangles.end());
    }

    return {{coordinates, first_t}, {coordinates, second_t}};
}

// Node in the AABB tree
struct AABBNode
{
    Mesh mesh;
    bool leaf{false}; // True if this node is a leaf (only one triangle)

    std::unique_ptr<AABBNode> left_child{nullptr};
    std::unique_ptr<AABBNode> right_child{nullptr};

    AABBNode(const Mesh& mesh):mesh{mesh}{}

    bool isLeaf() const { return leaf; }
    void makeLeaf() { leaf = true; }
};

// AABB Tree structure
struct AABBTree
{
    std::unique_ptr<AABBNode> root{nullptr}; // Root node of the tree

    AABBTree(const Mesh& mesh) {
        root = std::make_unique<AABBNode>(mesh);
    }

    void build() { build(root, 0); std::cout << "[ OK ] Build AABB tree\n"; }

    void print(const std::unique_ptr<AABBNode>& node) const;

private:
    void build(std::unique_ptr<AABBNode>& node, unsigned depth);
};

// Recursively builds the AABB tree with max depth 16
void AABBTree::build(std::unique_ptr<AABBNode>& node, unsigned depth)
{
    if(node == nullptr) return;
    if(node->isLeaf()) return;

    if(depth > 16 || node->mesh.triangles.size() <= 1)
    {
        node->makeLeaf();
        return;
    }

    unsigned short largest_axis = node->mesh.aabb.getLargestAxis();

    node->mesh.sortTrianglesByAxis(largest_axis);
    auto sub_meshes = node->mesh.splitMesh();

    node->left_child = std::make_unique<AABBNode>(sub_meshes.first);
    node->right_child = std::make_unique<AABBNode>(sub_meshes.second);

    build(node->left_child, depth + 1);
    build(node->right_child, depth + 1);
}

