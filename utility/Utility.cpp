#include "Utility.hpp"
#include <fstream>

///////////////////////////////////////////////////////////////////////////////
std::vector<uint32_t> readFile(const char* filename)
{
    std::vector<uint32_t> result;
    uint32_t buffer[4096];

    auto handle = std::fopen(filename, "rb");

    for (;;)
    {
        const auto bytesRead = std::fread(buffer, 1, sizeof(buffer), handle);

        result.insert(result.end(), buffer, buffer + bytesRead);

        if (bytesRead < sizeof(buffer))
        {
            break;
        }
    }

    std::fclose(handle);

    return result;
}