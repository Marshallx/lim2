#include <regex>

#include "JamlColor.h"
#include "MxiLogging.h"

namespace jaml
{
    Color Color::Parse(std::string_view const & specv)
    {
        auto const spec = std::string{ specv };
        constexpr static auto const pattern = R"(^\s*(?:(#|0x)\s*([0-9a-f]{2})\s*,?\s*([0-9a-f]{2})\s*?\s*([0-9a-f]{2}))|(rgb)\s*(?:\(|\s)?\s*([0-9]*(?:\.[0-9]*)%?)\s*[,\s]\s*([0-9]*(?:\.[0-9]*)%?)\s*[,\s]\s*([0-9]*(?:\.[0-9]*)%?)\s*\)?\s*$)";
        static auto const regex = std::regex(pattern, std::regex_constants::ECMAScript | std::regex_constants::icase);

        auto matches = std::smatch{};
        if (!std::regex_search(spec, matches, regex)) MX_THROW(std::format("Invalid color: bad format. Expected #RRGGBB or rgb(rrr,ggg,bbb) or rgb(N.F[%],N.F[%],N.F[%]), saw {}", spec).c_str());

        auto const type = matches[1].str();
        auto const red = matches[2].str();
        auto const green = matches[3].str();
        auto const blue = matches[4].str();

        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;

        constexpr static auto const parseComponent = [](std::string const & component)
        {
            size_t r = 0;
            if (component.find_first_of('%') != std::string::npos)
            {
                r = (size_t)((std::stod(component) / 100.0L) * 255.0L);
            }
            else if (component.find_first_of('.') != std::string::npos)
            {
                r = (size_t)(std::stod(component) * 255.0L);
            }
            else
            {
                r = (size_t)std::stoull(component);
            }
            if (r > 255) MX_THROW(std::format("Invalid color: each component must resolve to an integer between 0 and 255. Saw \"{}\" => {}", component, r).c_str());
            return (uint8_t)r;
        };

        if (type == "#" || type == "0x" || type == "0X")
        {
            r = (uint8_t)std::stoi(red, nullptr, 16);
            g = (uint8_t)std::stoi(green, nullptr, 16);
            b = (uint8_t)std::stoi(blue, nullptr, 16);
        }
        else
        {
            r = parseComponent(red);
            g = parseComponent(green);
            b = parseComponent(blue);
        }
        return { r,g,b };
    }
}