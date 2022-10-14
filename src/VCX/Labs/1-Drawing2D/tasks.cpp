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
        for (std::size_t x = 0; x < input.GetSizeX()-2; ++x)
            for (std::size_t y = 0; y < input.GetSizeY()-2; ++y) {
                glm::vec3 color = {0, 0, 0};
                for (std::size_t i = 0; i < 3; ++i)
                        for (std::size_t j = 0; j < 3; ++j)
                                color += input[{ x+i, y+j }];
                color /= 9;
                output.SetAt({ x, y }, color);
            }
    }

    void Edge(
        ImageRGB &       output,
        ImageRGB const & input) {
        const float filter1[3][3] = {
            { 1., 2., 1. },
            { 0, 0, 0 },
            { -1., -2., -1. }
        };
        const float filter2[3][3] = {
            { 1., 0, -1. },
            { 2., 0, -2. },
            { 1., 0, -1. }
        };
        for (std::size_t x = 0; x < input.GetSizeX()-2; ++x)
            for (std::size_t y = 0; y < input.GetSizeY()-2; ++y) {
                glm::vec3 gx = {0, 0, 0}, gy = gx;
                for (std::size_t i = 0; i < 3; ++i)
                        for (std::size_t j = 0; j < 3; ++j){
                            gx += input[{ x+i, y+j }] * filter1[i][j];
                            gy += input[{ x+i, y+j }] * filter2[i][j];
                        }
                output.SetAt({ x, y }, glm::sqrt(gx * gx + gy * gy));
            }
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
            g[y * width] = inputBack.GetAt({(std::size_t)offset.x, offset.y + y}) - inputFront.GetAt({0, y});
            // set boundary for (width - 1, y), your code: g[y * width + width - 1] = ?
            g[y * width + width - 1] = inputBack.GetAt({offset.x + width - 1, offset.y + y}) - inputFront.GetAt({width - 1, y});
        }
        for (std::size_t x = 0; x < width; ++x) {
            // set boundary for (x, 0), your code: g[x] = ?
            g[x] = inputBack.GetAt({offset.x + x, (std::size_t)offset.y}) - inputFront.GetAt({x, 0});
            // set boundary for (x, height - 1), your code: g[(height - 1) * width + x] = ?
            g[(height - 1) * width + x] = inputBack.GetAt({offset.x + x, offset.y + height - 1}) - inputFront.GetAt({x, height - 1});
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
        glm::ivec2 a = p0, b = p1;
        bool swapXY = abs(a.x-b.x) < abs(a.y-b.y);
        if (swapXY) {
            std::swap(a.x, a.y);
            std::swap(b.x, b.y);
        }
        if (a.x > b.x) std::swap(a, b);
        bool flipY = a.y > b.y;
        if (flipY) {
            a.y = -a.y;
            b.y = -b.y;
        }
        std::size_t y = a.y;
        int dx = 2*(b.x-a.x), dy = 2*(b.y-a.y);
        int dydx = dy-dx, F = dy-dx/2;
        for (std::size_t x = a.x; x<=b.x; ++x){
            std::size_t px = x, py = y;
            if (flipY) py = -py;
            if (swapXY) std::swap(px, py);
            canvas.SetAt({px, py}, color);

            if (F<0) F+=dy;
            else ++y, F += dydx;
        }
    }

    /******************* 5. Triangle Drawing *****************/
    void DrawTriangleFilled(
        ImageRGB &       canvas,
        glm::vec3 const  color,
        glm::ivec2 const p0,
        glm::ivec2 const p1,
        glm::ivec2 const p2) {
        // your code here:
        glm::ivec2 a = p0, b = p1, c = p2;
        if (a.x > b.x) std::swap(a, b);
        if (a.x > c.x) std::swap(a, c);
        if ((b.x-a.x)*(c.y-a.y) < (c.x-a.x)*(b.y-a.y)) std::swap(b, c);
        for (int x = a.x; x <= b.x || x <= c.x; ++x){
            int yl, yr;
            if (x <= b.x) {
                if (b.x == a.x) yl = std::min(a.y, b.y);
                else yl = (a.y*(b.x-a.x) + (b.y-a.y)*(x-a.x) + b.x-a.x-1) / (b.x-a.x); // ceil
            }
            else yl = (b.y*(c.x-b.x) + (c.y-b.y)*(x-b.x) + c.x-b.x-1) / (c.x-b.x); // ceil
            if (x <= c.x) {
                if (c.x == a.x) yr = std::max(a.y, c.y);
                else yr = (a.y*(c.x-a.x) + (c.y-a.y)*(x-a.x)) / (c.x-a.x); // floor
            }
            else yr = (c.y*(b.x-c.x) + (b.y-c.y)*(x-c.x)) / (b.x-c.x);
            for (std::size_t y = yl; y<=yr; ++y)
                if (0<=x && x < canvas.GetSizeX() && y < canvas.GetSizeY())
                    canvas.SetAt({ (std::size_t)x, y }, color);
        }
    }

    /******************* 6. Image Supersampling *****************/
    void Supersample(
        ImageRGB &       output,
        ImageRGB const & input,
        int              rate) {
        // your code here:
        int ix = input.GetSizeX(), iy = input.GetSizeY();
        int ox = output.GetSizeX(), oy = output.GetSizeY();
        for(int i=0; i<ox; ++i) for(int j=0; j<oy; ++j){
            int x = i*ix/ox, y = j*iy/oy;
            glm::vec3 color = {0, 0, 0};
            int cnt = 0;
            for(int tx=0; tx<rate; ++tx) for(int ty=0; ty<rate; ++ty)
                if (x+tx<ix && y+ty<iy){
                    color += input.GetAt({ (std::size_t)x+tx, (std::size_t)y+ty });
                    ++cnt;
                }
            color /= cnt;
            output.SetAt({ (std::size_t)i, (std::size_t)j }, color);
        }
    }

    /******************* 7. Bezier Curve *****************/
    glm::vec2 CalculateBezierPoint(
        std::span<glm::vec2> points,
        float const          t) {
        // your code here:
        if (points.size() == 1)
            return points[0];
        std::vector<glm::vec2> p;
        for (std::size_t i = 0; i < points.size() - 1; ++i)
            p.push_back(points[i]*(1-t) + points[i+1]*t);
        return CalculateBezierPoint(p, t);
    }
} // namespace VCX::Labs::Drawing2D