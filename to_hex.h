#ifndef TO_HEX_H
#define TO_HEX_H

#include <string>
#include <iomanip>
#include <sstream>

template<typename T>
std::string to_hex(T value, size_t width=sizeof(T)/2, bool prefix=true)
{
    std::stringstream ss;

    ss << std::setfill('0') << std::setw(width) << std::hex << (value | 0);

    if (prefix)
        return "0x"+ss.str();

    return ss.str();
}


#endif /* TO_HEX_H */
