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

    std::pair<uint32_t, uint32_t> edge2pair(DCEL::HalfEdge const *e) {
        return e->To() < e->From() ? std::make_pair(e->To(), e->From())
                                   : std::make_pair(e->From(), e->To());
    }

    /******************* 1. Mesh Subdivision *****************/
    void SubdivisionMesh(Engine::SurfaceMesh const & input, Engine::SurfaceMesh & output, std::uint32_t numIterations) {
        Engine::SurfaceMesh mesh = input;
        while(numIterations--) {
            DCEL links;
            links.AddFaces(mesh.Indices);
            if (!links.IsValid()){
                std::cerr << "Invalid mesh" << std::endl;
                return;
            }

            output.Positions.clear();
            output.Indices.clear();

            /* Calculate existed vertices */
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

            std::map<std::pair<uint32_t, uint32_t>, int> edgeMap;
            int num = mesh.Positions.size();

            /* Calculate new vertices */
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
                edgeMap[edge2pair(e)] = num++;
            }

            /* Divide triangles */
            for (DCEL::Triangle const &t : links.GetFaces()) {
                for (int i = 0; i < 3; ++i){
                    output.Indices.push_back(*t.Indices(i));
                    output.Indices.push_back(edgeMap[edge2pair(t.Edges((i+2)%3))]);
                    output.Indices.push_back(edgeMap[edge2pair(t.Edges((i+1)%3))]);
                }
                output.Indices.push_back(edgeMap[edge2pair(t.Edges(0))]);
                output.Indices.push_back(edgeMap[edge2pair(t.Edges(1))]);
                output.Indices.push_back(edgeMap[edge2pair(t.Edges(2))]);
            }

            mesh = output;
        }
        output = mesh;
    }

    /******************* 2. Mesh Parameterization *****************/
    void Parameterization(Engine::SurfaceMesh const & input, Engine::SurfaceMesh & output, const std::uint32_t numIterations) {
        DCEL links;
        links.AddFaces(input.Indices);
        if (!links.IsValid()){
            std::cerr << "Invalid mesh" << std::endl;
            return;
        }

        output = input;
        int n = input.Positions.size();
        uint32_t x = n;
        for (int i = 0; i < n; ++i)
            if (links.GetVertex(i).IsSide()){
                x = i;
                break;
            }
        if (x == n){
            std::cerr << "No boundary" << std::endl;
            return;
        }
        std::vector<int> boundary;
        boundary.push_back(x);
        // std::cerr << "Boundary: " << x << std::endl;
        boundary.push_back(links.GetVertex(x).GetSideNeighbors().first);
        // std::cerr << "Boundary: " << boundary.back() << std::endl;
        while (true){
            x = boundary.back();
            auto [n1, n2] = links.GetVertex(x).GetSideNeighbors();
            x = (n1 == boundary[boundary.size() - 2] ? n2 : n1);
            // std::cerr << "Boundary: " << x << std::endl;
            if (x == boundary[0])
                break;
            boundary.push_back(x);
        }
        auto &uv = output.TexCoords;
        uv = std::vector<glm::vec2>(n, glm::vec2(0));
        double pi = acos(-1);
        for (int i = 0; i < boundary.size(); ++i){
            uv[boundary[i]] = glm::vec2(cos(2 * pi * i / boundary.size()) / 2 + 0.5, sin(2 * pi * i / boundary.size()) / 2 + 0.5);
            // std::cerr << uv[boundary[i]].x << " " << uv[boundary[i]].y << std::endl;
        }
        // std::cerr << n << ' ' << boundary.size() << std::endl;
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
        std::cerr << "finish" << std::endl;
        for (int i = 0; i < n; ++i)
            assert(0<=uv[i].x && uv[i].x<=1 && 0<=uv[i].y && uv[i].y<=1);
    }

    std::pair<float, glm::vec3> CalculateCost(glm::dmat4x4 Q, glm::vec3 p1, glm::vec3 p2){
        auto P = Q;
        P[3][0] = P[3][1] = P[3][2] = 0;
        P[3][3] = 1;
        if (glm::abs(glm::determinant(P)) < 1e-15){ // Not invertible
            glm::dvec4 w1 = glm::dvec4(p1, 1);
            glm::dvec4 w2 = glm::dvec4(p2, 1);
            double co2 = glm::dot((w1 - w2), Q * (w1 - w2));
            double co1 = glm::dot(w1, Q * w2) * 2;
            if (abs(co2) < 1e-15) { // Linear function
                if (co1 > 0)
                    return {glm::dot(w2, Q * w2), p2};
                else
                    return {glm::dot(w1, Q * w1), p1};
            }
            else { // Quadratic function
                double t = -co1 / (2 * co2);
                if (t < 0)
                    return {glm::dot(w2, Q * w2), p2};
                else if (t > 1)
                    return {glm::dot(w1, Q * w1), p1};
                else
                    return {glm::dot(w1 * t + w2 * (1- t), Q * (w1 * t + w2 * (1- t))), p1 * (float)t + p2 * (1 - (float)t)};
            }
        }
        else{
            P = glm::inverse(P);
            glm::dvec4 p = glm::dvec4(0, 0, 0, 1) * P;
            // fprintf(stderr, "-------------\n");
            // fprintf(stderr, "%.3lf %.3lf %.3lf %.3lf\n", P[0][0], P[0][1], P[0][2], P[0][3]);
            // fprintf(stderr, "%.3lf %.3lf %.3lf %.3lf\n", P[1][0], P[1][1], P[1][2], P[1][3]);
            // fprintf(stderr, "%.3lf %.3lf %.3lf %.3lf\n", P[2][0], P[2][1], P[2][2], P[2][3]);
            // fprintf(stderr, "%.3lf %.3lf %.3lf %.3lf\n", P[3][0], P[3][1], P[3][2], P[3][3]);
            // fprintf(stderr, "%.3lf %.3lf %.3lf %.3lf\n", p.x, p.y, p.z, p.w);
            float cost = glm::dot(p, Q * p);
            return std::make_pair(cost, glm::vec3(p));
        }
    }

    /******************* 3. Mesh Simplification *****************/
    void SimplifyMesh(Engine::SurfaceMesh const & input, Engine::SurfaceMesh & output, float valid_pair_threshold, float simplification_ratio) {
        // std::cerr << simplification_ratio << ' ' << valid_pair_threshold << std::endl;

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

        /* Build disjoint set union */
        std::vector<int> fa(n);
        for (int i = 0; i < n; ++i) fa[i] = i;
        std::function<int(int)> find = [&](int x){
            if (fa[x] == x) return x;
            return fa[x] = find(fa[x]);
        };

        /* Delete pairs until simplification_ratio reached */
        int deleteNum = ceil(n * (1 - simplification_ratio));
        while (deleteNum--){
            // std::cerr << deleteNum << ' ' << pairs.size() << std::endl;
            if (pairs.empty()){
                std::cerr << "No valid pair" << std::endl;
                break;
            }
            auto pair = std::min_element(pairs.begin(), pairs.end())->second;
            int x = pair.first, y = pair.second;
            assert(x == find(x) && y == find(y));
            Q[x] += Q[y];
            fa[y] = x;
            // std::cerr << pos[x].x << ' ' << pos[x].y << ' ' << pos[x].z << std::endl;
            pos[x] = CalculateCost(Q[x], pos[x], pos[y]).second;
            // std::cerr << pos[x].x << ' ' << pos[x].y << ' ' << pos[x].z << std::endl;
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

        /* Rebuild structure after simplification */
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
            for (int i = 0; i < pos.size(); ++i){
                glm::vec3 sum(0);
                float totalWeight = 0;
                for (auto f: links.GetVertex(i).GetFaces()){
                    int id = 0;
                    while (*f->Indices(id) != i) ++id;
                    int x = *f->Indices((id+1)%3);
                    float weight;
                    if (useUniformWeight){
                        weight = 1;
                    } else {
                        int y = *f->Indices((id+2)%3);
                        int z = f->OppositeVertex((id+2)%3);
                        float alpha = glm::acos(glm::dot(pos[i] - pos[z], pos[x] - pos[z]) / glm::length(pos[i] - pos[z]) / glm::length(pos[x] - pos[z]));
                        float beta = glm::acos(glm::dot(pos[i] - pos[y], pos[x] - pos[y]) / glm::length(pos[i] - pos[y]) / glm::length(pos[x] - pos[y]));
                        weight = glm::cot(alpha) + glm::cot(beta);
                    }
                    sum += weight * pos[*f->Indices((id+1)%3)];
                    totalWeight += weight;
                }
                newPos[i] = (1 - lambda) * pos[i] + lambda * sum / totalWeight;
            }
            pos = newPos;
        }
    }

    /******************* 5. Marching Cubes *****************/
    void MarchingCubes(Engine::SurfaceMesh & output, const std::function<float(const glm::vec3 &)> & sdf, const glm::vec3 & grid_min, const float dx, const int n) {
        // your code here
    }
} // namespace VCX::Labs::GeometryProcessing
