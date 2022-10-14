#include <random>

#include <spdlog/spdlog.h>

#include "Labs/1-Drawing2D/tasks.h"

using VCX::Labs::Common::ImageRGB;

namespace VCX::Labs::Drawing2D {
    /******************* 1.Image Dithering *****************/
    void DitheringThreshold(
        ImageRGB &       output,
        ImageRGB const & input) {
        for (std::size_t x = 0; x < input.GetSizeX(); ++x)
            for (std::size_t y = 0; y < input.GetSizeY(); ++y) {
                glm::vec3 color = input[{ x, y }];
                output.SetAt({ x, y }, {
                                           color.r > 0.5 ? 1 : 0,
                                           color.g > 0.5 ? 1 : 0,
                                           color.b > 0.5 ? 1 : 0,
                                       });
            }
    }

    void DitheringRandomUniform(
        ImageRGB &       output,
        ImageRGB const & input) {
        std::mt19937 rng(0);
        std::uniform_real_distribution<float> d(-0.5, 0.5);
        for (std::size_t x = 0; x < input.GetSizeX(); ++x)
            for (std::size_t y = 0; y < input.GetSizeY(); ++y) {
                glm::vec3 color = input[{ x, y }];
                float noise = d(rng);
                output.SetAt({ x, y }, {
                                           color.r + noise > 0.5 ? 1 : 0,
                                           color.g + noise> 0.5 ? 1 : 0,
                                           color.b + noise> 0.5 ? 1 : 0,
                                       });
            }
    }

    void DitheringRandomBlueNoise(
        ImageRGB &       output,
        ImageRGB const & input,
        ImageRGB const & noise) {
        for (std::size_t x = 0; x < input.GetSizeX(); ++x)
            for (std::size_t y = 0; y < input.GetSizeY(); ++y) {
                glm::vec3 color = input[{ x, y }];
                glm::vec3 colorN = noise[{ x, y }];
                output.SetAt({ x, y }, {
                                           color.r + colorN.r > 1 ? 1 : 0,
                                           color.g + colorN.g > 1 ? 1 : 0,
                                           color.b + colorN.b > 1 ? 1 : 0,
                                       });
            }
    }

    void DitheringOrdered(
        ImageRGB &       output,
        ImageRGB const & input) {
        const int order[][2] = {{1, 1}, {1, 0}, {2, 1}, {1, 2}, {0, 2}, {2, 0}, {0, 0}, {2, 2}, {0, 1}};
        for (std::size_t x = 0; x < input.GetSizeX(); ++x)
            for (std::size_t y = 0; y < input.GetSizeY(); ++y) {
                glm::vec3 color = input[{ x, y }];
                for(std::size_t i = 0; i < 9; ++i)
                    output.SetAt({ x*3 + order[i][0], y*3 + order[i][1] }, {
                                           color.r >= i/9. ? 1 : 0,
                                           color.g >= i/9. ? 1 : 0,
                                           color.b >= i/9. ? 1 : 0,
                                       });
            }
    }

    void DitheringErrorDiffuse(
        ImageRGB &       output,
        ImageRGB const & input) {
        output = input;
        for (std::size_t x = 0; x < input.GetSizeX(); ++x)
            for (std::size_t y = 0; y < input.GetSizeY(); ++y) {
                glm::vec3 color = output[{ x, y }];
                if (color.r > 0.5){
                    output.SetAt({ x, y }, { 1, 1, 1 });
                    color.r -= 1;
                }
                else{
                    output.SetAt({ x, y }, { 0, 0, 0 });
                }
                if (y + 1 < input.GetSizeY())
                    output.SetAt({ x, y+1 }, output[{ x, y+1 }] + color * 7.0f / 16.0f);
                if (x + 1 < input.GetSizeX())
                    output.SetAt({ x+1, y }, output[{ x+1, y }] + color * 5.0f / 16.0f);
                if (x + 1 < input.GetSizeX() && y + 1 < input.GetSizeY())
                    output.SetAt({ x+1, y+1 }, output[{ x+1, y+1 }] + color * 1.0f / 16.0f);
                if (x + 1 < input.GetSizeX() && y > 0)
                    output.SetAt({ x+1, y-1 }, output[{ x+1, y-1 }] + color * 3.0f / 16.0f);
            }
    }

    /******************* 2.Image Filtering *****************/
    void Blur(
        ImageRGB &       output,
        ImageRGB const & input) {
        // your code here:
    }

    void Edge(
        ImageRGB &       output,
        ImageRGB const & input) {
        // your code here:
    }

    /******************* 3. Image Inpainting *****************/
    void Inpainting(
        ImageRGB &         output,
        ImageRGB const &   inputBack,
        ImageRGB const &   inputFront,
        const glm::ivec2 & offset) {
        output             = inputBack;
        size_t      width  = inputFront.GetSizeX();
        size_t      height = inputFront.GetSizeY();
        glm::vec3 * g      = new glm::vec3[width * height];
        memset(g, 0, sizeof(glm::vec3) * width * height);
        // set boundary condition
        for (std::size_t y = 0; y < height; ++y) {
            // set boundary for (0, y), your code: g[y * width] = ?
            // set boundary for (width - 1, y), your code: g[y * width + width - 1] = ?
        }
        for (std::size_t x = 0; x < width; ++x) {
            // set boundary for (x, 0), your code: g[x] = ?
            // set boundary for (x, height - 1), your code: g[(height - 1) * width + x] = ?
        }

        // Jacobi iteration, solve Ag = b
        for (int iter = 0; iter < 8000; ++iter) {
            for (std::size_t y = 1; y < height - 1; ++y)
                for (std::size_t x = 1; x < width - 1; ++x) {
                    g[y * width + x] = (g[(y - 1) * width + x] + g[(y + 1) * width + x] + g[y * width + x - 1] + g[y * width + x + 1]);
                    g[y * width + x] = g[y * width + x] * glm::vec3(0.25);
                }
        }

        for (std::size_t y = 0; y < inputFront.GetSizeY(); ++y)
            for (std::size_t x = 0; x < inputFront.GetSizeX(); ++x) {
                glm::vec3 color = g[y * width + x] + inputFront.GetAt({ x, y });
                output.SetAt({ x + offset.x, y + offset.y }, color);
            }
        delete[] g;
    }

    /******************* 4. Line Drawing *****************/
    void DrawLine(
        ImageRGB &       canvas,
        glm::vec3 const  color,
        glm::ivec2 const p0,
        glm::ivec2 const p1) {
        // your code here:
    }

    /******************* 5. Triangle Drawing *****************/
    void DrawTriangleFilled(
        ImageRGB &       canvas,
        glm::vec3 const  color,
        glm::ivec2 const p0,
        glm::ivec2 const p1,
        glm::ivec2 const p2) {
        // your code here:
    }

    /******************* 6. Image Supersampling *****************/
    void Supersample(
        ImageRGB &       output,
        ImageRGB const & input,
        int              rate) {
        // your code here:
    }

    /******************* 7. Bezier Curve *****************/
    glm::vec2 CalculateBezierPoint(
        std::span<glm::vec2> points,
        float const          t) {
        // your code here:
        return glm::vec2 {0, 0};
    }
} // namespace VCX::Labs::Drawing2D