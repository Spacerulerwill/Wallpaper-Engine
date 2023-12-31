#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <core/Application.hpp>
#include <system_error>
#include <util/Log.hpp>

int main() {
    Log::Init();
    Application app;
    try {
        app.Run();
        return EXIT_SUCCESS;
    }
    catch (const std::system_error& e) {
        LOG_CRITICAL(e.what());
        app.Cleanup();
        return EXIT_FAILURE;
    }
    catch (const std::runtime_error& e) {
        LOG_CRITICAL(e.what());
        app.Cleanup();
        return EXIT_FAILURE;
    }
}

