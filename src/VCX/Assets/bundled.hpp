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
            "assets/svgs/TikTok.svg",
            "assets/svgs/Peking_University_seal.svg",
            "assets/svgs/blog_logo.svg",
            "assets/svgs/basic_shapes.svg",
            "assets/svgs/arcs2.svg",
        })
    };

    enum class ExampleSVG {
        TikTok,
        Peking University,
        Hand Written text,
        
    };
}
