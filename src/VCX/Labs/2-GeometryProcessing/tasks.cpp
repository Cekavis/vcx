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

    /******************* 3. Mesh Simplification *****************/
    void SimplifyMesh(Engine::SurfaceMesh const & input, Engine::SurfaceMesh & output, float valid_pair_threshold, float simplification_ratio) {
        // your code here
    }

    /******************* 4. Mesh Smoothing *****************/
    void SmoothMesh(Engine::SurfaceMesh const & input, Engine::SurfaceMesh & output, std::uint32_t numIterations, float lambda, bool useUniformWeight) {
        // your code here
    }

    /******************* 5. Marching Cubes *****************/
    void MarchingCubes(Engine::SurfaceMesh & output, const std::function<float(const glm::vec3 &)> & sdf, const glm::vec3 & grid_min, const float dx, const int n) {
        // your code here
    }
} // namespace VCX::Labs::GeometryProcessing
