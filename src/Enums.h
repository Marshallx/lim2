#pragma once

#include <stdexcept>
#include <string_view>

namespace mx
{

    class OrderBy
    {
    public:
        static OrderBy COLOR;
        static OrderBy DESCRIPTION;
        static OrderBy ITEMID;
        static OrderBy SETID;
        static OrderBy WANTEDQTY;
        static OrderBy SETQTY;
        static OrderBy SHORTQTY;
        static OrderBy SPAREQTY;
        static OrderBy LASTCHECKED;

        enum class Value
        {
            COLOR, DESCRIPTION, ITEMID, SETID, WANTEDQTY, SETQTY, SHORTQTY, SPAREQTY, LASTCHECKED
        };

        OrderBy() = delete;
        OrderBy(OrderBy const &) = delete; // Disallow copy
        OrderBy(OrderBy &&) = delete; // Disallow move
        explicit operator bool() const = delete; // Disallow if(x)

        constexpr operator Value() const { return val; } // Allow switch(x)

        char const * ToString() const noexcept { return str; }

        static OrderBy * FromString(std::string_view const & s);

    private:
        OrderBy(OrderBy::Value v, char const * s) : val(v), str(s) {};
        Value val;
        char const * str;
    };

}