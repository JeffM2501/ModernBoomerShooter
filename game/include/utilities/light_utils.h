#pragma once

#include <string_view>
#include <vector>

namespace LightUtils
{
    inline std::vector<float> ParseLightSequence(const std::string_view text)
    {
        std::vector<float> results;

        float range = int('m') - int('a');
        for (const char c : text)
        {
            results.push_back((c - 'a') / range);
        }
        return results;
    }
}