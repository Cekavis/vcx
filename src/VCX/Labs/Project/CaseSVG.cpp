#include <algorithm>
#include <array>
#include <iostream>
#include <tinyxml2.h>
#include <chrono>

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
        if (ImGui::BeginCombo("SVG", GetSVGName(_SVGIdx).c_str())) {
            for (std::size_t i = 0; i < Assets::ExampleSVGs.size(); ++i) {
                bool selected = i == _SVGIdx;
                if (ImGui::Selectable(GetSVGName(i).c_str(), selected)) {
                    if (! selected) {
                        _SVGIdx  = i;
                        _recompute = true;
                    }
                }
            }
            ImGui::EndCombo();
        }
        ImGui::Spacing();

        _recompute |= ImGui::SliderInt("Scale", &_scale, 1, 3);
        ImGui::Spacing();

        std::string path = GetSVGName(_SVGIdx);
        path = path.substr(0, path.length() - 4) + "-" + std::to_string(_width) + "x" + std::to_string(_height) + "-" + std::to_string(_time) + "ms.png";
        Common::ImGuiHelper::SaveImage(_texture, std::make_pair(_width, _height), path.c_str());
        ImGui::Spacing();

        ImGui::Checkbox("Zoom Tooltip", &_enableZoom);
        ImGui::Spacing();


        ImGui::Text("Size: %dx%d", _width, _height);
        ImGui::Spacing();
        
        ImGui::Text("Rendering Time: %dms", _time);
        ImGui::Spacing();

    }

    Common::CaseRenderResult CaseSVG::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        int width = std::max(100, (int)desiredSize.first - 40) * _scale;
        int height = std::max(100, (int)desiredSize.second - 40) * _scale;
        if (width != _windowWidth || height != _windowHeight) {
            _windowWidth = width;
            _windowHeight = height;
            _recompute = true;
        }

        if (_recompute) {
            _recompute = false;
            _width = width;
            _height = height;

            auto tex { Common::CreatePureImageRGB(100, 100, { 1., 1., 1. }) };

            auto start = std::chrono::system_clock::now();
            tinyxml2::XMLDocument doc;
            // std::cerr << "Loading SVG file: " << GetSVGName(_SVGIdx) << std::endl;
            if (doc.LoadFile(std::filesystem::path(Assets::ExampleSVGs[_SVGIdx]).string().c_str())) {
                std::cerr << "Failed to load SVG file: " << GetSVGName(_SVGIdx) << std::endl;
            }
            else {
                auto const * root = doc.FirstChildElement("svg");
                if (!root) {
                    std::cerr << "Failed to find root element in SVG file: " << GetSVGName(_SVGIdx) << std::endl;
                }
                else {
                    if (!render(tex, root, _width, _height))
                        std::cerr << "Failed to render SVG file: " << GetSVGName(_SVGIdx) << std::endl;
                }
            }
            auto end = std::chrono::system_clock::now();
            _time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

            /* Downsampling */
            _width /= _scale, _height /= _scale;
            auto _tex { Common::CreatePureImageRGB(_width, _height, { 1., 1., 1. }) };
            for (std::size_t i = 0; i < _width; ++i)
                for(std::size_t j = 0; j < _height; ++j){
                    glm::vec3 color(0);
                    for (int x = 0; x < _scale; ++x)
                        for (int y = 0; y < _scale; ++y)
                            color += tex.GetAt({i * _scale + x, j * _scale + y});
                    color /= _scale * _scale;
                    _tex.SetAt({i, j}, color);
                }
            gl_using(_texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _width, _height, 0, GL_RGB, GL_UNSIGNED_BYTE, _tex.GetBytes().data());
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
            Common::ImGuiHelper::ZoomTooltip(_texture, { _width / _scale, _height / _scale}, pos);
    }
}
