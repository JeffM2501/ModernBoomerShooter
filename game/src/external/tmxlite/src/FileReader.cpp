#include <tmxlite/FileReader.hpp>

#ifdef USE_EXTLIBS
#include <pugixml.hpp>
#else
#include "detail/pugixml.hpp"
#endif

namespace tmx
{
    static ReadFileDataCallback ReadCallback;
    static ReleaseFileDataCallback ReleaseCallback;

    static bool AllowRelativePaths = false;

    void SetTMXFileReadCallback(ReadFileDataCallback readCallback, ReleaseFileDataCallback releaseCallback)
    {
        ReadCallback = readCallback;
        ReleaseCallback = releaseCallback;
    }

    void ClearTMXFileReadCallback()
    {
        ReadCallback = nullptr;
        ReleaseCallback = nullptr;
    }

    pugi::xml_parse_result ReadXMLDocument(pugi::xml_document& document, const char* filePath)
    {
        if (ReadCallback == nullptr)
            return document.load_file(filePath);

        size_t size = 0;
        void* data = ReadCallback(filePath, size);

        if (data == nullptr || size == 0)
            return pugi::xml_parse_result();

        auto result = document.load_buffer(data, size);
        if (ReleaseCallback)
            ReleaseCallback(filePath, data);

        return result;
    }

    void SetAllowRelativePaths(bool allow)
    {
        AllowRelativePaths = allow;
    }

    bool GetAllowRelativePaths()
    {
        return AllowRelativePaths;
    }
}