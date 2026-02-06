#include "RedisUtils.hpp"
#include "LuaScriptManager.hpp"
#include <cctype>
#include <cstddef>
#include <drogon/nosql/RedisResult.h>
#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <trantor/utils/Logger.h>

namespace {
    inline void trim_inplace(std::string& s) {
        size_t b = 0, e = s.size();
        while ( b < e && std::isspace(static_cast<unsigned char>(s[b])) ) ++b;
        while ( e > b && std::isspace(static_cast<unsigned char>(s[e-1])) ) --e;
        if (b != 0 || e != s.size()) s = s.substr(b, e-b);
    }

    inline void strip_quotes_inplace(std::string& s) {
        if (s.size() >=2 && s.front() == '"' && s.back() == '"') {
            s = s.substr(1, s.size() - 2);
        }
    }
    inline void unescape_basic_inplace(std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (size_t i = 0; i < s.size(); ++i) {
            if (s[i] == '\\' && i + 1 < s.size()) {
                char n = s[i+1];
                if (n == '"') { out.push_back('"'); ++i; continue; }
                if (n == '\\') { out.push_back('\\'); ++i; continue; }
            }
            out.push_back(s[i]);
        }
        s.swap(out);
    }
    inline void normalize_redis_message(std::string& s) {
        trim_inplace(s);
        unescape_basic_inplace(s);
        trim_inplace(s);
        strip_quotes_inplace(s);
        trim_inplace(s);
    }
}

std::atomic<bool> RedisUtils::scriptsLoaded_ = false;

RedisUtils& RedisUtils::instance()
{
    static RedisUtils inst;
    return inst;
}

void RedisUtils::Initialize(const std::string& luaScriptDir)
{
    if (Initialize_) {
        LOG_WARN << "RedisUtils already initialized";
        return;
    }
    LOG_INFO << "Initializing RedisUtils...";
    try {
        client_    = app().getRedisClient();
        subClient_ = app().getRedisClient();
        LOG_INFO << "Redis client obtained Successfully";
    }
    catch (const std::exception& e) {
        LOG_ERROR << "Failed to get Redis client: " << e.what();
        throw;
    }
    LOG_INFO << "Loading Lua script from: " << luaScriptDir;
    LuaScriptManager::instance().loadScriptsFromDirectory(luaScriptDir);

    preloadAllScripts([](bool success) {
        if (success) {
            LOG_INFO << "All Lua Script preloaded to Redis successfully";
        }
        else {
            LOG_WARN << "Some Lua scripts failed to preload, will load on demand";
        }
    });

    Initialize_ = true;
    LOG_INFO << "RedisUtils initialized successfully";
}

void RedisUtils::ensureInitialized()
{
    if (!Initialize_) {
        LOG_ERROR << "RedisUtils not initialized! call initialized first.";
        throw std::runtime_error("RedisUtils not initialized");
    }
}

void RedisUtils::set(const std::string& key, const std::string& value,
                     const std::function<void(bool)> callback, int expireSeconds)
{
    if (expireSeconds > 0) {
        client_->execCommandAsync(
            [callback](const RedisResult& r) { callback(r.asString() == "OK"); },
            [callback](const std::exception& e) {
                LOG_ERROR << "Redis SET error: " << e.what();
                callback(false);
            },
            "SET %s %s EX %d",
            key.c_str(),
            value.c_str(),
            expireSeconds);
    }
    else {
        client_->execCommandAsync(
            [callback](const RedisResult& r) { callback(r.asString() == "OK"); },
            [callback](const std::exception& e) {
                LOG_ERROR << "Redis SET error: " << e.what();
                callback(false);
            },
            "SET %s %s",
            key.c_str(),
            value.c_str());
    }
}

void RedisUtils::get(const std::string&                              key,
                     std::function<void(std::optional<std::string>)> callback)
{
    client_->execCommandAsync(
        [callback](const RedisResult& r) {
            if (r.type() == RedisResultType::kNil) {
                callback(std::nullopt);
            }
            else {
                callback(r.asString());
            }
        },
        [callback](const std::exception& e) {
            LOG_ERROR << "Redis Get error: " << e.what();
            callback(std::nullopt);
        },
        "GET %s");
}

void RedisUtils::del(const std::string& key, std::function<void(bool)> callback)
{
    client_->execCommandAsync([callback](const RedisResult& r) { callback(r.asInteger() > 0); },
                              [callback](const std::exception& e) {
                                  LOG_ERROR << "Redis del error: " << e.what();
                                  callback(false);
                              },
                              "DEL %s",
                              key.c_str());
}

void RedisUtils::hset(const std::string& key, const std::string& field, const std::string& value,
                      std::function<void(bool)> callback)
{
    client_->execCommandAsync([callback](const RedisResult& r) { callback(true); },
                              [callback](const std::exception& e) {
                                  LOG_ERROR << "Redis HSET error: " << e.what();
                                  callback(false);
                              },
                              "HSET %s %s %s",
                              key.c_str(),
                              field.c_str(),
                              value.c_str());
}

void RedisUtils::hget(const std::string& key, const std::string& field,
                      std::function<void(std::optional<std::string>)> callback)
{
    client_->execCommandAsync(
        [callback](const RedisResult& r) {
            if (r.type() == RedisResultType::kNil) {
                callback(std::nullopt);
            }
            else {
                callback(r.asString());
            }
        },
        [callback](const std::exception& e) {
            LOG_ERROR << "Redis HGET error: " << e.what();
            callback(std::nullopt);
        },
        "HGET %s %s",
        key.c_str(),
        field.c_str());
}

void RedisUtils::hgetall(const std::string&                                      key,
                         std::function<void(std::map<std::string, std::string>)> callback)
{
    client_->execCommandAsync(
        [callback](const RedisResult& r) {
            std::map<std::string, std::string> result;
            if (r.type() == RedisResultType::kArray) {
                auto arr = r.asArray();
                for (size_t i = 0; i < arr.size(); i += 2) {
                    if (i + 1 < arr.size()) {
                        result[arr[i].asString()] = arr[i + 1].asString();
                    }
                }
            }
            callback(result);
        },
        [callback](const std::exception& e) {
            LOG_ERROR << "Redis HGETALL error: " << e.what();
            callback({});
        },
        "HGETALL %s",
        key.c_str());
}

void RedisUtils::evalScript(const std::string& scriptName, const std::vector<std::string>& keys,
                            const std::vector<std::string>&         args,
                            std::function<void(const RedisResult&)> callback)
{
    std::string sha = LuaScriptManager::instance().getScriptSha(scriptName);
    if (sha.empty()) {
        LOG_WARN << "Script SHA not found for: " << scriptName << ", loading now...";
        loadScriptToRedis(
            scriptName,
            [this, scriptName, keys, args, callback](bool success, const std::string& newSha) {
                if (success) {
                    evalScript(scriptName, keys, args, callback);
                }
                else {
                    LOG_ERROR << "Failed to load script: " << scriptName;
                    callback(RedisResult(nullptr));
                }
            });
        return;
    }
    std::string cmd = "EVALSHA " + sha + " " + std::to_string(keys.size());
    for (const auto& key : keys) {
        cmd += " \"" + key + "\"";
    }
    for (const auto& arg : args) {
        cmd += " \"" + arg + "\"";
    }

    LOG_INFO << "exec command is : " << cmd;

    client_->execCommandAsync(
        [callback](const RedisResult& r) { callback(r); },
        [this, scriptName, keys, args, callback, sha](const std::exception& e) {
            std::string errorMsg = e.what();
            // check noscript error
            if (errorMsg.find("NOSCRIPT") != std::string::npos) {
                LOG_WARN << "Script not found in Redis (NOSCRIPT): " << scriptName
                         << ", reloading...";

                loadScriptToRedis(scriptName,
                                  [this, scriptName, keys, args, callback](
                                      bool success, const std::string& newsha) {
                                      if (success) {
                                          LOG_INFO << "Script reloaded successfully: "
                                                   << scriptName;
                                          evalScript(scriptName, keys, args, callback);
                                      }
                                      else {
                                          LOG_ERROR << "Failed to reload script: " << scriptName;
                                          callback(RedisResult(nullptr));
                                      }
                                  });
            }
            else {
                LOG_ERROR << "Redis EVALSHA error for " << scriptName << ": " << errorMsg;
                callback(RedisResult(nullptr));
            }
        },
        cmd.c_str());
}

void RedisUtils::validateTokenAndGetUser(const std::string& token, int expireSeconds,
                                         std::function<void(std::optional<std::string>)> callback)
{
    evalScript("validate_token",
               {token},
               {std::to_string(expireSeconds)},
               [callback](const RedisResult& r) {
                   if (r.type() == RedisResultType::kNil) {
                       callback(std::nullopt);
                   }
                   else {
                       callback(r.asString());
                   }
               });
}

void RedisUtils::checkRateLimit(const std::string& userId, int maxRequests, int windowsSeconds,
                                std::function<void(bool, int)> callback)
{
    auto        now = std::to_string(std::time(nullptr));
    std::string key = "ratelimit:" + userId;

    evalScript("rate_limit",
               {key},
               {now, std::to_string(windowsSeconds), std::to_string(maxRequests)},
               [callback](const RedisResult& r) {
                   if (r.type() == RedisResultType::kArray) {
                       auto arr       = r.asArray();
                       bool allowed   = arr[0].asInteger() == 1;
                       int  remaining = arr[1].asInteger();
                       callback(allowed, remaining);
                   }
               });
}

void RedisUtils::getNextId(const std::string& counterKey, std::function<void(long long)> callback)
{
    evalScript("get_next_id", {counterKey}, {}, [callback](const RedisResult& r) {
        callback(r.asInteger());
    });
}

void RedisUtils::acquireLock(const std::string& lockKey, const std::string& lockValue,
                             int expireSeconds, std::function<void(bool)> callback)
{
    evalScript("acquire_lock",
               {lockKey},
               {lockValue, std::to_string(expireSeconds)},
               [callback](const RedisResult& r) { callback(r.asInteger() == 1); });
}

void RedisUtils::releaseLock(const std::string& lockKey, const std::string& lockValue,
                             std::function<void(bool)> callback)
{
    evalScript("release_lock", {lockKey}, {lockValue}, [callback](const RedisResult& r) {
        callback(r.asInteger() == 1);
    });
}

void RedisUtils::atomicIncrement(const std::string& key, int increment, int maxValue,
                                 int expireSeconds, std::function<void(bool, long long)> callback)
{
    evalScript("atomic_increment",
               {key},
               {std::to_string(increment), std::to_string(maxValue), std::to_string(expireSeconds)},
               [callback](const RedisResult& r) {
                   if (r.type() == RedisResultType::kArray) {
                       auto      arr     = r.asArray();
                       bool      success = arr[0].asInteger() == 1;
                       long long value   = arr[1].asInteger();
                       callback(success, value);
                   }
                   else {
                       callback(false, 0);
                   }
               });
}

void RedisUtils::batchSetHash(const std::string&                        key,
                              const std::map<std::string, std::string>& fields, int expireSeconds,
                              std::function<void(int)> callback)
{
    std::vector<std::string> args;
    args.push_back(std::to_string(expireSeconds));
    LOG_INFO << "key: " << key << "  expireSeconds: " << expireSeconds;
    int count = 0;

    for (const auto& [field, value] : fields) {
        args.push_back(field);
        args.push_back(value);
        LOG_INFO << "count: " << count++ << "field" << field << "value: " << value;
    }

    evalScript("batch_set_hash", {key}, args, [callback](const RedisResult& r) {
        callback(r.asInteger());
    });
}

void RedisUtils::checkAndUpdate(const std::string& key, int minValue, int decrement,
                                std::function<void(bool, long long)> callback)
{
    evalScript("check_and_update",
               {key},
               {std::to_string(minValue), std::to_string(decrement)},
               [callback](const RedisResult& r) {
                   if (r.type() == RedisResultType::kArray) {
                       auto      arr     = r.asArray();
                       bool      success = arr[0].asInteger() == 1;
                       long long value   = arr[1].asInteger();
                       callback(success, value);
                   }
                   else {
                       callback(false, -1);
                   }
               });
}

void RedisUtils::saveToken(const std::string& token, const std::string& userId, int expireSeconds,
                           std::function<void(bool)> callback)
{
    std::string key = "token:" + token;

    std::map<std::string, std::string> tokenData;
    tokenData["user_id"]   = userId;
    tokenData["create_at"] = std::to_string(std::time(nullptr));

    batchSetHash(key, tokenData, expireSeconds, [callback, key](int result) {
        if (result > 0) {
            LOG_INFO << "Token saved to Redis: " << key;
            callback(true);
        }
        else {
            LOG_ERROR << "Failed to save token to Redis: " << key;
            callback(false);
        }
    });
}

void RedisUtils::getTokenInfo(const std::string&                              token,
                              std::function<void(std::optional<std::string>)> callback)
{
    std::string key = "token" + token;
    hget(key, "user_id", [callback](std::optional<std::string> userId) { callback(userId); });
}

void RedisUtils::deleteToken(const std::string& token, std::function<void(bool)> callback)
{
    std::string key = token;
    del(key, callback);
}

void RedisUtils::subscribeTokenExpiration(
    std::function<void(const std::string& expiredToken)> onExpired)
{
    // Redis 键过期事件的频道格式: __keyevent@<db>__:expired
    // 假设使用 db0
    std::string channel = "__keyevent@0__:expired";

    LOG_INFO << "Subscribing to Redis expiration events on channel: " << channel;

    // 正确的 Drogon Redis 订阅方式
    subscriber_ = subClient_->newSubscriber();

    subscriber_->subscribe(
        channel, [onExpired](const std::string& channel, const std::string& message) {
            // message 就是过期的键名
            LOG_INFO << "Redis key expired on channel " << channel << ": " << message;
            std::string tmp_msg = message;
            normalize_redis_message(tmp_msg);

            // 检查是否是 token 键
            if (tmp_msg.find("token:") == 0) {
                // 提取 token（去掉 "token:" 前缀）
                std::string token = tmp_msg.substr(6);
                LOG_INFO << "Token expired: " << token;

                // 调用回调处理过期的 token
                onExpired(token);
            }
            else {
                LOG_INFO << "message is:" << message;
            }
        });

    LOG_INFO << "Successfully subscribed to Redis expiration events";
}

void RedisUtils::preloadAllScripts(std::function<void(bool)> callback)
{
    std::vector<std::string> scriptNames = {"validate_token",
                                            "rate_limit",
                                            "acquire_lock",
                                            "release_lock",
                                            "get_next_id",
                                            "atomic_increment",
                                            "batch_set_hash",
                                            "check_and_update"};
    auto                     counter     = std::make_shared<std::atomic<int>>(scriptNames.size());
    auto                     allSuccess  = std::make_shared<std::atomic<bool>>(true);

    for (const std::string& name : scriptNames) {
        loadScriptToRedis(
            name, [name, counter, allSuccess, callback](bool success, const std::string& sha) {
                if (!success) {
                    LOG_ERROR << "Failed to preload script: " << name;
                    allSuccess->store(false);
                }
                else {
                    LOG_INFO << "Preloaded script: " << name << "-> " << sha;
                }
                if (--(*counter) == 0) {
                    scriptsLoaded_.store(true);
                    callback(allSuccess->load());
                }
            });
    }
}

void RedisUtils::loadScriptToRedis(const std::string&                            scriptName,
                                   std::function<void(bool, const std::string&)> callback)
{
    std::string script = LuaScriptManager::instance().getScript(scriptName);
    if (script.empty()) {
        LOG_ERROR << "Script not found in manager: " << scriptName;
        callback(false, "");
        return;
    }

    client_->execCommandAsync(
        [scriptName, callback](const RedisResult& r) {
            if (r.type() == RedisResultType::kString) {
                std::string sha = r.asString();
                LuaScriptManager::instance().setScriptSha(scriptName, sha);
                callback(true, sha);
            }
            else {
                LOG_ERROR << "SCRIPT LOAD returned unexcepted type for: " << scriptName;
                callback(false, "");
            }
        },
        [scriptName, callback](const std::exception& e) {
            LOG_ERROR << "Failed to load script " << scriptName << ": " << e.what();
            callback(false, "");
        },
        "SCRIPT LOAD %s",
        script.c_str());
}
