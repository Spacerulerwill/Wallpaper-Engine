#include "Log.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>

std::shared_ptr<spdlog::logger> Log::s_Logger;

void Log::Init() {
	spdlog::set_pattern("%^[%T] %n: %v%$");
	s_Logger = spdlog::stdout_color_mt("Wallpaper Engine");
	s_Logger->set_level(spdlog::level::trace);

	LOG_INFO("Logger initialised");
}