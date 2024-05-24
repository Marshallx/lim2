#pragma once

#include <iostream>
#include <source_location>;
#include <sstream>

namespace mx
{
    namespace err
    {
        enum ErrorType
        {
            General,
            HResult
        };

        enum ErrorCode
        {
            Unknown,
            UnexpectedValue,
            UnexpectedEnd,
        };

        std::ostringstream formatError(std::string_view const & message, std::source_location const && source = {})
        {
            auto oss = std::ostringstream{};
            oss << "ERROR: " << source.file_name() << '#' << source.line() << ',' << source.column() << ": " << message << std::endl;
            return oss;
        }
    }
}

#define MX_THROW(message) { throw std::runtime_error(formatError(message).str()); }
