#include "Labs/Project/App.h"

namespace VCX::Labs::Project {

    App::App() :
        _caseSVG({ Assets::ExampleSVG::TikTok, Assets::ExampleSVG::PekingUniversity, Assets::ExampleSVG::HandwrittenText, Assets::ExampleSVG::BasicShapes, Assets::ExampleSVG::Arcs }),
        _ui(Labs::Common::UIOptions { }) {
    }

    void App::OnFrame() {
        _ui.Setup(_cases, _caseId);
    }
}
