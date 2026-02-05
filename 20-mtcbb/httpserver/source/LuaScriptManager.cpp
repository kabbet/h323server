#include "LuaScriptManager.hpp"
#include <fstream>
#include <sstream>
#include <trantor/utils/Logger.h>

LuaScriptManager& LuaScriptManager::instance()
{
    static LuaScriptManager inst;
    return inst;
}

bool LuaScriptManager::loadScript(const std::string& name, const std::string& filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open()) {
        LOG_ERROR << "Failed to open Lua script file: " << filepath;
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    script_[name] = buffer.str();

    LOG_INFO << "Loaded lua script: " << name << " from " << filepath;
    return true;
}

bool LuaScriptManager::loadScriptsFromDirectory(const std::string& dirpath)
{
    std::vector<std::pair<std::string, std::string>> scriptFiles = {
        {"validate_token", dirpath + "/validate_token.lua"},
        {"rate_limit", dirpath + "/rate_limit.lua"},
        {"acquire_lock", dirpath + "/acquire_lock.lua"},
        {"release_lock", dirpath + "/release_lock.lua"},
        {"get_next_id", dirpath + "/get_next_id.lua"},
        {"atomic_increment", dirpath + "/atomic_increment.lua"},
        {"batch_set_hash", dirpath +  "/batch_set_hash.lua"},
        {"check_and_update", dirpath + "/check_and_update.lua"}
    };
    bool allSuccess = true;
    for (const auto& [name, path] : scriptFiles) {
        if (!loadScript(name, path)) {
            allSuccess = false;
        }
    }
    return allSuccess;
}

std::string LuaScriptManager::getScript(const std::string& name) const
{
    auto it = script_.find(name);
    if (it != script_.end()) {
        return it->second;
    }
    LOG_ERROR << "Script not found: " << name;
    return "";
}

bool LuaScriptManager::hasScript(const std::string& name) const
{
    return script_.find(name) != script_.end();
}

void LuaScriptManager::setScriptSha(const std::string& name, const std::string& sha)
{
    scriptShas_[name] = sha;
}

std::string LuaScriptManager::getScriptSha(const std::string& name) const
{
    auto it = scriptShas_.find(name);
    if (it != scriptShas_.end()) {
        return it->second;
    }
    return "";
}

