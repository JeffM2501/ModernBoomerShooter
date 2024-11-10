#pragma once

#include <tmxlite/Config.hpp>
#include <functional>

namespace pugi
{
    class xml_document;
    struct xml_parse_result;
}

namespace tmx
{
    using ReadFileDataCallback = std::function<void* (const char*, size_t&)>;
    using ReleaseFileDataCallback = std::function<void(const char*, void*)>;

    void SetTMXFileReadCallback(ReadFileDataCallback readCallback, ReleaseFileDataCallback releaseCallback);
    void ClearTMXFileReadCallback();

    pugi::xml_parse_result ReadXMLDocument(pugi::xml_document& document, const char* filePath);

    void SetAllowRelativePaths(bool allow);
    bool GetAllowRelativePaths();
}