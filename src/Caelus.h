#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

#include <Windows.h>

namespace Caelus
{

    enum Resolved
    {
        RESOLVED = 0, UNRESOLVED = 1
    };



    class ResolvedPos
    {
    public:
        std::optional<int> coord[4] = { std::nullopt };
        std::optional<int> size[2] = { std::nullopt };
    };

}