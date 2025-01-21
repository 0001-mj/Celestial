#pragma once
#include "Memory.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdarg>
#include <string>
#include <memory>

enum class ELogVerbosity
{
    Log,
    VeryVerbose,
    Verbose,
    Display,
    Warning,
    Error,
    Fatal
};

class LogCategory
{
public:
    constexpr LogCategory(const char* InName) : Name(InName), DefaultVerbosity(ELogVerbosity::Log) {}
    constexpr LogCategory(const char* InName, ELogVerbosity InDefaultVerbosity)
        : Name(InName), DefaultVerbosity(InDefaultVerbosity) {}

    const char* GetName() const { return Name; }
    ELogVerbosity GetDefaultVerbosity() const { return DefaultVerbosity; }

private:
    const char* Name;
    ELogVerbosity DefaultVerbosity;
};

#define DECLARE_LOG_CATEGORY_EXTERN(CategoryName, DefaultVerbosity) \
    extern LogCategory CategoryName;

#define DEFINE_LOG_CATEGORY(CategoryName) \
    LogCategory CategoryName(#CategoryName);

#define DEFINE_LOG_CATEGORY_WITH_VERBOSITY(CategoryName, DefaultVerbosity) \
    LogCategory CategoryName(#CategoryName, DefaultVerbosity);

class Logger
{
public:
    static std::unique_ptr<Logger>& Get()
    {
        static std::unique_ptr<Logger> Instance = std::make_unique<Logger>();
        return Instance;
    }

    void SetLogFile(const std::string& FilePath)
    {
        if (LogFile.is_open())
        {
            LogFile.close();
        }
        LogFile.open(FilePath, std::ios::out | std::ios::app);

        if (!LogFile)
        {
            std::cerr << "[Error] Failed to open log file: " << FilePath << std::endl;
        }
    }

    template <typename... Args>
    void LogMessage(const LogCategory& Category, ELogVerbosity Verbosity, const char* Format, Args... args)
    {
        char Buffer[1024];
        int Written = snprintf(Buffer, sizeof(Buffer), Format, args...);
        if (Written < 0 || Written >= sizeof(Buffer))
        {
            std::cerr << "[Error] Failed to format log message." << std::endl;
            return;
        }

        std::ostringstream Message;
        Message << Category.GetName() << ": " << ToString(Verbosity) << ": " << Buffer;

        std::cout << Message.str() << std::endl;

        if (LogFile.is_open())
        {
            LogFile << Message.str() << std::endl;
        }

        if (Verbosity == ELogVerbosity::Fatal)
        {
            std::cerr << "Fatal error logged. Terminating application..." << std::endl;
            std::abort();
        }
    }

public:
    Logger() = default;

    const char* ToString(ELogVerbosity Verbosity) const
    {
        switch (Verbosity)
        {
        case ELogVerbosity::VeryVerbose: return "VeryVerbose";
        case ELogVerbosity::Verbose: return "Verbose";
        case ELogVerbosity::Display: return "Display";
        case ELogVerbosity::Warning: return "Warning";
        case ELogVerbosity::Error: return "Error";
        case ELogVerbosity::Fatal: return "Fatal";
        case ELogVerbosity::Log: return "Log";
        default: return "Unknown";
        }
    }

    std::ofstream LogFile;
};

#define LOG(Category, Verbosity, Format, ...) \
    Logger::Get()->LogMessage(Category, Verbosity, Format, ##__VA_ARGS__)

#define CLOG(Condition, Category, Verbosity, Format, ...) \
    if (Condition) Logger::Get()->LogMessage(Category, Verbosity, Format, ##__VA_ARGS__)

constexpr auto VeryVerbose = ELogVerbosity::VeryVerbose;
constexpr auto Verbose = ELogVerbosity::Verbose;
constexpr auto Display = ELogVerbosity::Display;
constexpr auto Warning = ELogVerbosity::Warning;
constexpr auto Error = ELogVerbosity::Error;
constexpr auto Fatal = ELogVerbosity::Fatal;
constexpr auto Log = ELogVerbosity::Log;

DEFINE_LOG_CATEGORY(LogNet);
DEFINE_LOG_CATEGORY(LogTemp);
DEFINE_LOG_CATEGORY(LogWorld);
DEFINE_LOG_CATEGORY(LogMemory);
DEFINE_LOG_CATEGORY(LogGameMode);
DEFINE_LOG_CATEGORY(LogNetTraffic);