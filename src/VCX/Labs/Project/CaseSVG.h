#pragma once

#include "Engine/GL/Frame.hpp"
#include "Engine/Async.hpp"
#include "Labs/Common/ICase.h"
#include "Labs/Common/ImageRGB.h"
#include "Assets/bundled.h"

namespace VCX::Labs::Project {

    class CaseSVG : public Common::ICase {
    public:
        CaseSVG(std::initializer_list<Assets::ExampleSVG> && SVGs);

        virtual std::string_view const GetName() override { return "Draw fixed images"; }
        
        virtual void OnSetupPropsUI() override;
        virtual Common::CaseRenderResult OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) override;
        virtual void OnProcessInput(ImVec2 const & pos) override;
    
    private:

        std::vector<Assets::ExampleSVG> _SVGs;

        Engine::GL::UniqueTexture2D _texture;

        int  _SVGIdx        = 0;
        int  _windowWidth   = 0;
        int  _windowHeight  = 0;
        int  _width         = 0;
        int  _height        = 0;
        bool _enableZoom    = true;
        bool _recompute     = true;

        const std::string GetSVGName(std::size_t const i) const { return std::filesystem::path(Assets::ExampleSVGs[i]).filename().string(); }
    };
}
