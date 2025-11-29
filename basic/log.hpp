#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>

#define LOG(level) Logger(level, __FILE__, __LINE__)

enum LEVEL { DEBUG, INFO, WARNING, ERROR };
enum class OutPutType { STDIO, LOGFILE };

class Logger {
 public:
  static bool initLog(LEVEL level, OutPutType type,
                      std::string path = "log.txt") {
    Logger::type = type;
    GlobalLevel = level;
    Logger::path = path;
    return InitOutPut();
  }

  static void releaseLog(
      std::optional<bool> removeLogFile = std::nullopt /*remove log file*/) {
    if (LogFile.is_open()) {
      LogFile.close();
    }

    // have error will not delete log file
    if (removeLogFile == std::nullopt && haveError) {
      return;
    } else if (removeLogFile.has_value() && !*removeLogFile) {
      return;
    }

    // delete log file here
    if (std::filesystem::exists(path)) {
      std::error_code ec;
      bool status = std::filesystem::remove(path, ec);
      if (ec) {
        std::cerr << logerr << ec.message();
      }
    }
  }

  Logger(LEVEL level, const char* file, int line)
      : Level(level), file(file), line(line) {
    switch (level) {
      case DEBUG:
        prefix = "DEBUG";
        color = "\033[34m";  // Blue
        break;
      case INFO:
        prefix = "INFO";
        color = "\033[32m";  // Green
        break;
      case WARNING:
        prefix = "WARNING";
        color = "\033[33m";  // Yellow
        break;
      case ERROR:
        prefix = "ERROR";
        color = "\033[31m";  // Red
        break;
    }

    if (level == ERROR) {
      haveError = true;
    }
  }

  void rateLimitCheck() {
    auto now = std::chrono::steady_clock::now();

    // every second will reset suppress logs
    if (std::chrono::duration_cast<std::chrono::seconds>(now - lastSecond)
            .count() >= 1) {
      logCount = 0;
      lastSecond = now;
      suppressLogs = false;
    }

    logCount++;

    if (logCount > 50) {
      suppressLogs = true;
    }
  }

  void limitSize() {
    // limit log file size, over 50 MB auto cut
    if (type == OutPutType::LOGFILE && LogFile.is_open()) {
      LogFile.flush();
      auto size = LogFile.tellp();
      // const std::streamoff MAX_SIZE = 10 * 1024 * 1024;  // 10MB
      const std::streamoff MAX_SIZE = 10 * 1024;  // 50KB

      if (size >= MAX_SIZE) {
        LogFile.close();
        LogFile.open(path, std::ios::out | std::ios::trunc);
        if (LogFile.is_open()) {
          output = &LogFile;
          (*output) << logerr
                    << "Log file truncated because it exceeded 50MB\n";
        } else {
          type = OutPutType::STDIO;
          output = &std::cerr;
          std::cerr << logerr << "Failed to truncate and reopen log file\n";
        }
      }
    }
  }

  void writeLog() {
    // debug option only used in debug
    // so it will always output to cerr
    if (Level == DEBUG) {
#ifndef NDEBUG
      std::cerr << color << "[" << file << ":" << line << "|" << prefix << "] "
                << "\033[0m" << os.str() << "\n";
#endif
    } else if (Level >= GlobalLevel) {
      if (type == OutPutType::STDIO) {
        (*output) << color << "[" << prefix << "] " << "\033[0m" << os.str()
                  << "\n";
      } else {
        (*output) << "[" << prefix << "] " << os.str() << "\n";
      }
    }
  }

  ~Logger() {
    rateLimitCheck();
    if (!suppressLogs) {
      writeLog();
    }

    if (type == OutPutType::LOGFILE) {
      limitSize();
    }
  }

  template <typename T>
  Logger& operator<<(const T& info) {
    os << info;
    return *this;
  }

 private:
  static inline LEVEL GlobalLevel = INFO;
  static inline OutPutType type = OutPutType::STDIO;
  static inline std::ostream* output = &std::cerr;
  static inline std::ofstream LogFile;
  static inline std::string path;
  static inline std::string_view logerr = "[LOG ERROR] ";
  std::ostringstream os;
  const char* file;
  int line;
  LEVEL Level;
  std::string prefix, color;
  static inline bool haveError = false;
  static inline int logCount = 0;
  static inline bool suppressLogs = false;
  static inline auto lastSecond = std::chrono::steady_clock::now();

  static std::string makeUniquePath(const std::string& path, int counter = 0) {
    auto p = std::filesystem::path(path);
    std::string stem = p.stem().string();
    std::string ext = p.extension().string();
    std::string parent = p.parent_path().string();

    std::ostringstream oss;
    oss << stem << "_";
    if (counter > 0) oss << counter;
    oss << ext;

    std::filesystem::path newPath =
        parent.empty() ? oss.str() : (parent + "/" + oss.str());

    if (!std::filesystem::exists(newPath)) {
      return newPath.string();
    } else {
      return makeUniquePath(path, counter + 1);
    }
  }

  static bool InitOutPut() {
    if (type == OutPutType::STDIO) {
      output = &std::cerr;
      return true;
    }

    if (std::filesystem::exists(path)) {
      path = makeUniquePath(path);
    }

    std::filesystem::remove(path);

    std::string parent_path =
        std::filesystem::path(path).parent_path().string();
    if (!parent_path.empty() && !std::filesystem::exists(parent_path)) {
      std::filesystem::create_directories(parent_path);
    }

    LogFile.open(path, std::ios::app);
    if (LogFile.is_open()) {
      output = &LogFile;
      return true;
    } else {
      type = OutPutType::STDIO;
      output = &std::cerr;
      std::cerr << logerr << "Open " << path << "failed.";
      return false;
    }
  }
};
