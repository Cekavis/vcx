#include <iostream>

#include "RenderSVG.h"

namespace VCX::Labs::Project {

    static float imgWidth, imgHeight, width, height, ratio;

    bool render(ImageRGB &image, tinyxml2::XMLElement const *root, std::uint32_t &_width, std::uint32_t &_height) {
        if (root->QueryFloatAttribute("width", &imgWidth))
            return 0;
        if (root->QueryFloatAttribute("height", &imgHeight))
            return 0;
        if (_width / imgWidth < _height / imgHeight)
            _height = ceil(_width * imgHeight / imgWidth), ratio = imgWidth / _width;
        else
            _width = ceil(_height * imgWidth / imgHeight), ratio = imgHeight / _height;

        width = _width, height = _height;
        image = Common::CreatePureImageRGB(width, height, { 1., 1., 1. });

        for (auto child = root->FirstChildElement(); child != NULL; child = child->NextSiblingElement()) {
            if (child->Name() == std::string("path"))
                DrawPath(image, child);
            if (child->Name() == std::string("polygon"))
                DrawPolygon(image, child);
        }
        return 1;
    }

    void DrawPolygon(ImageRGB &image, const tinyxml2::XMLElement *path) {
        auto points = ParsePoints(path->Attribute("points"));
        if (points.empty()){
            std::cout << "Empty polygon" << std::endl;
            return;
        }
        for (auto &p : points) p /= ratio;
        points.push_back(points[0]);
        int n = points.size() - 1;
        for (int i = 0; i < n; i++)
            _drawLine(image, GetColor(path->Attribute("stroke")), points[i], points[i+1]);
        _drawPolygonFilled(image, GetColor(path->Attribute("fill")), points);
    }

    void DrawPath(ImageRGB &image, const tinyxml2::XMLElement *path) {

    }


    /* From Lab 1 */
    void _drawLine(
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
    
    /* From Lab 1 */
    void _drawTriangleFilled(
        ImageRGB &       canvas,
        glm::vec3 const  color,
        glm::ivec2 const p0,
        glm::ivec2 const p1,
        glm::ivec2 const p2) {
            
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
            else yr = (c.y*(b.x-c.x) + (b.y-c.y)*(x-c.x)) / (b.x-c.x); // floor
            for (std::size_t y = yl; y<=yr; ++y)
                if (0<=x && x < canvas.GetSizeX() && y < canvas.GetSizeY())
                    canvas.SetAt({ (std::size_t)x, y }, color);
        }
    }

    void _drawPolygonFilled(
        ImageRGB &                     canvas,
        glm::vec3 const                color,
        std::vector<glm::vec2> const & points) {
        int n = points.size() - 1;
        for (int x = 0; x < width; ++x) {
            std::vector<int> ys;
            for (int i = 0; i < n; ++i) {
                glm::vec2 const *p0 = &points[i], *p1 = &points[i+1];
                if (p0->x > p1->x) std::swap(p0, p1);
                if (p0->x <= x && x < p1->x) {
                    int y = (p0->y*(p1->x-p0->x) + (p1->y-p0->y)*(x-p0->x) + p1->x-p0->x-1) / (p1->x-p0->x);
                    ys.push_back(y);
                }
            }
            std::sort(ys.begin(), ys.end());
            for (int i = 0; i < ys.size(); i += 2)
                for (int y = ys[i]; y <= ys[i + 1]; ++y)
                    canvas.SetAt({ (std::size_t)x, (std::size_t)y }, color);
        }
    }

    std::vector<glm::vec2> ParsePoints(const char *s){
        std::vector<float> numbers;
        int pos = 0;
        for (int i = 0;; ++i) {
            if (s[i] == ' ' || s[i] == ',' || !s[i]) {
                if (i != pos)
                    numbers.push_back(std::stof(std::string(s + pos, s + i)));
                pos = i + 1;
            }
            if (!s[i]) break;
        }

        if (numbers.size() % 2 != 0)
            std::cerr << "ParsePoints: odd number of numbers in points string\n";
        std::vector<glm::vec2> points;
        for (int i = 0; i < numbers.size(); i += 2)
            points.push_back({numbers[i], numbers[i + 1]});
        return points;
    }

    glm::vec3 GetColor(const char *s){
        if (s == NULL) return {0, 0, 0};
        if (s[0] == '#') {
            if (strlen(s) == 7) {
                int r = std::stoi(std::string(s + 1, s + 3), 0, 16);
                int g = std::stoi(std::string(s + 3, s + 5), 0, 16);
                int b = std::stoi(std::string(s + 5, s + 7), 0, 16);
                return {r / 255.0f, g / 255.0f, b / 255.0f};
            }
            else {
                std::cerr << "GetColor: invalid color string\n";
                return {0, 0, 0};
            }
        }
        else if (strcmp(s, "red") == 0) return {1, 0, 0};
        else if (strcmp(s, "green") == 0) return {0, 1, 0};
        else if (strcmp(s, "blue") == 0) return {0, 0, 1};
        else if (strcmp(s, "yellow") == 0) return {1, 1, 0};
        else if (strcmp(s, "cyan") == 0) return {0, 1, 1};
        else if (strcmp(s, "magenta") == 0) return {1, 0, 1};
        else if (strcmp(s, "white") == 0) return {1, 1, 1};
        else if (strcmp(s, "black") == 0) return {0, 0, 0};
        else {
            std::cerr << "GetColor: invalid color string\n";
            return {0, 0, 0};
        }
    }
}