#pragma once

#include <exception>
#include <filesystem>
#include <string_view>

namespace mxi
{
    enum LogLevel
    {
        Debug, Info, Warn, Error, Fatal
    };

    void log(LogLevel const & level, std::string_view const & message);
    void init_logger(std::filesystem::path const & logfile);
}

#define MX_LOG_DEBUG(message) mxi::log(mxi::LogLevel::Debug, message)
#define MX_LOG_INFO(message) mxi::log(mxi::LogLevel::Info, message)
#define MX_LOG_WARN(message) mxi::log(mxi::LogLevel::Warn, message)
#define MX_LOG_ERROR(message) mxi::log(mxi::LogLevel::Error, message)
#define MX_LOG_FATAL(message) mxi::log(mxi::LogLevel::Fatal, message)

#define MX_THROW(message) throw std::runtime_error(message)
