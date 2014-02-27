#ifndef ESO_GLOBAL_CONFIG_TYPES
#define ESO_GLOBAL_CONFIG_TYPES

#include <string>
#include <vector>

typedef std::vector<unsigned char> uchar_vec;

/**
 * Converts a uchar_vec to a std::string.
 */
std::string to_string(uchar_vec u)
{
    return std::string{u.begin(), u.end()};
}

#endif
