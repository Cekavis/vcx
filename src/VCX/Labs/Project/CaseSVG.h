#pragma once

#include "Engine/GL/Frame.hpp"
#include "Engine/Async.hpp"
#include "Labs/Common/ICase.h"
#include "Labs/Common/ImageRGB.h"

namespace VCX::Labs::Project {

    class CaseSVG : public Common::ICase {
    public:
        CaseSVG();

        virtual std::string_view const GetName() override { return "Draw fixed images"; }
        
        virtual void OnSetupPropsUI() override;
        virtual Common::CaseRenderResult OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) override;
        virtual void OnProcessInput(ImVec2 const & pos) override;
    
    private:

        Engine::GL::UniqueTexture2D _texture;

        Common::ImageRGB _empty;

        Engine::Async<Common::ImageRGB> _task;

        int  _width         = 0;
        int  _height        = 0;
        bool _enableZoom    = true;
        bool _recompute     = true;
        char _filePath[256] = "C:\\Users\\Vlad\\Desktop\\test.svg";
    };
}
