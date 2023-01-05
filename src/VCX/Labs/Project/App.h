#pragma once

#include <vector>

#include "Engine/app.h"
#include "Labs/Project/CaseSVG.h"
#include "Labs/Common/UI.h"

namespace VCX::Labs::Project {
    class App : public Engine::IApp {
    private:
        Common::UI    _ui;

        CaseSVG     _caseSVG;

        std::size_t   _caseId = 0;

        std::vector<std::reference_wrapper<Common::ICase>> _cases = { _caseSVG };

    public:
        App();

        void OnFrame() override;
    };
}
