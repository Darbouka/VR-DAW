#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <memory>

namespace VR_DAW {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};

class Logger {
public:
    static Logger& getInstance();

    void setLogFile(const std::string& filename);
    void setLogLevel(LogLevel level);
    void enableConsoleOutput(bool enable);

    template<typename... Args>
    void log(LogLevel level, const std::string& message, Args&&... args) {
        if (level < currentLevel) return;

        std::string formattedMessage = formatMessage(message, std::forward<Args>(args)...);
        std::string logEntry = createLogEntry(level, formattedMessage);

        std::lock_guard<std::mutex> lock(mutex);
        
        if (logFile.is_open()) {
            logFile << logEntry << std::endl;
            logFile.flush();
        }

        if (consoleOutput) {
            std::cout << getColorCode(level) << logEntry << "\033[0m" << std::endl;
        }
    }

    template<typename... Args>
    void debug(const std::string& message, Args&&... args) {
        log(LogLevel::Debug, message, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void info(const std::string& message, Args&&... args) {
        log(LogLevel::Info, message, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void warning(const std::string& message, Args&&... args) {
        log(LogLevel::Warning, message, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void error(const std::string& message, Args&&... args) {
        log(LogLevel::Error, message, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void fatal(const std::string& message, Args&&... args) {
        log(LogLevel::Fatal, message, std::forward<Args>(args)...);
    }

private:
    Logger();
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::string formatMessage(const std::string& message);
    template<typename T, typename... Args>
    std::string formatMessage(const std::string& message, T&& value, Args&&... args) {
        size_t pos = message.find("{}");
        if (pos == std::string::npos) return message;

        std::stringstream ss;
        ss << std::forward<T>(value);
        return message.substr(0, pos) + ss.str() + 
               formatMessage(message.substr(pos + 2), std::forward<Args>(args)...);
    }

    std::string createLogEntry(LogLevel level, const std::string& message);
    std::string getLevelString(LogLevel level);
    std::string getColorCode(LogLevel level);
    std::string getTimestamp();

    std::ofstream logFile;
    std::mutex mutex;
    LogLevel currentLevel;
    bool consoleOutput;
};

// Makros für einfache Verwendung
#define LOG_DEBUG(...) VR_DAW::Logger::getInstance().debug(__VA_ARGS__)
#define LOG_INFO(...) VR_DAW::Logger::getInstance().info(__VA_ARGS__)
#define LOG_WARNING(...) VR_DAW::Logger::getInstance().warning(__VA_ARGS__)
#define LOG_ERROR(...) VR_DAW::Logger::getInstance().error(__VA_ARGS__)
#define LOG_FATAL(...) VR_DAW::Logger::getInstance().fatal(__VA_ARGS__)

} // namespace VR_DAW 