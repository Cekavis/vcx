#include "Assets/bundled.hpp"
#include "Labs/Project/App.h"

int main() {
    using namespace VCX;
    return Engine::RunApp<Labs::Project::App>(Engine::AppContextOptions {
        .Title      = "VCX Project",
        .WindowSize = { 800, 600 },
        .FontSize   = 16,

        .IconFileNames = Assets::DefaultIcons,
        .FontFileNames = Assets::DefaultFonts,
    });
}
