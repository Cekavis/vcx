#include "Labs/Project/App.h"

namespace VCX::Labs::Project {

    App::App() :
        _ui(Labs::Common::UIOptions { }) {
    }

    void App::OnFrame() {
        _ui.Setup(_cases, _caseId);
    }
}
