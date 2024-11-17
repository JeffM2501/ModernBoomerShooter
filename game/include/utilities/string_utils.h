#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace StringUtils
{
    static inline std::vector<std::string> SplitString(std::string_view input, std::string_view delimiter)
    {
        size_t pos_start = 0, pos_end, delim_len = delimiter.length();
        std::string token;
        std::vector<std::string> results;

        while ((pos_end = input.find(delimiter, pos_start)) != std::string::npos)
        {
            token = input.substr(pos_start, pos_end - pos_start);
            pos_start = pos_end + delim_len;
            results.push_back(token);
        }

        results.push_back(std::string(input.substr(pos_start)));
        return results;
    }
}