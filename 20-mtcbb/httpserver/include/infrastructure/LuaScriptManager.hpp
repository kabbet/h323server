#ifndef LUASCRIPTMANAGER_HPP
#define LUASCRIPTMANAGER_HPP

#include <map>
#include <string>

class LuaScriptManager
{
public:
    static LuaScriptManager& instance();
    bool loadScript(const std::string& name, const std::string& filepath);
    bool loadScriptsFromDirectory(const std::string& dirpath);
    std::string getScript(const std::string& name) const;
    bool hasScript(const std::string& name) const;
    void setScriptSha(const std::string& name, const std::string& sha);
    std::string getScriptSha(const std::string& name) const;

private:
    LuaScriptManager() = default;
    std::map<std::string, std::string> script_;
    std::map<std::string, std::string> scriptShas_;
};

#endif
