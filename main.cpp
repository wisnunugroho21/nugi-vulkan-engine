#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "src/app/app.hpp"

int main(int argc, char const *argv[])
{
    nugiEngine::EngineApp app{};

    try {
        app.run();
    } catch(const std::exception &e) {
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
