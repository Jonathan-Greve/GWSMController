#include "pch.h"

int get_first_integer_in_string(const std::string& input)
{
    // Find the first digit in the string
    size_t pos = input.find_first_of("0123456789");
    if (pos == std::string::npos)
    {
        // No digits were found, so we cannot extract an integer
        return -1;
    }

    // Extract the integer by starting at the first digit and continuing until we reach a non-digit character
    std::string integer_str = "";
    while (pos < input.size() && std::isdigit(input[pos]))
    {
        integer_str += input[pos];
        ++pos;
    }

    // Convert the string to an integer
    int integer = std::stoi(integer_str);

    return integer;
}
