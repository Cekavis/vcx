#include <algorithm>
#include <array>

#include "Labs/Project/CaseSVG.h"
#include "Labs/Common/ImGuiHelper.h"

namespace VCX::Labs::Project {

    static constexpr auto c_Sizes = std::to_array<std::pair<std::uint32_t, std::uint32_t>>({
        { 320U, 320U },
        { 640U, 640U } 
    });

    static constexpr auto c_SizeItems = std::array<char const *, 2> {
        "Small (320 x 320)",
        "Large (640 x 640)"
    };

    static constexpr auto c_BgItems = std::array<char const *, 3> {
        "White",
        "Black",
        "Checkboard"
    };

    CaseSVG::CaseSVG() : 
        _empty(
            Common::CreatePureImageRGB(100, 100, { 2.f / 17, 2.f / 17, 2.f / 17 })
        ) {
        auto const useTexture { _texture.Use() };
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 100, 100, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    }

    void CaseSVG::OnSetupPropsUI() {
        ImGui::Checkbox("Zoom Tooltip", &_enableZoom);
        ImGui::InputText("File Path", _filePath, sizeof(_filePath));
    }

    Common::CaseRenderResult CaseSVG::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        auto const width = desiredSize.first - 40;
        auto const height = desiredSize.second - 40;
        if (width != _width || height != _height) {
            _width = width;
            _height = height;
            _recompute = true;
        }

        if (_recompute) {
            _recompute = false;
            _task.Emplace([=]() {
                Common::ImageRGB image({ 0, 0 });
                

                return image;
            });
        }
        auto const useTexture { _texture.Use() };
        _empty = Common::CreatePureImageRGB(width, height, { 2.f / 17, 2.f / 17, 2.f / 17 });
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, 0, 0, width, height,
            GL_RGB, GL_UNSIGNED_BYTE,
            _task.ValueOr(_empty).GetBytes().data());
        glGenerateMipmap(GL_TEXTURE_2D);
        return Common::CaseRenderResult {
            .Fixed     = true,
            .Image     = _texture,
            .ImageSize = { width, height }
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
