#ifndef LOG_H
#define LOG_H

#include <memory>
#include <spdlog/spdlog.h>

#define DEBUG_MODE

class Log {
public:
    static void Init();

    inline static std::shared_ptr<spdlog::logger>& GetLogger() { return s_Logger; }

private:

    static std::shared_ptr<spdlog::logger> s_Logger;

};

// debug messages
#ifdef DEBUG_MODE

#define LOG_CRITICAL(...)    Log::GetLogger()->critical(__VA_ARGS__)
#define LOG_ERROR(...)		Log::GetLogger()->error(__VA_ARGS__)
#define LOG_WARNING(...)		Log::GetLogger()->warn(__VA_ARGS__)
#define LOG_INFO(...)		Log::GetLogger()->info(__VA_ARGS__)
#define LOG_TRACE(...)		Log::GetLogger()->trace(__VA_ARGS__)
#else
#define LOG_CRITICAL 
#define LOG_ERROR 
#define LOG_WARNING 
#define LOG_INFO 
#define LOG_TRACE 
#endif

#endif // !LOG_H