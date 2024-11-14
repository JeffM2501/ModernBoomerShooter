#include "services/table_manager.h"
#include "services/resource_manager.h"
#include <unordered_map>

static std::vector<std::string> SplitString(std::string_view input, std::string_view delimiter)
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

bool Table::HasField(const std::string& key) const
{
    return find(key) != end();
}

std::string_view Table::GetField(const std::string& key) const
{
    auto itr = find(key);
    if (itr == end())
        return std::string();

    return itr->second;
}

const Table* Table::GetFieldAsTable(const std::string& key) const
{
    return TableManager::GetTable(GetField(key));
}

std::vector<std::string> Table::SplitField(const std::string& key, std::string_view deliminator) const
{
    auto itr = find(key);
    if (itr == end())
        return std::vector<std::string>();

    return SplitString(itr->second, deliminator);
}

namespace TableManager
{
    static std::hash<std::string_view> StringHasher;
    std::unordered_map<size_t, Table> LoadedTables;

    void Init()
    {

    }

    void Cleanup()
    {
        UnloadAll();
    }


    const Table* GetTable(std::string_view name)
    {
        size_t hash = StringHasher(name);
        
        auto itr = LoadedTables.find(hash);
        if (itr != LoadedTables.end())
            return &itr->second;

        auto resource = ResourceManager::OpenResource(name, true);

        if (resource == nullptr)
            return nullptr;

        auto lines = SplitString(std::string_view((char*)resource->DataBuffer, resource->DataSize), "\n");

        Table& table = LoadedTables.try_emplace(hash).first->second;

        for (auto& line : lines)
        {
            auto parts = SplitString(line, ";");
            if (!parts.empty())
            {
                table.insert_or_assign(parts[0], parts.size() > 1 ? parts[1] : std::string());
            }
        }

        ResourceManager::ReleaseResource(resource);

        return &table;
    }

    void UnloadAll()
    {
        LoadedTables.clear();
    }
};