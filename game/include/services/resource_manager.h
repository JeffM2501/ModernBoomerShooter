#pragma once

#include <string_view>
#include <memory>

struct ResoureInfo
{
    size_t NameHash = 0;

    uint8_t* DataBuffer = nullptr;
    size_t DataSize = 0;

    ~ResoureInfo();
};

namespace ResourceManager
{
    void Init(std::string_view rootFolder);
    void Cleanup();

    std::shared_ptr<ResoureInfo> OpenResource(std::string_view filePath);

    void ReleaseResource(std::shared_ptr<ResoureInfo> resource);
    void ReleaseResource(const char* resourceName);
};