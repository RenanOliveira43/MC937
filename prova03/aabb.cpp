#include <vector>
#include <array>
#include <memory>
#include <algorithm>
#include <limits>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct AABB {
    glm::vec3 min_corner;
    glm::vec3 max_corner;
    
    AABB() : min_corner(std::numeric_limits<float>::max()), max_corner(std::numeric_limits<float>::lowest()) {}
             
    AABB(const glm::vec3& min, const glm::vec3& max) : 
        min_corner(min), max_corner(max) {}
    
    bool isEmpty() const {
        return min_corner.x > max_corner.x || min_corner.y > max_corner.y || min_corner.z > max_corner.z;
    }
    
    AABB transform(const glm::mat4& matrix) const {
        if (isEmpty()) return *this;
        
        glm::vec3 corners[8] = {
            {min_corner.x, min_corner.y, min_corner.z},
            {min_corner.x, min_corner.y, max_corner.z},
            {min_corner.x, max_corner.y, min_corner.z},
            {min_corner.x, max_corner.y, max_corner.z},
            {max_corner.x, min_corner.y, min_corner.z},
            {max_corner.x, min_corner.y, max_corner.z},
            {max_corner.x, max_corner.y, min_corner.z},
            {max_corner.x, max_corner.y, max_corner.z}
        };

        glm::vec3 new_min(std::numeric_limits<float>::max());
        glm::vec3 new_max(std::numeric_limits<float>::lowest());

        for (const auto& corner : corners) {
            glm::vec3 transformed = glm::vec3(matrix * glm::vec4(corner, 1.0f));
            new_min = glm::min(new_min, transformed);
            new_max = glm::max(new_max, transformed);
        }

        return AABB(new_min, new_max);
    }
    
    bool intersects(const AABB& other) const {
        return (min_corner.x <= other.max_corner.x && max_corner.x >= other.min_corner.x) &&
               (min_corner.y <= other.max_corner.y && max_corner.y >= other.min_corner.y) &&
               (min_corner.z <= other.max_corner.z && max_corner.z >= other.min_corner.z);
    }
    
    unsigned short getLargestAxis() const {
        glm::vec3 size = max_corner - min_corner;

        if (size.x >= size.y && size.x >= size.z) {
            return 0;
        }
        else if (size.y >= size.z) {
            return 1;
        }
        else {
            return 2;
        }
    }
};

struct Mesh {
    using coordinate_t = std::vector<glm::vec3>;
    using triangles_t = std::vector<std::array<unsigned, 3>>;
    
    AABB aabb;
    std::shared_ptr<const coordinate_t> coordinates;
    triangles_t triangles;
    
    Mesh(std::shared_ptr<const coordinate_t> coords, const triangles_t& tris) : 
        coordinates(std::move(coords)), triangles(tris) {
        updateAABB();
    }
    
    void updateAABB() {
        if (triangles.empty()) {
            aabb = AABB();
            return;
        }

        glm::vec3 min(std::numeric_limits<float>::max());
        glm::vec3 max(std::numeric_limits<float>::lowest());

        for (const auto& tri : triangles) {
            for (auto idx : tri) {
                const glm::vec3& v = (*coordinates)[idx];
                min = glm::min(min, v);
                max = glm::max(max, v);
            }
        }

        aabb = AABB(min, max);
    }
    
    void sortTrianglesByAxis(unsigned axis) {
        auto comparator = [&](const std::array<unsigned, 3>& a, const std::array<unsigned, 3>& b) {
            glm::vec3 centroid_a = ((*coordinates)[a[0]] + (*coordinates)[a[1]] + (*coordinates)[a[2]]) / 3.0f;
            glm::vec3 centroid_b = ((*coordinates)[b[0]] + (*coordinates)[b[1]] + (*coordinates)[b[2]]) / 3.0f;
            return centroid_a[axis] < centroid_b[axis];
        };
        std::sort(triangles.begin(), triangles.end(), comparator);
    }
    
    std::pair<Mesh, Mesh> splitMesh() const {
        if (triangles.empty()) 
            return {Mesh(coordinates, {}), Mesh(coordinates, {})};

        unsigned axis = aabb.getLargestAxis();
        std::vector<float> centroids;
        centroids.reserve(triangles.size());
        
        for (const auto& tri : triangles) {
            glm::vec3 centroid = ((*coordinates)[tri[0]] + (*coordinates)[tri[1]] + (*coordinates)[tri[2]]) / 3.0f;
            centroids.push_back(centroid[axis]);
        }

        size_t mid = centroids.size() / 2;
        std::nth_element(centroids.begin(), centroids.begin() + mid, centroids.end());
        float median = centroids[mid];

        triangles_t first, second;
        for (const auto& tri : triangles) {
            glm::vec3 centroid = ((*coordinates)[tri[0]] + (*coordinates)[tri[1]] + (*coordinates)[tri[2]]) / 3.0f;
            (centroid[axis] <= median ? first : second).push_back(tri);
        }

        if (first.empty() || second.empty()) {
            size_t half = triangles.size() / 2;
            first.assign(triangles.begin(), triangles.begin() + half);
            second.assign(triangles.begin() + half, triangles.end());
        }

        return {Mesh(coordinates, first), Mesh(coordinates, second)};
    }
};

struct AABBNode {
    Mesh mesh;
    AABB original_aabb;
    AABB transformed_aabb;
    bool leaf{false};
    std::unique_ptr<AABBNode> left_child;
    std::unique_ptr<AABBNode> right_child;
    
    AABBNode(const Mesh& m) : mesh(m), original_aabb(m.aabb), transformed_aabb(m.aabb) {}
    
    bool isLeaf() const { return leaf; }
    void makeLeaf() { leaf = true; }
};

class AABBTree {
    std::unique_ptr<AABBNode> root;
    glm::mat4 last_transform{1.0f};
    bool force_update{true};
    unsigned max_depth{16};
    unsigned min_triangles{4};
    
public:
    AABBTree(const Mesh& mesh) {
        root = std::make_unique<AABBNode>(mesh);
    }
    
    void build() {
        build(root, 0);
    }
    
    void updateTransform(const glm::mat4& transform) {
        if (transform == last_transform && !force_update) return;
        last_transform = transform;
        force_update = false;
        updateNodeTransform(root.get(), transform);
    }
    
    void forceUpdate() { 
        force_update = true; 
    }

    AABBNode* getRoot() const { return root.get(); }

        
private:
    void build(std::unique_ptr<AABBNode>& node, unsigned depth) {
        if (!node || node->isLeaf()) return;

        if (depth > max_depth || node->mesh.triangles.size() <= min_triangles) {
            node->makeLeaf();
            return;
        }

        auto sub_meshes = node->mesh.splitMesh();
        node->left_child = std::make_unique<AABBNode>(sub_meshes.first);
        node->right_child = std::make_unique<AABBNode>(sub_meshes.second);

        build(node->left_child, depth + 1);
        build(node->right_child, depth + 1);
    }
    
    void updateNodeTransform(AABBNode* node, const glm::mat4& transform) {
        if (!node) return;
        
        node->transformed_aabb = node->original_aabb.transform(transform);
        
        if (!node->isLeaf()) {
            updateNodeTransform(node->left_child.get(), transform);
            updateNodeTransform(node->right_child.get(), transform);
        }
    }
};