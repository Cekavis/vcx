#include <iostream>

#include "RenderSVG.h"

namespace VCX::Labs::Project {

    static float xmin = 0, xmax = 0, ymin = 0, ymax = 0;

    bool render(ImageRGB &image, tinyxml2::XMLElement const *root, std::uint32_t &width, std::uint32_t &height) {
        xmin = ymin = INFINITY;
        xmax = ymax = -INFINITY;
        DrawLine(image, {1, 0, 0}, {1, 1}, {width-1, height-1});
        return 1;
    }

    /* From Lab 1 */
    void DrawLine(
        ImageRGB &       canvas,
        glm::vec3 const  color,
        glm::ivec2 const p0,
        glm::ivec2 const p1) {

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
}