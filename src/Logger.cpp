#include "Logger.h"

#include <sstream>
#include <iomanip>

std::string getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
    localtime_r(&now_c, &tm_buf);

    std::ostringstream timeStream;
    timeStream << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
    return timeStream.str();
}

std::string getLogTimestamp() {
    return "[" + getCurrentTime() + "] ";
}

