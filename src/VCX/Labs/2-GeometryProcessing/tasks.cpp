#include <iostream>
#include <list>
#include <map>
#include <set>
#include <unordered_set>
#include <cmath>

#include <glm/gtc/matrix_inverse.hpp>
#include <spdlog/spdlog.h>

#include "Labs/2-GeometryProcessing/DCEL.hpp"
#include "Labs/2-GeometryProcessing/tasks.h"

namespace VCX::Labs::GeometryProcessing {

#include "Labs/2-GeometryProcessing/marching_cubes_table.h"

    /*
     * Helper function for task 1
     * Converts a DCEL::HalfEdge to an unordered pair of vertex ids
     */
    std::pair<uint32_t, uint32_t> edge2pair(DCEL::HalfEdge const *e) {
        return e->To() < e->From() ? std::make_pair(e->To(), e->From())
                                   : std::make_pair(e->From(), e->To());
    }

    /******************* 1. Mesh Subdivision *****************/
    void SubdivisionMesh(Engine::SurfaceMesh const & input, Engine::SurfaceMesh & output, std::uint32_t numIterations) {
        Engine::SurfaceMesh mesh = input; // temporary mesh
        /* Subdivide mesh for 'numIterations' times */
        while(numIterations--) {
            /* Build DCEL */
            DCEL links;
            links.AddFaces(mesh.Indices);
            if (!links.IsValid()){
                std::cerr << "Invalid mesh" << std::endl;
                return;
            }

            output.Positions.clear();
            output.Indices.clear();

            /* Calculate new positions of existing vertices */
            for (std::size_t i = 0; i < mesh.Positions.size(); ++i) {
                DCEL::Vertex v = links.GetVertex(i);
                auto neighbors = v.GetNeighbors();
                int deg = neighbors.size();
                float u = 3. / (deg == 3 ? 16 : 8 * deg);
                glm::vec3 p = mesh.Positions[i] * (1 - deg * u);
                for (auto n : neighbors)
                    p += mesh.Positions[n] * u;
                output.Positions.push_back(p);
            }

            /* 'edgeId' maps an edge to vertex id in new mesh */
            std::map<std::pair<uint32_t, uint32_t>, int> edgeId;
            int num = mesh.Positions.size();

            /* Calculate positions of vertices derived from edges*/
            for (DCEL::HalfEdge const * e : links.GetEdges()){
                glm::vec3 p = mesh.Positions[e->To()] + mesh.Positions[e->From()];
                p = p * 3.0f;
                p += mesh.Positions[e->OppositeVertex()];
                if (e->PairEdge() != nullptr)
                    p += mesh.Positions[e->PairEdge()->OppositeVertex()];
                else {
                    std::cerr << "No pair edge" << std::endl;
                    return;
                }
                output.Positions.push_back(p / 8.0f);
                edgeId[edge2pair(e)] = num++;
            }

            /* Divide triangles */
            for (DCEL::Triangle const &t : links.GetFaces()) {
                for (int i = 0; i < 3; ++i){
                    output.Indices.push_back(*t.Indices(i));
                    output.Indices.push_back(edgeId[edge2pair(t.Edges((i+2)%3))]);
                    output.Indices.push_back(edgeId[edge2pair(t.Edges((i+1)%3))]);
                }
                output.Indices.push_back(edgeId[edge2pair(t.Edges(0))]);
                output.Indices.push_back(edgeId[edge2pair(t.Edges(1))]);
                output.Indices.push_back(edgeId[edge2pair(t.Edges(2))]);
            }

            mesh = output;
        }
        output = mesh;
    }

    /******************* 2. Mesh Parameterization *****************/
    void Parameterization(Engine::SurfaceMesh const & input, Engine::SurfaceMesh & output, const std::uint32_t numIterations) {
        /* Build DCEL */
        DCEL links;
        links.AddFaces(input.Indices);
        if (!links.IsValid()){
            std::cerr << "Invalid mesh" << std::endl;
            return;
        }

        /* Find a vertex 'x' on the boundary */
        output = input;
        int n = input.Positions.size();
        int x = n;
        for (int i = 0; i < n; ++i) if (links.GetVertex(i).IsSide()){
            x = i;
            break;
        }
        if (x == n){
            std::cerr << "No boundary" << std::endl;
            return;
        }

        /* Find all vertices on the boundary in order */
        std::vector<int> boundary;
        boundary.push_back(x); // start from 'x'
        boundary.push_back(links.GetVertex(x).GetSideNeighbors().first); // next vertex
        while (true){
            x = boundary.back();
            auto [n1, n2] = links.GetVertex(x).GetSideNeighbors();
            x = (n1 == boundary[boundary.size() - 2] ? n2 : n1); // next vertex in this direction
            if (x == boundary[0])
                break;
            boundary.push_back(x);
        }

        /* Put all boundary vertices around a circle */
        auto &uv = output.TexCoords;
        uv = std::vector<glm::vec2>(n, glm::vec2(0));
        double pi = acos(-1);
        for (int i = 0; i < boundary.size(); ++i){
            uv[boundary[i]] = glm::vec2(
                cos(2 * pi * i / boundary.size()) / 2 + 0.5,
                sin(2 * pi * i / boundary.size()) / 2 + 0.5
            );
        }
        
        /* Calculate interior vertices using Jacobi method */
        for (int i = 0; i < numIterations; ++i){
            for (int j = 0; j < n; ++j){
                if (links.GetVertex(j).IsSide())
                    continue;
                auto neighbors = links.GetVertex(j).GetNeighbors();
                glm::vec2 p = glm::vec2(0);
                for (auto n : neighbors)
                    p += uv[n];
                if (neighbors.size())
                    uv[j] = p / (float)neighbors.size();
            }
        }
    }



    /*
     * Helper function for task 3
     * Calculate the cost and the result position of contracting 'p1' and 'p2'
     */
    std::pair<float, glm::vec3> CalculateCost(glm::dmat4x4 Q, glm::vec3 p1, glm::vec3 p2){
        auto P = Q;
        P[3][0] = P[3][1] = P[3][2] = 0;
        P[3][3] = 1;
        if (glm::abs(glm::determinant(P)) < 1e-15){
            /* If 'P' is not invertible, find the optimal vertex along the segment 'p1''p2' */
            glm::dvec4 w1 = glm::dvec4(p1, 1);
            glm::dvec4 w2 = glm::dvec4(p2, 1);
            /* Minimize (x*w1+(1-x)*w2)^T * Q * (x*w1+(1-x)*w2) */
            double co2 = glm::dot((w1 - w2), Q * (w1 - w2)); // coefficient of x^2
            double co1 = glm::dot(w1, Q * w2) * 2;           // coefficient of x
            if (abs(co2) < 1e-15) { // case of linear function
                if (co1 > 0)
                    return std::make_pair(glm::dot(w2, Q * w2), p2);
                else
                    return std::make_pair(glm::dot(w1, Q * w1), p1);
            }
            else { // case of quadratic function
                double t = -co1 / (2 * co2);
                if (t < 0)
                    return std::make_pair(glm::dot(w2, Q * w2), p2);
                else if (t > 1)
                    return std::make_pair(glm::dot(w1, Q * w1), p1);
                else
                    return std::make_pair(
                        glm::dot(w1 * t + w2 * (1- t), Q * (w1 * t + w2 * (1- t))),
                        p1 * (float)t + p2 * (1 - (float)t)
                    );
            }
        }
        else{
            /* If 'P' is invertible */
            P = glm::inverse(P);
            glm::dvec4 p = glm::dvec4(0, 0, 0, 1) * P;
            float cost = glm::dot(p, Q * p);
            return std::make_pair(cost, glm::vec3(p));
        }
    }

    /******************* 3. Mesh Simplification *****************/
    void SimplifyMesh(Engine::SurfaceMesh const & input, Engine::SurfaceMesh & output, float valid_pair_threshold, float simplification_ratio) {
        /* Build DCEL */
        DCEL links;
        links.AddFaces(input.Indices);
        if (!links.IsValid()){
            std::cerr << "Invalid mesh" << std::endl;
            return;
        }

        /* Calculate Q for initial vertices */
        int n = input.Positions.size();
        auto pos = input.Positions;
        std::vector<glm::dmat4x4> Q(n, glm::dmat4x4(0));
        for (int i = 0; i < n; ++i){
            for (auto f: links.GetVertex(i).GetFaces()){
                glm::vec3 normal = glm::cross(pos[*f->Indices(1)] - pos[*f->Indices(0)], pos[*f->Indices(2)] - pos[*f->Indices(0)]);
                normal = glm::normalize(normal);
                glm::vec4 p = glm::vec4(normal, -glm::dot(normal, pos[i]));
                Q[i] += glm::outerProduct(p, p);
            }
        }

        /* Find all valid pairs */
        std::vector<std::pair<float, std::pair<int, int>>> pairs;
        for (int i = 0; i < n; ++i) {
            auto neighbors = links.GetVertex(i).GetNeighbors();
            std::sort(neighbors.begin(), neighbors.end());
            int k = 0;
            for (int j = 0; j < i; ++j) {
                while (k < neighbors.size() && neighbors[k] < j)
                    ++k;
                if (k < neighbors.size() && neighbors[k] == j || glm::length(pos[i] - pos[j]) < valid_pair_threshold){
                    auto [cost, p] = CalculateCost(Q[i]+Q[j], pos[i], pos[j]);
                    pairs.push_back(std::make_pair(cost, std::make_pair(i, j)));
                }
            }
        }

        /* Build DSU (Disjoint Set Union) */
        std::vector<int> fa(n);
        for (int i = 0; i < n; ++i) fa[i] = i;
        std::function<int(int)> find = [&](int x){
            if (fa[x] == x) return x;
            return fa[x] = find(fa[x]);
        };

        /* Delete pairs until 'simplification_ratio' reached */
        int deleteNum = ceil(n * (1 - simplification_ratio));
        while (deleteNum--){
            if (pairs.empty()){
                std::cerr << "No valid pair" << std::endl;
                break;
            }
            /* Find the optimal pair 'x' and 'y' */
            auto pair = std::min_element(pairs.begin(), pairs.end())->second;
            int x = pair.first, y = pair.second;
            /* Contract 'x' and 'y' */
            Q[x] += Q[y];
            fa[y] = x;
            pos[x] = CalculateCost(Q[x], pos[x], pos[y]).second;
            /* Update other pairs */
            std::vector<std::pair<int, int>> newPairs;
            for (auto [_, pair]: pairs){
                int u = find(pair.first), v = find(pair.second);
                if (u == v) continue;
                newPairs.push_back(std::make_pair(u, v));
            }
            std::sort(newPairs.begin(), newPairs.end());
            newPairs.erase(std::unique(newPairs.begin(), newPairs.end()), newPairs.end());
            pairs.clear();
            for (auto [u, v]: newPairs){
                auto [cost, p] = CalculateCost(Q[u]+Q[v], pos[u], pos[v]);
                pairs.push_back(std::make_pair(cost, std::make_pair(u, v)));
            }
        }

        /* Rebuild mesh after simplification */
        std::vector<int> id(n), cnt(n);
        output.Positions.clear();
        for (int i = 0; i < n; ++i) if (find(i) == i) {
            id[i] = output.Positions.size();
            output.Positions.push_back(pos[i]);
        }
        for (auto const &f: links.GetFaces()){
            uint32_t x = id[find(*f.Indices(0))], y = id[find(*f.Indices(1))], z = id[find(*f.Indices(2))];
            if (x != y && x != z && y != z){
                output.Indices.push_back(x);
                output.Indices.push_back(y);
                output.Indices.push_back(z);
            }
        }
    }

    /******************* 4. Mesh Smoothing *****************/
    void SmoothMesh(Engine::SurfaceMesh const & input, Engine::SurfaceMesh & output, std::uint32_t numIterations, float lambda, bool useUniformWeight) {
        /* Build DCEL */
        DCEL links;
        links.AddFaces(input.Indices);
        if (!links.IsValid()){
            std::cerr << "Invalid mesh" << std::endl;
            return;
        }

        output = input;
        while (numIterations--){
            /* Calculate new positions */
            auto &pos = output.Positions;
            std::vector<glm::vec3> newPos(pos.size());
            /* Iterate over all vertices */
            for (int i = 0; i < pos.size(); ++i){
                glm::vec3 sum(0);
                float totalWeight = 0;
                /* Iterate over relating triangles */
                for (auto f: links.GetVertex(i).GetFaces()){
                    int id = 0;
                    while (*f->Indices(id) != i) ++id;
                    int x = *f->Indices((id+1)%3); // a neighbor of vertex 'i'
                    float weight;
                    if (useUniformWeight){
                        weight = 1;
                    } else {
                        int y = *f->Indices((id+2)%3); // 'i', 'x', and 'y' form a triangle
                        int z = f->OppositeVertex((id+2)%3); // 'i', 'x', and 'z' form another triangle
                        float cosAlpha = glm::dot(pos[i] - pos[z], pos[x] - pos[z]) / glm::length(pos[i] - pos[z]) / glm::length(pos[x] - pos[z]);
                        float cosBeta = glm::dot(pos[i] - pos[y], pos[x] - pos[y]) / glm::length(pos[i] - pos[y]) / glm::length(pos[x] - pos[y]);
                        cosAlpha = glm::max(glm::min(cosAlpha, 1.0f), -1.0f);
                        cosBeta = glm::max(glm::min(cosBeta, 1.0f), -1.0f);
                        float alpha = acos(cosAlpha), beta = acos(cosBeta);
                        weight = glm::cot(alpha) + glm::cot(beta);
                    }
                    sum += weight * pos[x];
                    totalWeight += weight;
                }
                newPos[i] = (1 - lambda) * pos[i] + lambda * sum / totalWeight;
            }
            pos = newPos;
        }
    }


    /*
     * Helper function for task 5
     * Compare two integer vectors (used in std::map)
     */
    struct cmp_ivec4{
        bool operator()(glm::ivec4 const &a, glm::ivec4 const &b) const {
            if (a.x != b.x) return a.x < b.x;
            if (a.y != b.y) return a.y < b.y;
            if (a.z != b.z) return a.z < b.z;
            return a.w < b.w;
        }
    };

    /******************* 5. Marching Cubes *****************/
    void MarchingCubes(Engine::SurfaceMesh & output, const std::function<float(const glm::vec3 &)> & sdf, const glm::vec3 & grid_min, const float dx, const int n) {
        /* 'edgeId' maps an edge of a cube (specified by direction and one vertex) to vertex id in the result mesh */
        std::map<glm::ivec4, int, cmp_ivec4> edgeId;
        /* March cubes */
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                for (int k = 0; k < n; ++k){
                    glm::vec3 p = grid_min + glm::vec3(i, j, k) * dx;
                    /* Calculate 'state': vertices that are inside the surface */
                    int state = 0;
                    for (int t = 0; t < 8; ++t){
                        glm::vec3 v = p + glm::vec3(t&1, t>>1&1, t>>2&1) * dx;
                        if (sdf(v) < 0) state |= 1 << t;
                    }
                    /* Iterate over triangles that should be added */
                    for (int t = 0; c_EdgeOrdsTable[state][t] != -1; t += 3){
                        for (int o = 0; o < 3; ++o){
                            int edge = c_EdgeOrdsTable[state][t+o];
                            int dir = edge >> 2;
                            glm::ivec4 key = glm::ivec4(dir, i, j, k);
                            if (edge&1) key[(dir+1)%3+1] += 1;
                            if (edge&2) key[(dir+2)%3+1] += 1;
                            if (!edgeId.count(key)){
                                edgeId[key] = output.Positions.size();
                                /* Interpolate vertex position */
                                glm::vec3 p1 = grid_min + glm::vec3(key[1], key[2], key[3]) * dx;
                                glm::vec3 p2 = p1;
                                p2[dir] += dx;
                                output.Positions.push_back(p1 + (p2 - p1) * sdf(p1) / (sdf(p1) - sdf(p2)));
                            }
                            output.Indices.push_back(edgeId[key]);
                        }
                    }
                }
    }
} // namespace VCX::Labs::GeometryProcessing
