#pragma once

#include <tinyxml2.h>
#include "Labs/Common/ImageRGB.h"

using VCX::Labs::Common::ImageRGB;

namespace VCX::Labs::Project {
    void render(ImageRGB &image, const tinyxml2::XMLElement *root, int &width, int &height, bool noDraw = false);
    void _render(tinyxml2::XMLElement const *ele);

    void DrawLine(const tinyxml2::XMLElement *ele);
    void DrawRect(const tinyxml2::XMLElement *ele);
    void DrawPolygon(const tinyxml2::XMLElement *ele, int isPolyline = 0);
    void DrawCircle(const tinyxml2::XMLElement *ele);
    void DrawEllipse(const tinyxml2::XMLElement *ele);
    void DrawPath(const tinyxml2::XMLElement *ele);

    void _drawPolygonFilled(glm::vec4, std::vector<std::vector<glm::vec2>> const &, int rule = 0);
    void _drawPolyline(glm::vec4, std::vector<glm::vec2>, float);
    void _drawCircle(glm::vec4, glm::vec2, float, float = 0);

    std::vector<float> ParseNumbers(const char *&s, int n = -1);
    std::vector<glm::vec2> ParsePoints(const char *s);
    glm::vec3 GetRGBFromHex(const char *s);
    glm::vec3 GetRGB(const char *s);
    glm::vec4 GetColor(const tinyxml2::XMLElement *ele, const char *attr);
    void DivideBezier3(std::vector<glm::vec2> &points, glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3);
    void DivideBezier2(std::vector<glm::vec2> &points, glm::vec2 p0, glm::vec2 p1, glm::vec2 p2);
    void CalcArc(std::vector<glm::vec2> &points, glm::vec2 p0, glm::vec2 p1, float rx, float ry, float rotation, int largeArc, int sweep);
}