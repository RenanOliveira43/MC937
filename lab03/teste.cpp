#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;
};

struct Triangle {
    glm::vec3 v0, v1, v2;
    glm::vec3 normal;
};

// glm::vec3 lightPosition = glm::vec3(10.0f, 15.0f, 10.0f);
glm::vec3 lightPosition = glm::vec3(0.0f, 10.0f, 0.0f);
glm::vec3 ambientColor = glm::vec3(0.1f, 0.1f, 0.3f);  
glm::vec3 diffuseColor = glm::vec3(0.3f, 0.8f, 0.3f);  
glm::vec3 specularColor = glm::vec3(1.0f, 0.3f, 0.3f);  

float shininess = 51.2f;  



std::vector<glm::vec3> vertices;  
std::vector<glm::ivec3> faceIndices;


bool loadOBJ(const std::string& path) {
    FILE* file = std::fopen(path.c_str(), "r");
    if (!file) {
        std::cerr << "Failed to open OBJ file: " << path << std::endl;
        return false;
    }

    char line[128];
    while (std::fgets(line, sizeof(line), file)) {
        if (line[0] == 'v' && line[1] == ' ') {
            glm::vec3 vertex;
            std::sscanf(line, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
            vertices.push_back(vertex);
        } else if (line[0] == 'f') {
            unsigned int vIndex[3];
            std::sscanf(line, "f %d %d %d", &vIndex[0], &vIndex[1], &vIndex[2]);
            faceIndices.emplace_back(vIndex[0], vIndex[1], vIndex[2]);
        }
    }

    std::fclose(file);
    return true;
}

bool intersectRayTriangle(const Ray &ray, const Triangle &tri, float &tOut) {
    glm::vec3 e1 = tri.v1 - tri.v0;
    glm::vec3 e2 = tri.v2 - tri.v0;

    glm::vec3 p = glm::cross(ray.direction, e2);
    float det = glm::dot(e1, p);
    
    if (fabs(det) < 1e-6f) {
        return false;
    }

    glm::vec3 T = ray.origin - tri.v0;
    float u = glm::dot(T, p) / det;
    
    if (u < 0.0f || u > 1.0f) {
        return false;
    }

    glm::vec3 q = glm::cross(T, e1);
    float v = glm::dot(ray.direction, q) / det;
    
    if (v < 0.0f || u + v > 1.0f) {
        return false;
    }

    float t = glm::dot(e2, q) / det;

    if (t < 1e-6f) {
        return false;
    }

    tOut = t;

    return true;
}

// Cálculo da iluminação ADS (Gouraud)
glm::vec3 computeADS(glm::vec3 pos, glm::vec3 normal) {
    glm::vec3 L = glm::normalize(lightPosition - pos); 
    glm::vec3 V = glm::normalize(-pos);
    glm::vec3 R = glm::reflect(-L, normal);
    
    glm::vec3 ambient = ambientColor;
    glm::vec3 diffuse = diffuseColor * glm::max(glm::dot(normal, L), 0.0f);
    glm::vec3 specular = specularColor * std::pow(glm::max(glm::dot(R, V), 0.0f), shininess);
    
    return ambient + diffuse + specular;
}

bool isInShadow(const glm::vec3& point, const glm::vec3& normal, const std::vector<Triangle>& triangles) {
    glm::vec3 lightDir = glm::normalize(lightPosition - point);
    Ray shadowRay;
    shadowRay.origin = point + normal * 1e-4f;  
    shadowRay.direction = lightDir;

    float distToLight = glm::length(lightPosition - point);

    for (const auto& tri : triangles) {
        float t;
        if (intersectRayTriangle(shadowRay, tri, t)) {
            if (t < distToLight) {
                return true;  
            }
        }
    }
    return false;  
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <arquivo.obj>" << std::endl;
        return 1;
    }
    if (!loadOBJ(argv[1])) {
        std::cerr << "Erro ao carregar o arquivo OBJ." << std::endl;
        return 1;
    }


    std::ofstream objOut("resultado.obj");
    std::ofstream mtlOut("resultado.mtl");
    objOut << "mtllib resultado.mtl\n";

    for (const auto& v : vertices) {
        objOut << "v " << v.x << " " << v.y << " " << v.z << "\n";
    }

    std::vector<Triangle> triangles;
    for (auto& f : faceIndices) {
        Triangle tri;
        tri.v0 = vertices[f.x - 1];  
        tri.v1 = vertices[f.y - 1];
        tri.v2 = vertices[f.z - 1];
        tri.normal = glm::normalize(glm::cross(tri.v1 - tri.v0, tri.v2 - tri.v0));
        triangles.push_back(tri);
    }

    for (int triIndex = 0; triIndex < triangles.size(); ++triIndex) {
        const Triangle& tri = triangles[triIndex];

        glm::vec3 c0 = isInShadow(tri.v0, tri.normal, triangles) ? ambientColor : computeADS(tri.v0, tri.normal);
        glm::vec3 c1 = isInShadow(tri.v1, tri.normal, triangles) ? ambientColor : computeADS(tri.v1, tri.normal);
        glm::vec3 c2 = isInShadow(tri.v2, tri.normal, triangles) ? ambientColor : computeADS(tri.v2, tri.normal);

        glm::vec3 finalColor = (c0 + c1 + c2) / 3.0f;
        finalColor = glm::clamp(finalColor, glm::vec3(0.0f), glm::vec3(1.0f));

        // Exporta o material
        char matName[64];
        sprintf(matName, "mat%d", triIndex);
        mtlOut << "newmtl " << matName << "\n";
        mtlOut << "Kd " << finalColor.r << " " << finalColor.g << " " << finalColor.b << "\n\n";

        int i0 = faceIndices[triIndex].x;
        int i1 = faceIndices[triIndex].y;
        int i2 = faceIndices[triIndex].z;
        objOut << "usemtl " << matName << "\n";
        objOut << "f " << i0 << " " << i1 << " " << i2 << "\n";
    }
        
    objOut.close();
    mtlOut.close();
    std::cout << "Exportado para resultado.obj e resultado.mtl com sucesso!\n";
    return 0;
}
