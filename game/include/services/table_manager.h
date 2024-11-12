#pragma once

#include <string_view>

#include <vector>
#include<map>
#include<string>

static constexpr char BootstrapTable[] = "bootstrap.table";

class Table : public std::map<std::string, std::string>
{
public:
    bool HasField(const std::string& key) const;
    std::string_view GetField(const std::string& key) const;

    std::vector<std::string> SplitField(const std::string& key, std::string_view deliminator) const;
};

namespace TableManager
{
    void Init();
    void Cleanup();

    const Table* GetTable(std::string_view name);
    void UnloadAll();
};