#include <vector>
#include <cstdint>

///////////////////////////////////////////////////////////////////////////////
template <typename T>
T RoundToNextMultiple(const T a, const T multiple)
{
    return ((a + multiple - 1) / multiple) * multiple;
}

std::vector<char> readFile(const std::string& filename);