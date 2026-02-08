#ifndef REDISUTILS_HPP
#define REDISUTILS_HPP

#include "LuaScriptManager.hpp"
#include "drogon/drogon.h"
#include <atomic>
#include <drogon/nosql/RedisClient.h>
#include <drogon/nosql/RedisResult.h>
#include <drogon/nosql/RedisSubscriber.h>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <trantor/utils/Logger.h>

using namespace drogon;
using namespace drogon::nosql;

class RedisUtils
{
public:
    static RedisUtils& instance();
    void               Initialize(const std::string& luaScriptDir);

    // ============================basic characters operator ============================
    void set(const std::string& key, const std::string& value,
             const std::function<void(bool)> callback, int expireSeconds = 0);
    void get(const std::string& key, std::function<void(std::optional<std::string>)> callback);
    void del(const std::string& key, std::function<void(bool)> callback);

    // ============================Hash operator ============================
    void hset(const std::string& key, const std::string& field, const std::string& value,
              std::function<void(bool)> callback);
    void hget(const std::string& key, const std::string& field,
              std::function<void(std::optional<std::string>)> callback);
    void hgetall(const std::string&                                      key,
                 std::function<void(std::map<std::string, std::string>)> callback);

    // ============================ exec lua script ============================
    void evalScript(const std::string& scriptName, const std::vector<std::string>& keys,
                    const std::vector<std::string>&         args,
                    std::function<void(const RedisResult&)> callback);

    // ============================ senior exec ============================
    void validateTokenAndGetUser(const std::string& token, int expireSeconds,
                                 std::function<void(std::optional<std::string>)> callback);
    void checkRateLimit(const std::string& userId, int maxRequests, int windowSeconds,
                        std::function<void(bool, int)> callback);
    void getNextId(const std::string& counterKey, std::function<void(long long)> callback);
    void acquireLock(const std::string& lockKey, const std::string& lockValue, int expireSeconds,
                     std::function<void(bool)> callback);
    void releaseLock(const std::string& lockKey, const std::string& lockValue,
                     std::function<void(bool)> callback);
    void atomicIncrement(const std::string& key, int increment, int maxValue, int expireSeconds,
                         std::function<void(bool, long long)> callback);
    void batchSetHash(const std::string& key, const std::map<std::string, std::string>& fields,
                      int expireSeconds, std::function<void(int)> callback);
    void checkAndUpdate(const std::string& key, int minValue, int decrement,
                        std::function<void(bool, long long)> callback);

    // ============================ token operator ============================
    void saveToken(const std::string& token, const std::string& userId, int expireSeconds,
                   std::function<void(bool)> callback);
    void getTokenInfo(const std::string&                              token,
                      std::function<void(std::optional<std::string>)> callback);
    void deleteToken(const std::string& token, std::function<void(bool)> callback);
    void subscribeTokenExpiration(std::function<void(const std::string& expiredToken)> onExpired);


    // ============================ preload script ============================
    void preloadAllScripts(std::function<void(bool)> callback);
    void loadScriptToRedis(const std::string& scriptName, std::function<void(bool, const std::string&)> callback);
private:
    RedisUtils() = default;
    void ensureInitialized();

private:
    std::shared_ptr<RedisClient> client_;
    std::shared_ptr<RedisClient> subClient_;
    std::shared_ptr<RedisSubscriber> subscriber_;
    bool Initialize_ = false;

    // lua script status trace
    static std::atomic<bool> scriptsLoaded_;
    std::mutex scriptLoadMutex_;
};

#endif
