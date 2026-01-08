#include "../../include/utils/Logger.hpp"
#include <iostream>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>
std::ofstream Logger::logFile;
std::mutex Logger::mtx;

namespace {
std::string timestampNow() {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto t = system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void ensureParentDir(const std::string& filename) {
    namespace fs = std::filesystem;
    std::error_code ec;
    fs::path p(filename);
    if (!p.parent_path().empty()) fs::create_directories(p.parent_path(), ec);
}
} // namespace

void Logger::init(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mtx);
    ensureParentDir(filename);
    logFile.open(filename, std::ios::app);
    if (logFile) {
        logFile << "----- Logger started @ " << timestampNow() << " -----" << std::endl;
    }
}
void Logger::info(const std::string& msg) {
    std::lock_guard<std::mutex> lock(mtx);
    if (logFile) logFile << "[" << timestampNow() << "][INFO] " << msg << std::endl;
}
void Logger::warn(const std::string& msg) {
    std::lock_guard<std::mutex> lock(mtx);
    if (logFile) logFile << "[" << timestampNow() << "][WARN] " << msg << std::endl;
}
void Logger::error(const std::string& msg) {
    std::lock_guard<std::mutex> lock(mtx);
    if (logFile) logFile << "[" << timestampNow() << "][ERROR] " << msg << std::endl;
}
void Logger::close() {
    std::lock_guard<std::mutex> lock(mtx);
    if (logFile) {
        logFile << "----- Logger closed @ " << timestampNow() << " -----" << std::endl;
        logFile.close();
    }
}

