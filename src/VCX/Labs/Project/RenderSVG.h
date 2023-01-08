#pragma once

#include <tinyxml2.h>
#include "Labs/Common/ImageRGB.h"

using VCX::Labs::Common::ImageRGB;

namespace VCX::Labs::Project {
    bool render(ImageRGB &image, const tinyxml2::XMLElement *root, std::uint32_t &width, std::uint32_t &height);

    void DrawLine(const tinyxml2::XMLElement *path);
    void DrawRect(const tinyxml2::XMLElement *path);
    void DrawPolygon(const tinyxml2::XMLElement *path, int isPolyline = 0);
    void DrawCircle(const tinyxml2::XMLElement *path);
    void DrawPath(const tinyxml2::XMLElement *path);

    void _drawLine(glm::vec3, glm::ivec2, glm::ivec2);
    void _drawTriangleFilled(glm::vec3, glm::ivec2, glm::ivec2, glm::ivec2);
    void _drawPolygonFilled(glm::vec3, std::vector<glm::vec2> const &);
    void _drawThickLine(glm::vec3, glm::vec2, glm::vec2, float);
    void _drawCircle(glm::vec3, glm::vec2, float, float = 0);

    std::vector<float> ParseNumbers(const char *&s, int n = -1);
    std::vector<glm::vec2> ParsePoints(const char *s);
    glm::vec4 GetColorFromHex(const char *s);
    glm::vec4 GetColor(const char *s, const char *defaultColor = "none");
    void DivideBezier3(std::vector<glm::vec2> &points, glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3);
    void DivideBezier2(std::vector<glm::vec2> &points, glm::vec2 p0, glm::vec2 p1, glm::vec2 p2);
    void CalcArc(std::vector<glm::vec2> &points, glm::vec2 p0, glm::vec2 p1, float rx, float ry, float rotation, int largeArc, int sweep);
}