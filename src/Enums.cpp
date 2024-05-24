#include "Enums.h"

namespace mx
{
    OrderBy OrderBy::COLOR = { OrderBy::Value::COLOR, "COLOR" };
    OrderBy OrderBy::DESCRIPTION = { OrderBy::Value::DESCRIPTION, "DESCRIPTION" };
    OrderBy OrderBy::ITEMID = { OrderBy::Value::ITEMID, "ITEMID" };
    OrderBy OrderBy::SETID = { OrderBy::Value::SETID, "SETID" };
    OrderBy OrderBy::WANTEDQTY = { OrderBy::Value::WANTEDQTY, "WANTEDQTY" };
    OrderBy OrderBy::SETQTY = { OrderBy::Value::SETQTY, "SETQTY" };
    OrderBy OrderBy::SHORTQTY = { OrderBy::Value::SHORTQTY, "SHORTQTY" };
    OrderBy OrderBy::SPAREQTY = { OrderBy::Value::SPAREQTY, "SPAREQTY" };
    OrderBy OrderBy::LASTCHECKED = { OrderBy::Value::LASTCHECKED, "LASTCHECKED" };

    OrderBy * OrderBy::FromString(std::string_view const & s)
    {
        if (s == OrderBy::COLOR.ToString()) return &OrderBy::COLOR;
        if (s == OrderBy::DESCRIPTION.ToString()) return &OrderBy::DESCRIPTION;
        if (s == OrderBy::ITEMID.ToString()) return &OrderBy::ITEMID;
        if (s == OrderBy::SETID.ToString()) return &OrderBy::SETID;
        if (s == OrderBy::WANTEDQTY.ToString()) return &OrderBy::WANTEDQTY;
        if (s == OrderBy::SETQTY.ToString()) return &OrderBy::SETQTY;
        if (s == OrderBy::SHORTQTY.ToString()) return &OrderBy::SHORTQTY;
        if (s == OrderBy::SPAREQTY.ToString()) return &OrderBy::SPAREQTY;
        if (s == OrderBy::LASTCHECKED.ToString()) return &OrderBy::LASTCHECKED;
        throw std::runtime_error("Unknown OrderBy keyword");
    }
}