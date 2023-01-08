#include <iostream>

#include "RenderSVG.h"

namespace VCX::Labs::Project {

    static ImageRGB *canvas;
    static glm::ivec2 view;
    static float width, height, ratio;

    void _render(tinyxml2::XMLElement const *ele){
        for (auto child = ele->FirstChildElement(); child != NULL; child = child->NextSiblingElement()) {
            if (child->Name() == std::string("g"))
                _render(child);
            if (child->Name() == std::string("line"))
                DrawLine(child);
            if (child->Name() == std::string("rect"))
                DrawRect(child);
            if (child->Name() == std::string("polyline"))
                DrawPolygon(child, 1);
            if (child->Name() == std::string("polygon"))
                DrawPolygon(child);
            if (child->Name() == std::string("circle"))
                DrawCircle(child);
            if (child->Name() == std::string("path"))
                DrawPath(child);
        }
    }

    bool render(ImageRGB &image, tinyxml2::XMLElement const *root, std::uint32_t &_width, std::uint32_t &_height) {
        canvas = &image;
        width = _width, height = _height;

        glm::vec2 imgSize(0);
        root->QueryFloatAttribute("width", &imgSize.x);
        root->QueryFloatAttribute("height", &imgSize.y);

        /* Calculate view box */
        if (root->Attribute("viewBox")) {
            auto viewbox = ParsePoints(root->Attribute("viewBox"));
            auto size = viewbox[1] - viewbox[0];
            if (imgSize.y == 0) imgSize = size;
            float r = imgSize.x / imgSize.y;
            if (size.x / size.y < r)
                size.x = size.y * r;
            else
                size.y = size.x / r;
            imgSize = size;
        }

        /* Calculate screen size */
        if (width / imgSize.x < height / imgSize.y)
            height = ceil(width * imgSize.y / imgSize.x), ratio = imgSize.x / width;
        else
            width = ceil(height * imgSize.x / imgSize.y), ratio = imgSize.y / height;

        if (root->Attribute("viewBox")) {
            auto viewbox = ParsePoints(root->Attribute("viewBox"));
            view = (viewbox[0] + viewbox[1] - imgSize) / 2.0f / ratio;
        }
        else view = {0, 0};

        // std::cerr << view.x << " " << view.y << std::endl;
        // std::cerr << imgSize.x << " " << imgSize.y << std::endl;

        _width = width, _height = height;
        image = Common::CreatePureImageRGB(width, height, { 1., 1., 1. });

        _render(root);
        return 1;
    }

    void DrawLine(const tinyxml2::XMLElement *ele) {
        float x1, y1, x2, y2;
        if (ele->QueryFloatAttribute("x1", &x1)) return;
        if (ele->QueryFloatAttribute("y1", &y1)) return;
        if (ele->QueryFloatAttribute("x2", &x2)) return;
        if (ele->QueryFloatAttribute("y2", &y2)) return;
        x1 /= ratio, y1 /= ratio, x2 /= ratio, y2 /= ratio;

        glm::vec4 color = GetColor(ele->Attribute("stroke"));
        float width = ele->FloatAttribute("stroke-width", 0) / ratio / 2;
        if (color.a > 0)
            if (width > 0)
                _drawThickLine(color, { x1, y1 }, { x2, y2 }, width);
            else
                _drawLine(color, { x1, y1 }, { x2, y2 });
    }

    void DrawRect(const tinyxml2::XMLElement *ele) {
        float x, y, w, h;
        if (ele->QueryFloatAttribute("x", &x)) return;
        if (ele->QueryFloatAttribute("y", &y)) return;
        if (ele->QueryFloatAttribute("width", &w)) return;
        if (ele->QueryFloatAttribute("height", &h)) return;
        x /= ratio, y /= ratio, w /= ratio, h /= ratio;

        /* Draw interior */
        glm::vec4 color = GetColor(ele->Attribute("fill"), "black");
        if (color.a > 0)
            _drawPolygonFilled(color, {
                { x, y },
                { x + w, y },
                { x + w, y + h },
                { x, y + h }
            });
        
        /* Draw outline */
        color = GetColor(ele->Attribute("stroke"));
        float width = ele->FloatAttribute("stroke-width", 1) / ratio / 2;
        if (color.a > 0) {
            _drawThickLine(color, { x - width, y }, { x + w + width, y }, width);
            _drawThickLine(color, { x - width, y + h }, { x + w + width, y + h }, width);
            _drawThickLine(color, { x + w, y }, { x + w, y + h }, width);
            _drawThickLine(color, { x, y + h }, { x, y }, width);
        }
    }

    void DrawPolygon(const tinyxml2::XMLElement *ele, int isPolyline) {
        auto points = ParsePoints(ele->Attribute("points"));
        if (points.empty()){
            std::cout << "Empty polygon" << std::endl;
            return;
        }
        for (auto &p : points) p /= ratio;
        points.push_back(points[0]);
        int n = points.size() - 1;

        /* Draw interior */
        glm::vec4 color = GetColor(ele->Attribute("fill"), "black");
        if (color.a > 0)
            _drawPolygonFilled(color, points);

        /* Draw outline */
        color = GetColor(ele->Attribute("stroke"));
        float width = ele->FloatAttribute("stroke-width", 0) / ratio / 2;
        if (color.a > 0)
            for (int i = 0; i < n - isPolyline; i++)
                if (width > 0)
                    _drawThickLine(color, points[i], points[i + 1], width);
                else
                    _drawLine(color, points[i], points[i + 1]);
    }

    void DrawCircle(const tinyxml2::XMLElement *ele) {
        float cx, cy, r;
        if (ele->QueryFloatAttribute("cx", &cx)) return;
        if (ele->QueryFloatAttribute("cy", &cy)) return;
        if (ele->QueryFloatAttribute("r", &r)) return;
        cx /= ratio, cy /= ratio, r /= ratio;

        /* Draw interior */
        glm::vec4 color = GetColor(ele->Attribute("fill"), "black");
        if (color.a > 0)
            _drawCircle(color, { cx, cy }, r);

        /* Draw outline */
        color = GetColor(ele->Attribute("stroke"));
        float width = ele->FloatAttribute("stroke-width", 1) / ratio / 2;
        if (color.a > 0)
            _drawCircle(color, { cx, cy }, r + width, r - width);
    }

    void DrawPath(const tinyxml2::XMLElement *ele) {
        float x = 0, y = 0;
        std::vector<std::vector<glm::vec2>> paths;
        std::vector<glm::vec2> path;
        static float bx = 0, by = 0; // for Bezier shortcuts
        char lastCommand = 0;
        // std::cerr << "DrawPath" << std::endl;
        const char *s = ele->Attribute("d");
        int len = strlen(s);
        for (const char *i = s; i < s + len; i++) {
            // printf("%c\n", *i);
            char command = 0;
            if (!isspace(*i)) {
                if (isalpha(*i)){
                    command = *i++;
                    lastCommand = command;
                }
                else command = lastCommand;//, printf("[%c]", command);
            }
            // if (command) std::cerr << "Command: " << command << std::endl;
            if (toupper(command) == 'M') {
                auto p = ParseNumbers(i, 2);
                if (p.size() < 2) return;
                if (command == 'm') {
                    x += p[0];
                    y += p[1];
                } else {
                    x = p[0];
                    y = p[1];
                }
                path.push_back({ x, y });
            }
            else if (toupper(command) == 'L') {
                auto p = ParseNumbers(i, 2);
                if (p.size() < 2) return;
                if (command == 'l') {
                    x += p[0];
                    y += p[1];
                } else {
                    x = p[0];
                    y = p[1];
                }
                path.push_back({ x, y });
            }
            else if (toupper(command) == 'H') {
                auto p = ParseNumbers(i, 1);
                if (p.size() < 1) return;
                if (command == 'h') x += p[0];
                else x = p[0];
                path.push_back({ x, y });
            }
            else if (toupper(command) == 'V') {
                auto p = ParseNumbers(i, 1);
                if (p.size() < 1) return;
                if (command == 'v') y += p[0];
                else y = p[0];
                path.push_back({ x, y });
            }
            else if (toupper(command) == 'Z') {
                if (path.empty()) return;
                path.push_back(path[0]);
                x = path[0].x;
                y = path[0].y;
                // paths.push_back(path);
                // path.clear();
            }
            else if (toupper(command) == 'C') {
                auto p = ParseNumbers(i, 6);
                if (p.size() < 6) return;
                if (command == 'c') {
                    p[0] += x;
                    p[1] += y;
                    p[2] += x;
                    p[3] += y;
                    p[4] += x;
                    p[5] += y;
                }
                DivideBezier3(path, { x, y }, { p[0], p[1] }, { p[2], p[3] }, { p[4], p[5] });
                bx = p[2];
                by = p[3];
                x = p[4];
                y = p[5];
                path.push_back({ x, y });
            }
            else if (toupper(command) == 'S') {
                auto p = ParseNumbers(i, 4);
                if (p.size() < 4) return;
                if (command == 's') {
                    p[0] += x;
                    p[1] += y;
                    p[2] += x;
                    p[3] += y;
                }
                if (toupper(lastCommand) == 'C' || toupper(lastCommand) == 'S')
                    bx = 2 * x - bx, by = 2 * y - by;
                else
                    bx = x, by = y;
                DivideBezier3(path, { x, y }, { bx, by }, { p[0], p[1] }, { p[2], p[3] });
                bx = p[0];
                by = p[1];
                x = p[2];
                y = p[3];
                path.push_back({ x, y });
            }
            else if (toupper(command) == 'Q') {
                auto p = ParseNumbers(i, 4);
                if (p.size() < 4) return;
                if (command == 'q') {
                    p[0] += x;
                    p[1] += y;
                    p[2] += x;
                    p[3] += y;
                }
                DivideBezier2(path, { x, y }, { p[0], p[1] }, { p[2], p[3] });
                bx = p[0];
                by = p[1];
                x = p[2];
                y = p[3];
                path.push_back({ x, y });
            }
            else if (toupper(command) == 'T') {
                auto p = ParseNumbers(i, 2);
                if (p.size() < 2) return;
                if (command == 't') {
                    p[0] += x;
                    p[1] += y;
                }
                if (toupper(lastCommand) == 'Q' || toupper(lastCommand) == 'T')
                    bx = 2 * x - bx, by = 2 * y - by;
                else
                    bx = x, by = y;
                DivideBezier2(path, { x, y }, { bx, by }, { p[0], p[1] });
                bx = x;
                by = y;
                x = p[0];
                y = p[1];
                path.push_back({ x, y });
            }
            else if (toupper(command) == 'A') {
                auto p = ParseNumbers(i, 7);
                if (p.size() < 7) return;
                if (command == 'a') {
                    p[5] += x;
                    p[6] += y;
                }
                CalcArc(path, { x, y }, { p[5], p[6] }, p[0], p[1], p[2], p[3], p[4]);
                x = p[5];
                y = p[6];
            }
            // printf("[%.5f %.5f]\n", x, y);
        }
        if (!path.empty()) paths.push_back(path);
        

        for (auto &path : paths){
            // for (auto &p : path)
            //     std::cerr << p.x << " " << p.y << std::endl;
            if (path.empty()) return;
            for (auto &p : path) p /= ratio;
            path.push_back(path[0]);
            glm::vec4 color = GetColor(ele->Attribute("fill"), "black");
            if (color.a > 0)
                _drawPolygonFilled(color, path);
            color = GetColor(ele->Attribute("stroke"));
            float width = ele->FloatAttribute("stroke-width", 0) / ratio / 2;
            if (color.a > 0)
                for (int i = 0; i < path.size() - 2; i++)
                    if (width > 0)
                        _drawThickLine(color, path[i], path[i + 1], width);
                    else
                        _drawLine(color, path[i], path[i + 1]);
        }
    }

    void _draw(glm::ivec2 p, glm::vec3 color) {
        p -= view;
        if (p.x < 0 || p.y < 0 || p.x >= width || p.y >= height) return;
        canvas->SetAt({(std::size_t)p.x, (std::size_t)p.y}, color);
    }

    /* From Lab 1 */
    void _drawLine(
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
        int y = a.y;
        int dx = 2*(b.x-a.x), dy = 2*(b.y-a.y);
        int dydx = dy-dx, F = dy-dx/2;
        for (int x = a.x; x<=b.x; ++x){
            int px = x, py = y;
            if (flipY) py = -py;
            if (swapXY) std::swap(px, py);
            _draw({px, py}, color);

            if (F<0) F+=dy;
            else ++y, F += dydx;
        }
    }
    
    /* From Lab 1 */
    // void _drawTriangleFilled(
    //     glm::vec3 const  color,
    //     glm::ivec2 const p0,
    //     glm::ivec2 const p1,
    //     glm::ivec2 const p2) {
            
    //     glm::ivec2 a = p0, b = p1, c = p2;
    //     if (a.x > b.x) std::swap(a, b);
    //     if (a.x > c.x) std::swap(a, c);
    //     if ((b.x-a.x)*(c.y-a.y) < (c.x-a.x)*(b.y-a.y)) std::swap(b, c);
    //     for (int x = a.x; x <= b.x || x <= c.x; ++x){
    //         int yl, yr;
    //         if (x <= b.x) {
    //             if (b.x == a.x) yl = std::min(a.y, b.y);
    //             else yl = (a.y*(b.x-a.x) + (b.y-a.y)*(x-a.x) + b.x-a.x-1) / (b.x-a.x); // ceil
    //         }
    //         else yl = (b.y*(c.x-b.x) + (c.y-b.y)*(x-b.x) + c.x-b.x-1) / (c.x-b.x); // ceil
    //         if (x <= c.x) {
    //             if (c.x == a.x) yr = std::max(a.y, c.y);
    //             else yr = (a.y*(c.x-a.x) + (c.y-a.y)*(x-a.x)) / (c.x-a.x); // floor
    //         }
    //         else yr = (c.y*(b.x-c.x) + (b.y-c.y)*(x-c.x)) / (b.x-c.x); // floor
    //         for (int y = yl; y<=yr; ++y)
    //             if (0<=x && x < width)
    //                 _draw({ x, y }, color);
    //     }
    // }

    void _drawPolygonFilled(
        glm::vec3 const                color,
        std::vector<glm::vec2> const & points) {

        int n = points.size() - 1;
        for (int x = view.x; x < view.x + width; ++x) {
            std::vector<float> ys;
            for (int i = 0; i < n; ++i) {
                glm::vec2 const *p0 = &points[i], *p1 = &points[i+1];
                if (p0->x > p1->x) std::swap(p0, p1);
                if (p0->x <= x && x < p1->x) {
                    // float y = (p0->y*(p1->x-p0->x) + (p1->y-p0->y)*(x-p0->x) + p1->x-p0->x-1) / (p1->x-p0->x);
                    float y = p0->y + (p1->y-p0->y) * (x-p0->x) / (p1->x-p0->x);
                    y = std::max(std::min(p0->y, p1->y), std::min(std::max(p0->y, p1->y), y));
                    ys.push_back(y);
                }
            }
            std::sort(ys.begin(), ys.end());
            // if (!ys.empty()){
            //     for (int y: ys) printf("[%d]", y);
            //     printf("\n");
            // }
            for (int i = 0; i < ys.size(); i += 2){
                for (int y = ceil(ys[i]); y <= ys[i + 1]; ++y)
                    _draw({ x, y }, color);
                // if (ys[i+1] - ys[i] < 2)
                //     printf("[%.5f]\n", ys[i+1] - ys[i]);
            }
        }
    }

    void _drawThickLine(
        glm::vec3 const  color,
        glm::vec2 const p0,
        glm::vec2 const p1,
        float            width) {
        
        std::vector<glm::vec2> points;
        glm::vec2 normal = glm::normalize(glm::vec2(p1.y - p0.y, p0.x - p1.x));
        points.push_back(p0 + normal * width);
        points.push_back(p0 - normal * width);
        points.push_back(p1 - normal * width);
        points.push_back(p1 + normal * width);
        points.push_back(p0 + normal * width);
        _drawPolygonFilled(color, points);
    }

    void _drawCircle(
        glm::vec3 const  color,
        glm::vec2 const  center,
        float            r1,
        float            r2) {
        
        for (int x = view.x; x < view.x + width; ++x) if (fabs(x - center.x) <= r1) {
            float y1 = center.y - sqrt(r1*r1 - (x-center.x)*(x-center.x));
            float y2 = center.y + sqrt(r1*r1 - (x-center.x)*(x-center.x));
            float y3 = y1, y4 = y1;
            if (fabs(x - center.x) <= r2) {
                y3 = center.y - sqrt(r2*r2 - (x-center.x)*(x-center.x));
                y4 = center.y + sqrt(r2*r2 - (x-center.x)*(x-center.x));
            }
            for (int y = y1; y < y3; ++y)
                _draw({ x, y }, color);
            for (int y = y4; y < y2; ++y)
                _draw({ x, y }, color);
        }
    }

    std::vector<float> ParseNumbers(const char *&s, int n){
        std::vector<float> numbers;
        int pos = 0;
        for (int i = 0;; ++i) {
            if (!s[i] || std::string("0123456789.-+").find(s[i]) == std::string::npos || (s[i] == '-' && pos < i)) {
                if (i != pos)
                    numbers.push_back(std::stof(std::string(s + pos, s + i)));
                pos = i + (s[i]!='-');
            }
            if (!s[i] || numbers.size() == n){
                s += i - 1;
                break;
            }
        }
        // for (auto i: numbers)
        //     printf("[%f]", i);
        // printf("\n");
        return numbers;
    }

    std::vector<glm::vec2> ParsePoints(const char *s){
        auto numbers = ParseNumbers(s);
        if (numbers.size() % 2 != 0)
            std::cerr << "ParsePoints: odd number of numbers in points string\n";
        std::vector<glm::vec2> points;
        for (int i = 0; i < numbers.size(); i += 2)
            points.push_back({numbers[i], numbers[i + 1]});
        return points;
    }
    
    glm::vec4 GetColorFromHex(const char *s){
        int r = std::stoi(std::string(s    , s + 2), 0, 16);
        int g = std::stoi(std::string(s + 2, s + 4), 0, 16);
        int b = std::stoi(std::string(s + 4, s + 6), 0, 16);
        return {r / 255.0f, g / 255.0f, b / 255.0f, 1};
    }

    glm::vec4 GetColor(const char *s, const char *defaultColor){
        if (s == NULL) s = defaultColor;
        if (s == NULL) return glm::vec4(0);
        if (s[0] == '#') {
            if (strlen(s) == 7){
                bool ok = true;
                for (int i = 1; i < 7; ++i)
                    if (!isxdigit((unsigned char)s[i]))
                        ok = false;
                if (ok) return GetColorFromHex(s + 1);
            }
            std::cerr << "GetColor: invalid color string\n";
            return glm::vec4(0);
        }
        /* See https://developer.mozilla.org/en-US/docs/Web/CSS/named-color */
        else if (strcmp(s, "black"  ) == 0) return GetColorFromHex("000000");
        else if (strcmp(s, "silver" ) == 0) return GetColorFromHex("c0c0c0");
        else if (strcmp(s, "gray"   ) == 0) return GetColorFromHex("808080");
        else if (strcmp(s, "white"  ) == 0) return GetColorFromHex("ffffff");
        else if (strcmp(s, "maroon" ) == 0) return GetColorFromHex("800000");
        else if (strcmp(s, "red"    ) == 0) return GetColorFromHex("ff0000");
        else if (strcmp(s, "purple" ) == 0) return GetColorFromHex("800080");
        else if (strcmp(s, "fuchsia") == 0) return GetColorFromHex("ff00ff");
        else if (strcmp(s, "green"  ) == 0) return GetColorFromHex("008000");
        else if (strcmp(s, "lime"   ) == 0) return GetColorFromHex("00ff00");
        else if (strcmp(s, "olive"  ) == 0) return GetColorFromHex("808000");
        else if (strcmp(s, "yellow" ) == 0) return GetColorFromHex("ffff00");
        else if (strcmp(s, "navy"   ) == 0) return GetColorFromHex("000080");
        else if (strcmp(s, "blue"   ) == 0) return GetColorFromHex("0000ff");
        else if (strcmp(s, "teal"   ) == 0) return GetColorFromHex("008080");
        else if (strcmp(s, "aqua"   ) == 0) return GetColorFromHex("00ffff");
        else if (strcmp(s, "orange" ) == 0) return GetColorFromHex("ffa500");

        else if (strcmp(s, "transparent") == 0) return glm::vec4(0);
        else if (strcmp(s, "none") == 0) return glm::vec4(0);
        else {
            std::cerr << "GetColor: invalid color string\n";
            return {0, 0, 0, 0};
        }
    }
    
    void DivideBezier3(std::vector<glm::vec2> &points, glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3) {
        glm::vec2 p01 = (p0 + p1) * 0.5f;
        glm::vec2 p12 = (p1 + p2) * 0.5f;
        glm::vec2 p23 = (p2 + p3) * 0.5f;
        glm::vec2 p012 = (p01 + p12) * 0.5f;
        glm::vec2 p123 = (p12 + p23) * 0.5f;
        glm::vec2 p0123 = (p012 + p123) * 0.5f;
        if (glm::length(p0123 - p0) < ratio && glm::length(p0123 - p3) < ratio) {
            points.push_back(p0123);
        } else {
            DivideBezier3(points, p0, p01, p012, p0123);
            DivideBezier3(points, p0123, p123, p23, p3);
        }
    }

    void DivideBezier2(std::vector<glm::vec2> &points, glm::vec2 p0, glm::vec2 p1, glm::vec2 p2) {
        glm::vec2 p01 = (p0 + p1) * 0.5f;
        glm::vec2 p12 = (p1 + p2) * 0.5f;
        glm::vec2 p012 = (p01 + p12) * 0.5f;
        if (glm::length(p012 - p0) < ratio && glm::length(p012 - p2) < ratio) {
            points.push_back(p012);
        } else {
            DivideBezier2(points, p0, p01, p012);
            DivideBezier2(points, p012, p12, p2);
        }
    }

    glm::vec2 Rotate(glm::vec2 p, float angle) {
        float c = cos(angle);
        float s = sin(angle);
        return {p.x * c - p.y * s, p.x * s + p.y * c};
    }

    void DivideArc(glm::vec2 center, float l, float r, std::vector<glm::vec2> &p, float th){
        glm::vec2 p0 = {cos(l), sin(l)};
        glm::vec2 p1 = {cos(r), sin(r)};
        if (glm::length(p1 - p0) > th || fabs(r - l) > 1){
            float m = (l + r) * 0.5f;
            DivideArc(center, l, m, p, th);
            DivideArc(center, m, r, p, th);
        } else {
            p.push_back(center + p1);
        }
    }

    void CalcArc(std::vector<glm::vec2> &points, glm::vec2 p0, glm::vec2 p1, float rx, float ry, float rotation, int largeArc, int sweep) {
        rotation = glm::radians(rotation);
        p1 = Rotate(p1 - p0, -rotation);
        p1.x /= rx;
        p1.y /= ry;
        // std::cerr << p1.x << " " << p1.y << std::endl;
        float xxyy = p1.x * p1.x + p1.y * p1.y;
        // std::cerr << xxyy << std::endl;
        if (xxyy > 4) {
            float k = sqrt(xxyy) / 2;
            p1 /= k;
            rx *= k;
            ry *= k;
            xxyy = 4;
        }
        glm::vec2 center;
        center.x = (p1.x - p1.y * sqrt((4 - xxyy) * xxyy) / xxyy) / 2;
        center.y = (p1.y + p1.x * sqrt((4 - xxyy) * xxyy) / xxyy) / 2;
        if ((largeArc == 1) ^ (sweep == 1) ^ (center.x * p1.y - center.y * p1.x < 0))
            center = p1 - center;
        // std::cerr << center.x << " " << center.y << std::endl;
        std::vector<glm::vec2> p;
        float l = atan2(-center.y, -center.x);
        float r = atan2(p1.y - center.y, p1.x - center.x);
        if (sweep == 0) std::swap(l, r);
        if (l > r) r += 2 * acos(-1);
        if (sweep == 0) std::swap(l, r);
        // std::cerr << l << ' ' << r << std::endl;
        DivideArc(center, l, r, p, ratio / (rx + ry));
        for (auto &v : p){
            v.x *= rx;
            v.y *= ry;
            v = Rotate(v, rotation) + p0;
            points.push_back(v);
            // std::cerr << v.x << " " << v.y << std::endl;
        }
        points.push_back(p1 + p0);
        // puts("--------------------");
    }
}