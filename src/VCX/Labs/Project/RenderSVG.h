#pragma once

#include <tinyxml2.h>
#include "Labs/Common/ImageRGB.h"

using VCX::Labs::Common::ImageRGB;

namespace VCX::Labs::Project {
    bool render(ImageRGB &image, const tinyxml2::XMLElement *root, std::uint32_t &width, std::uint32_t &height);

    void DrawPolygon(ImageRGB &image, const tinyxml2::XMLElement *path);
    void DrawPath(ImageRGB &image, const tinyxml2::XMLElement *path);

    void _drawLine(Common::ImageRGB &, glm::vec3, glm::ivec2, glm::ivec2);
    void _drawTriangleFilled(Common::ImageRGB &, glm::vec3, glm::ivec2, glm::ivec2, glm::ivec2);
    void _drawPolygonFilled(Common::ImageRGB &, glm::vec3, std::vector<glm::vec2> const &);

    std::vector<glm::vec2> ParsePoints(const char *s);
    glm::vec3 GetColor(const char *s);
}