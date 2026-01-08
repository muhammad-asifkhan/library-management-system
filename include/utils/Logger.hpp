#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <fstream>
#include <string>
#include <mutex>

class Logger {
private:
    static std::ofstream logFile;
    static std::mutex mtx;
public:
    static void init(const std::string& filename);
    static void info(const std::string& msg);
    static void warn(const std::string& msg);
    static void error(const std::string& msg);
    static void close();
};

#endif // LOGGER_HPP

