#pragma once

#include <tinyxml2.h>
#include "Labs/Common/ImageRGB.h"

using VCX::Labs::Common::ImageRGB;

namespace VCX::Labs::Project {
    bool render(ImageRGB &image, const tinyxml2::XMLElement *root, std::uint32_t &width, std::uint32_t &height);
    void DrawLine(Common::ImageRGB &, glm::vec3, glm::ivec2, glm::ivec2);
}