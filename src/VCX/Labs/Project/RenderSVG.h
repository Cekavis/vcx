#pragma once

#include <tinyxml2.h>
#include "Labs/Common/ImageRGB.h"

using VCX::Labs::Common::ImageRGB;

namespace VCX::Labs::Project {
    bool render(ImageRGB &image, const tinyxml2::XMLElement *root, std::uint32_t &width, std::uint32_t &height);

    void DrawLine(ImageRGB &image, const tinyxml2::XMLElement *path);
    void DrawRect(ImageRGB &image, const tinyxml2::XMLElement *path);
    void DrawPolygon(ImageRGB &image, const tinyxml2::XMLElement *path);
    void DrawPath(ImageRGB &image, const tinyxml2::XMLElement *path);

    void _drawLine(Common::ImageRGB &, glm::vec3, glm::ivec2, glm::ivec2);
    void _drawTriangleFilled(Common::ImageRGB &, glm::vec3, glm::ivec2, glm::ivec2, glm::ivec2);
    void _drawPolygonFilled(Common::ImageRGB &, glm::vec3, std::vector<glm::vec2> const &);
    void _drawThickLine(Common::ImageRGB &, glm::vec3, glm::vec2, glm::vec2, float);

    std::vector<glm::vec2> ParsePoints(const char *s);
    glm::vec4 GetColorFromHex(const char *s);
    glm::vec4 GetColor(const char *s);
}