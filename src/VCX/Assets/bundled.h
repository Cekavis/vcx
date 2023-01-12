#pragma once

#include <array>
#include <string_view>

namespace VCX::Assets {
    inline constexpr auto DefaultIcons {
        std::to_array<std::string_view>({
            "assets/images/vcl-logo-32x32.png",
            "assets/images/vcl-logo-48x48.png",
        })
    };

    inline constexpr auto DefaultFonts {
        std::to_array<std::string_view>({
            "assets/fonts/Ubuntu.ttf",
        })
    };

    inline constexpr auto ExampleSVGs {
        std::to_array<std::string_view>({
            "assets/svgs/Basic Shapes.svg",
            "assets/svgs/Peking University.svg",
            "assets/svgs/Handwritten Text.svg",
            "assets/svgs/Score.svg",
            "assets/svgs/TikTok.svg",
            "assets/svgs/Microsoft.svg",
            "assets/svgs/Twitter.svg",
            "assets/svgs/Google.svg",
            "assets/svgs/Mandala.svg",
            "assets/svgs/Girl.svg",
            "assets/svgs/Arcs.svg",
            "assets/svgs/Nonzero.svg",
            "assets/svgs/Evenodd.svg",
        })
    };
}
