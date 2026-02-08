#ifndef IREDISCLIENT_HPP
#define IREDISCLIENT_HPP

#include <functional>
#include <optional>
#include <string>
#include <map>
#include <vector>
#include <drogon/nosql/RedisResult.h>

using namespace drogon;
using namespace drogon::nosql;

namespace interfaces {

class IRedisClient {
public:
    virtual ~IRedisClient() = default;

    // 基本操作
    virtual void set(
        const std::string& key,
        const std::string& value,
        std::function<void(bool)> callback,
        int expireSeconds = 0) = 0;

    virtual void get(
        const std::string& key,
        std::function<void(std::optional<std::string>)> callback) = 0;

    virtual void del(
        const std::string& key,
        std::function<void(bool)> callback) = 0;

    // Hash 操作
    virtual void hset(
        const std::string& key,
        const std::string& field,
        const std::string& value,
        std::function<void(bool)> callback) = 0;

    virtual void hget(
        const std::string& key,
        const std::string& field,
        std::function<void(std::optional<std::string>)> callback) = 0;

    virtual void hgetall(
        const std::string& key,
        std::function<void(std::map<std::string, std::string>)> callback) = 0;

    // Token 操作
    virtual void saveToken(
        const std::string& token,
        const std::string& userId,
        int expireSeconds,
        std::function<void(bool)> callback) = 0;

    virtual void getTokenInfo(
        const std::string& token,
        std::function<void(std::optional<std::string>)> callback) = 0;

    virtual void deleteToken(
        const std::string& token,
        std::function<void(bool)> callback) = 0;

    // Lua 脚本执行
    virtual void evalScript(
        const std::string& scriptName,
        const std::vector<std::string>& keys,
        const std::vector<std::string>& args,
        std::function<void(const RedisResult&)> callback) = 0;
};

} // namespace interfaces

#endif
