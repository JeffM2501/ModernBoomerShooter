#include "services/resource_manager.h"

#include <unordered_map>
#include <string>
#include <string.h>

#include "raylib.h"

ResoureInfo::~ResoureInfo()
{
    UnloadFileData(DataBuffer);
}

namespace ResourceManager
{
    static std::hash<std::string_view> StringHasher;

    using ResourceMap = std::unordered_map<size_t, std::shared_ptr<ResoureInfo>>;
    ResourceMap OpenResources;

    bool SearchAndSetResourceDir(const char* folderName)
    {
        // check the working dir
        if (DirectoryExists(folderName))
        {
            ChangeDirectory(TextFormat("%s/%s", GetWorkingDirectory(), folderName));
            return true;
        }

        const char* appDir = GetApplicationDirectory();

        // check the applicationDir
        const char* dir = TextFormat("%s%s", appDir, folderName);
        if (DirectoryExists(dir))
        {
            ChangeDirectory(dir);
            return true;
        }

        // check one up from the app dir
        dir = TextFormat("%s../%s", appDir, folderName);
        if (DirectoryExists(dir))
        {
            ChangeDirectory(dir);
            return true;
        }

        // check two up from the app dir
        dir = TextFormat("%s../../%s", appDir, folderName);
        if (DirectoryExists(dir))
        {
            ChangeDirectory(dir);
            return true;
        }

        // check three up from the app dir
        dir = TextFormat("%s../../../%s", appDir, folderName);
        if (DirectoryExists(dir))
        {
            ChangeDirectory(dir);
            return true;
        }

        return false;
    }

    void Init(std::string_view rootFolder)
    {
        SearchAndSetResourceDir(rootFolder.data());

        // TODO, handle reading from packages here, like reading an archive
    }

    void Cleanup()
    {
        OpenResources.clear();
    }

    std::shared_ptr<ResoureInfo> OpenResource(std::string_view filePath, bool asText)
    {
        size_t pathHash = StringHasher(filePath);

        auto itr = OpenResources.find(pathHash);
        if (itr != OpenResources.end())
            return itr->second;

        int size = 0;
        uint8_t* buffer = nullptr;
        
        if (asText)
        {
            buffer = (uint8_t*)LoadFileText(filePath.data());
            if (buffer)
                size = (int)strlen((char*)buffer);
        }
        else
        {
            buffer = LoadFileData(filePath.data(), &size);
        }

        if (!buffer)
            return nullptr;

        std::shared_ptr<ResoureInfo> file = std::make_shared<ResoureInfo>();

        file->NameHash = pathHash;
        file->DataBuffer = buffer;
        file->DataSize = size;

        OpenResources.insert_or_assign(pathHash, file);
        return file;
    }

    void ReleaseResource(std::shared_ptr<ResoureInfo> resource)
    {
        if (!resource)
            return;

        auto itr = OpenResources.find(resource->NameHash);
        if (itr != OpenResources.end())
            OpenResources.erase(itr);
    }

    void ReleaseResource(const char* resourceName)
    {
        auto itr = OpenResources.find(StringHasher(resourceName));
        if (itr != OpenResources.end())
            OpenResources.erase(itr);
    }

    void ReleaseResourceByData(void* resourceData)
    {
        for (ResourceMap::iterator itr = OpenResources.begin(); itr != OpenResources.end(); itr++)
        {
            if (itr->second->DataBuffer == resourceData)
            {
                OpenResources.erase(itr);
                return;
            }
        }
    }
};