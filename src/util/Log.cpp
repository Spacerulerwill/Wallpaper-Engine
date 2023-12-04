#include <spdlog/sinks/stdout_color_sinks.h>
#include <util/Log.hpp>

std::shared_ptr<spdlog::logger> Log::sLogger;

void Log::Init() {
    spdlog::set_pattern("%^[%T] %n: %v%$");
    sLogger = spdlog::stdout_color_mt("Wallpaper Engine");
    sLogger->set_level(spdlog::level::trace);

    LOG_INFO("Logger initialised");
}