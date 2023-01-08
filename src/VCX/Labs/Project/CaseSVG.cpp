#include <algorithm>
#include <array>
#include <iostream>
#include <tinyxml2.h>

#include "Labs/Project/CaseSVG.h"
#include "Labs/Common/ImGuiHelper.h"
#include "Labs/Project/RenderSVG.h"

namespace VCX::Labs::Project {

    CaseSVG::CaseSVG() {
        gl_using(_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 320, 320, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    }

    void CaseSVG::OnSetupPropsUI() {
        ImGui::Checkbox("Zoom Tooltip", &_enableZoom);
        ImGui::InputText("File Path", _filePath, sizeof(_filePath));
    }

    Common::CaseRenderResult CaseSVG::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        std::uint32_t width = desiredSize.first - 40;
        std::uint32_t height = desiredSize.second - 40;
        if (width != _windowWidth || height != _windowHeight) {
            _windowWidth = width;
            _windowHeight = height;
            _recompute = true;
        }

        if (_recompute) {
            _recompute = false;
            auto tex { Common::CreateCheckboardImageRGB(320, 320) };

            tinyxml2::XMLDocument doc;
            // std::cerr << "Loading SVG file: " << _filePath << std::endl;
            if (doc.LoadFile(_filePath)) {
                std::cerr << "Failed to load SVG file: " << _filePath << std::endl;
            }
            else {
                auto const * root = doc.FirstChildElement("svg");
                if (!root) {
                    std::cerr << "Failed to find root element in SVG file: " << _filePath << std::endl;
                }
                else {
                    if (render(tex, root, width, height)) {
                        _width = width;
                        _height = height;
                    }
                    else std::cerr << "Failed to render SVG file: " << _filePath << std::endl;
                }
            }

            gl_using(_texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex.GetBytes().data());
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        return Common::CaseRenderResult {
            .Fixed     = true,
            .Image     = _texture,
            .ImageSize = { _width, _height }
        };
    }

    void CaseSVG::OnProcessInput(ImVec2 const & pos) {
        auto         window  = ImGui::GetCurrentWindow();
        bool         hovered = false;
        bool         held    = false;
        ImVec2 const delta   = ImGui::GetIO().MouseDelta;
        ImGui::ButtonBehavior(window->Rect(), window->GetID("##io"), &hovered, &held, ImGuiButtonFlags_MouseButtonLeft);
        if (held && delta.x != 0.f)
            ImGui::SetScrollX(window, window->Scroll.x - delta.x);
        if (held && delta.y != 0.f)
            ImGui::SetScrollY(window, window->Scroll.y - delta.y);
        if (_enableZoom && ! held && ImGui::IsItemHovered())
            Common::ImGuiHelper::ZoomTooltip(_texture, { _width, _height}, pos);
    }
}
