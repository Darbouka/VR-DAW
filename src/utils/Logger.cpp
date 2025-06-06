#include "Logger.hpp"
#include <iostream>
#include <ctime>

namespace VR_DAW {

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger()
    : currentLevel(LogLevel::Info)
    , consoleOutput(true)
{
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

void Logger::setLogFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (logFile.is_open()) {
        logFile.close();
    }
    
    logFile.open(filename, std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file: " << filename << std::endl;
    }
}

void Logger::setLogLevel(LogLevel level) {
    currentLevel = level;
}

void Logger::enableConsoleOutput(bool enable) {
    consoleOutput = enable;
}

std::string Logger::formatMessage(const std::string& message) {
    return message;
}

std::string Logger::createLogEntry(LogLevel level, const std::string& message) {
    return getTimestamp() + " [" + getLevelString(level) + "] " + message;
}

std::string Logger::getLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO";
        case LogLevel::Warning: return "WARNING";
        case LogLevel::Error:   return "ERROR";
        case LogLevel::Fatal:   return "FATAL";
        default:               return "UNKNOWN";
    }
}

std::string Logger::getColorCode(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:   return "\033[36m"; // Cyan
        case LogLevel::Info:    return "\033[32m"; // Green
        case LogLevel::Warning: return "\033[33m"; // Yellow
        case LogLevel::Error:   return "\033[31m"; // Red
        case LogLevel::Fatal:   return "\033[35m"; // Magenta
        default:               return "\033[0m";  // Reset
    }
}

std::string Logger::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

} // namespace VR_DAW 