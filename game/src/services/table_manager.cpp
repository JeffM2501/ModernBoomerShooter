#include "services/table_manager.h"
#include "services/resource_manager.h"
#include <unordered_map>

#include "utilities/string_utils.h"

bool Table::HasField(const std::string& key) const
{
    return find(key) != end();
}

constexpr char EmptyField[] = "";

std::string_view Table::GetField(const std::string& key) const
{
    auto itr = find(key);
    if (itr == end())
        return EmptyField;

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

    return StringUtils::SplitString(itr->second, deliminator);
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

        auto lines = StringUtils::SplitString(std::string_view((char*)resource->DataBuffer, resource->DataSize), "\n");

        Table& table = LoadedTables.try_emplace(hash).first->second;

        for (auto& line : lines)
        {
            auto parts = StringUtils::SplitString(line, ";");
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