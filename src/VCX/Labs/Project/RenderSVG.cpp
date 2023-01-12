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
            auto size = viewbox[1];
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
        glm::vec2 p1, p2;
        if (ele->QueryFloatAttribute("x1", &p1.x)) return;
        if (ele->QueryFloatAttribute("y1", &p1.y)) return;
        if (ele->QueryFloatAttribute("x2", &p2.x)) return;
        if (ele->QueryFloatAttribute("y2", &p2.y)) return;
        p1 /= ratio, p2 /= ratio;

        glm::vec4 color = GetColor(ele, "stroke");
        float width = ele->FloatAttribute("stroke-width", 1) / ratio / 2;
        if (color.a > 0)
            _drawPolyline(color, {p1, p2}, width);
    }

    void DrawRect(const tinyxml2::XMLElement *ele) {
        float x, y, w, h, rx, ry;
        x = ele->FloatAttribute("x", 0);
        y = ele->FloatAttribute("y", 0);
        rx = ele->FloatAttribute("rx", -1);
        ry = ele->FloatAttribute("ry", -1);
        if (rx < 0) rx = ry;
        if (ry < 0) ry = rx;
        if (rx < 0) rx = ry = 0;
        if (ele->QueryFloatAttribute("width", &w)) return;
        if (ele->QueryFloatAttribute("height", &h)) return;

        // copy ele to a new element
        tinyxml2::XMLDocument doc;
        auto path = doc.NewElement("path");
        for (auto s: {"stroke", "stroke-width", "fill", "fill-opacity", "stroke-opacity"})
            if (ele->Attribute(s)) path->SetAttribute(s, ele->Attribute(s));
        std::string d = "M " + std::to_string(x + w/2) + " " + std::to_string(y);
        d += " h " + std::to_string(w/2 - rx);
        d += " a " + std::to_string(rx) + " " + std::to_string(ry) + " 0 0 1 " + std::to_string(rx) + " " + std::to_string(ry);
        d += " v " + std::to_string(h - ry*2);
        d += " a " + std::to_string(rx) + " " + std::to_string(ry) + " 0 0 1 " + std::to_string(-rx) + " " + std::to_string(ry);
        d += " h " + std::to_string(-w + rx*2);
        d += " a " + std::to_string(rx) + " " + std::to_string(ry) + " 0 0 1 " + std::to_string(-rx) + " " + std::to_string(-ry);
        d += " v " + std::to_string(-h + ry*2);
        d += " a " + std::to_string(rx) + " " + std::to_string(ry) + " 0 0 1 " + std::to_string(rx) + " " + std::to_string(-ry);
        d += " z";
        path->SetAttribute("d", d.c_str());
        DrawPath(path);

        // x /= ratio, y /= ratio, w /= ratio, h /= ratio;

        // /* Draw interior */
        // glm::vec4 color = GetColor(ele, "fill");
        // if (color.a > 0)
        //     _drawPolygonFilled(color, {std::vector<glm::vec2>({
        //         { x, y },
        //         { x + w, y },
        //         { x + w, y + h },
        //         { x, y + h },
        //         { x, y }
        //     })});
        
        // /* Draw outline */
        // color = GetColor(ele, "stroke");
        // float width = ele->FloatAttribute("stroke-width", 1) / ratio / 2;
        // if (color.a > 0) {
        //     _drawPolyline(color, std::vector<glm::vec2>({
        //         {x, y},
        //         {x + w, y},
        //         {x + w, y + h},
        //         {x, y + h},
        //         {x, y}
        //     }), width);
        //     // _drawPolyline(color, { x - width, y }, { x + w + width, y }, width);
        //     // _drawPolyline(color, { x - width, y + h }, { x + w + width, y + h }, width);
        //     // _drawPolyline(color, { x + w, y }, { x + w, y + h }, width);
        //     // _drawPolyline(color, { x, y + h }, { x, y }, width);
        // }
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
        const char *fillRule = ele->Attribute("fill-rule");
        int rule = 0;
        if (fillRule && strcmp(fillRule, "evenodd") == 0) rule = 1;
        glm::vec4 color = GetColor(ele, "fill");
        if (color.a > 0)
            _drawPolygonFilled(color, {points}, rule);

        /* Draw outline */
        color = GetColor(ele, "stroke");
        float width = ele->FloatAttribute("stroke-width", 1) / ratio / 2;
        if (color.a > 0){
            if (isPolyline) points.pop_back();
            _drawPolyline(color, points, width);
        }
    }

    void DrawCircle(const tinyxml2::XMLElement *ele) {
        float cx, cy, r;
        if (ele->QueryFloatAttribute("cx", &cx)) return;
        if (ele->QueryFloatAttribute("cy", &cy)) return;
        if (ele->QueryFloatAttribute("r", &r)) return;
        cx /= ratio, cy /= ratio, r /= ratio;

        /* Draw interior */
        glm::vec4 color = GetColor(ele, "fill");
        if (color.a > 0)
            _drawCircle(color, { cx, cy }, r);

        /* Draw outline */
        color = GetColor(ele, "stroke");
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
        const char *s = ele->Attribute("d");
        if (!s) return;
        int len = strlen(s);
        // std::cerr << "DrawPath" << std::endl; 
        for (const char *i = s; i < s + len; i++) {
            // printf("%c%c%c\n", *i, *(i+1), *(i+2));
            char command = 0;
            if (!isspace(*i)) {
                if (isalpha(*i)){
                    command = *i++;
                    lastCommand = command;
                    if (command == 'M') lastCommand = 'L';
                    if (command == 'm') lastCommand = 'l';
                }
                else command = lastCommand;//, printf("[%c]", command);
            }
            else continue;
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
                if (!path.empty()){
                    paths.push_back(path);
                    path.clear();
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
                --i;
                if (path.empty()) return;
                path.push_back(path[0]);
                x = path[0].x;
                y = path[0].y;
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
                path.push_back({ x, y });
            }
            else ++i;
            // printf("[%.5f %.5f]\n", x, y);
        }
        if (!path.empty()) paths.push_back(path);
        

        for (auto &path : paths){
            // std::cerr << path.size() << std::endl;
            // for (auto &p : path)
            //     std::cerr << p.x << " " << p.y << std::endl;
            if (path.empty()) return;
            for (auto &p : path) p /= ratio;
            path.push_back(path[0]);
        }

        /* Draw Interior */
        const char *fillRule = ele->Attribute("fill-rule");
        int rule = 0;
        if (fillRule && strcmp(fillRule, "evenodd") == 0) rule = 1;
        glm::vec4 color = GetColor(ele, "fill");
        if (color.a > 0)
            _drawPolygonFilled(color, paths, rule);
        
        /* Draw Outline */
        for (auto &path : paths){
            color = GetColor(ele, "stroke");
            float width = ele->FloatAttribute("stroke-width", 1) / ratio / 2;
            if (color.a > 0){
                path.pop_back();
                _drawPolyline(color, path, width);
            }
        }
    }

    void _draw(glm::ivec2 p, glm::vec4 color) {
        p -= view;
        if (p.x < 0 || p.y < 0 || p.x >= width || p.y >= height) return;
        std::array<size_t, 2> P = {(std::size_t)p.x, (std::size_t)p.y};
        canvas->SetAt(P, glm::vec3(color) * color.a + canvas->GetAt(P) * (1 - color.a));
    }

    void _drawPolygonFilled(
        glm::vec4 const                             color,
        std::vector<std::vector<glm::vec2>> const & polygons,
        int                                         rule) {

        for (int x = view.x; x < view.x + width; ++x) {
            if (rule == 0){
                /* nonzero */
                std::vector<std::pair<float, int>> ys;
                for (auto const &polygon : polygons) {
                    int n = polygon.size() - 1;
                    for (int i = 0; i < n; ++i) {
                        glm::vec2 const *p0 = &polygon[i], *p1 = &polygon[(i+1)%n];
                        int t = 1;
                        if (p0->x > p1->x) std::swap(p0, p1), t = -1;
                        if (p0->x <= x && x < p1->x) {
                            float y = p0->y + (p1->y-p0->y) * (x-p0->x) / (p1->x-p0->x);
                            y = std::max(std::min(p0->y, p1->y), std::min(std::max(p0->y, p1->y), y));
                            ys.emplace_back(y, t);
                        }
                    }
                }
                std::sort(ys.begin(), ys.end());
                for (int i = 0, t = 0; i < ys.size(); ++i){
                    t += ys[i].second;
                    if (t != 0)
                        for (int y = ceil(ys[i].first); y <= ys[i + 1].first; ++y)
                            _draw({ x, y }, color);
                }
            }
            else{
                /* evenodd */
                std::vector<float> ys;
                for (auto const &polygon : polygons) {
                    int n = polygon.size() - 1;
                    for (int i = 0; i < n; ++i) {
                        glm::vec2 const *p0 = &polygon[i], *p1 = &polygon[(i+1)%n];
                        if (p0->x > p1->x) std::swap(p0, p1);
                        if (p0->x <= x && x < p1->x) {
                            // float y = (p0->y*(p1->x-p0->x) + (p1->y-p0->y)*(x-p0->x) + p1->x-p0->x-1) / (p1->x-p0->x);
                            float y = p0->y + (p1->y-p0->y) * (x-p0->x) / (p1->x-p0->x);
                            y = std::max(std::min(p0->y, p1->y), std::min(std::max(p0->y, p1->y), y));
                            ys.push_back(y);
                        }
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
    }

    void _drawPolyline(
        glm::vec4 const  color,
        std::vector<glm::vec2> p,
        float            width) {
        
        if (p.size() < 2) return;
        if (p[0] == p.back()) p.push_back(p[1]);
        std::vector<glm::vec2> points;
        // for (auto &i: p) std::cerr << i.x << " " << i.y << std::endl;
        for (int _ = 0; _ < 2; ++_){
            glm::vec2 prev(0);
            bool first = true;
            for (int i = 0; i < p.size() - 1; ++i) if (glm::length(p[i+1]-p[i]) > 0.001f) {
                glm::vec2 normal = glm::normalize(glm::vec2(p[i+1].y - p[i].y, p[i].x - p[i+1].x));
                if (!first && ((p[i].x - prev.x) * (p[i+1].y - prev.y) - (p[i].y - prev.y) * (p[i+1].x - prev.x) > 0)) {
                    float alpha = acos(glm::dot(glm::normalize(p[i+1] - p[i]), glm::normalize(prev - p[i])));
                    if (alpha > asin(0.25) * 2)
                        points.push_back(p[i] + normal * width - glm::normalize(p[i+1] - p[i]) * width / tan(alpha / 2));
                }
                points.push_back(p[i] + normal * width);
                points.push_back(p[i+1] + normal * width);
                first = false;
                prev = p[i];
            }
            std::reverse(p.begin(), p.end());
        }
        points.push_back(points[0]);
        _drawPolygonFilled(color, {points});
    }

    void _drawCircle(
        glm::vec4 const  color,
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
        bool dot = 0;
        for (int i = 0;; ++i) {
            if (std::string("0123456789.-+").find(s[i]) == std::string::npos || ((s[i] == '-' || s[i]=='.') && pos < i)) {
                bool o = dot;
                if (s[i]!='.' && i != pos || dot){
                    dot = 0;
                    numbers.push_back(std::stof(std::string(s + pos, s + i)));
                }
                if (s[i]=='.'){
                    dot = 1;
                    if (o) pos = i;
                }
                else pos = i + (s[i]!='-');
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
            std::cerr << "ParsePoints: odd number of numbers in points string" << std::endl;
        std::vector<glm::vec2> points;
        for (int i = 0; i < numbers.size(); i += 2)
            points.push_back({numbers[i], numbers[i + 1]});
        return points;
    }
    
    glm::vec3 GetRGBFromHex(const char *s){
        bool ok = true;
        for (int i = 0; s[i]; ++i)
            if (!isxdigit((unsigned char)s[i]))
                ok = false;
        if (!ok || (strlen(s) != 6 && strlen(s) != 3)){
            std::cerr << "GetRGBFromHex error" << std::endl;
            return glm::vec3(0);
        }
        int x = strlen(s) / 3;
        int r = std::stoi(std::string(s      , s + x  ), 0, 16);
        int g = std::stoi(std::string(s + x  , s + x*2), 0, 16);
        int b = std::stoi(std::string(s + x*2, s + x*3), 0, 16);
        return {r / 255.0f, g / 255.0f, b / 255.0f};
    }

    glm::vec3 GetRGB(const char *s){
        if (s == NULL) s = "black";
        if (s[0] == '#') {
            return GetRGBFromHex(s + 1);
        }
        else if (strncmp(s, "rgb", 3) == 0) {
            auto c = ParseNumbers(s);
            if (c.size() < 3){
                std::cerr << "GetRGB: invalid color rgb string" << std::endl;
                return glm::vec3(0);
            }
            return glm::vec3(c[0] / 255, c[1] / 255, c[2] / 255);
        }
        /* See https://developer.mozilla.org/en-US/docs/Web/CSS/named-color */
        else if (strcmp(s, "black"  ) == 0) return GetRGBFromHex("000000");
        else if (strcmp(s, "silver" ) == 0) return GetRGBFromHex("c0c0c0");
        else if (strcmp(s, "gray"   ) == 0) return GetRGBFromHex("808080");
        else if (strcmp(s, "white"  ) == 0) return GetRGBFromHex("ffffff");
        else if (strcmp(s, "maroon" ) == 0) return GetRGBFromHex("800000");
        else if (strcmp(s, "red"    ) == 0) return GetRGBFromHex("ff0000");
        else if (strcmp(s, "purple" ) == 0) return GetRGBFromHex("800080");
        else if (strcmp(s, "fuchsia") == 0) return GetRGBFromHex("ff00ff");
        else if (strcmp(s, "green"  ) == 0) return GetRGBFromHex("008000");
        else if (strcmp(s, "lime"   ) == 0) return GetRGBFromHex("00ff00");
        else if (strcmp(s, "olive"  ) == 0) return GetRGBFromHex("808000");
        else if (strcmp(s, "yellow" ) == 0) return GetRGBFromHex("ffff00");
        else if (strcmp(s, "navy"   ) == 0) return GetRGBFromHex("000080");
        else if (strcmp(s, "blue"   ) == 0) return GetRGBFromHex("0000ff");
        else if (strcmp(s, "teal"   ) == 0) return GetRGBFromHex("008080");
        else if (strcmp(s, "aqua"   ) == 0) return GetRGBFromHex("00ffff");
        else if (strcmp(s, "orange" ) == 0) return GetRGBFromHex("ffa500");
        else {
            std::cerr << "GetRGB: invalid color string" << std::endl;
            return glm::vec3(0);
        }
    }
    
    glm::vec4 GetColor(const tinyxml2::XMLElement *ele, const char *attr) {
        const char *color = ele->Attribute(attr);
        if (!color){
            if      (strcmp(attr, "stroke") == 0) color = "none";
            else if (strcmp(attr, "fill"  ) == 0) color = "black";
            else return glm::vec4(0);
        }
        if (strcmp(color, "transparent") == 0 || 
            strcmp(color, "none"       ) == 0)
            return glm::vec4(0);
        return glm::vec4(
            GetRGB(color),
            ele->FloatAttribute((std::string(attr) + "-opacity").c_str(), 1)
        );
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
        if (rx == 0 || ry == 0) return;
        rotation = glm::radians(rotation);
        p1 = Rotate(p1 - p0, -rotation);
        p1.x /= rx;
        p1.y /= ry;
        float xxyy = p1.x * p1.x + p1.y * p1.y;
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
        std::vector<glm::vec2> p;
        float l = atan2(-center.y, -center.x);
        float r = atan2(p1.y - center.y, p1.x - center.x);
        if (sweep == 0) std::swap(l, r);
        if (l > r) r += 2 * acos(-1);
        if (sweep == 0) std::swap(l, r);
        DivideArc(center, l, r, p, ratio / (rx + ry));
        for (auto &v : p){
            v.x *= rx;
            v.y *= ry;
            v = Rotate(v, rotation) + p0;
            points.push_back(v);
        }
    }
}