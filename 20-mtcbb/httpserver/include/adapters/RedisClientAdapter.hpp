#ifndef REDISCLIENTADAPTER_HPP
#define REDISCLIENTADAPTER_HPP

#include "interfaces/IRedisClient.hpp"
#include "RedisUtils.hpp"

namespace adapters {

// 适配器模式：将 RedisUtils 适配为 IRedisClient 接口
class RedisClientAdapter : public interfaces::IRedisClient {
public:
    explicit RedisClientAdapter(RedisUtils& redisUtils)
        : redisUtils_(redisUtils) {}

    void set(const std::string& key,
            const std::string& value,
            std::function<void(bool)> callback,
            int expireSeconds = 0) override {
        redisUtils_.set(key, value, callback, expireSeconds);
    }

    void get(const std::string& key,
            std::function<void(std::optional<std::string>)> callback) override {
        redisUtils_.get(key, callback);
    }

    void del(const std::string& key,
            std::function<void(bool)> callback) override {
        redisUtils_.del(key, callback);
    }

    void hset(const std::string& key,
             const std::string& field,
             const std::string& value,
             std::function<void(bool)> callback) override {
        redisUtils_.hset(key, field, value, callback);
    }

    void hget(const std::string& key,
             const std::string& field,
             std::function<void(std::optional<std::string>)> callback) override {
        redisUtils_.hget(key, field, callback);
    }

    void hgetall(const std::string& key,
                std::function<void(std::map<std::string, std::string>)> callback) override {
        redisUtils_.hgetall(key, callback);
    }

    void saveToken(const std::string& token,
                  const std::string& userId,
                  int expireSeconds,
                  std::function<void(bool)> callback) override {
        redisUtils_.saveToken(token, userId, expireSeconds, callback);
    }

    void getTokenInfo(const std::string& token,
                     std::function<void(std::optional<std::string>)> callback) override {
        redisUtils_.getTokenInfo(token, callback);
    }

    void deleteToken(const std::string& token,
                    std::function<void(bool)> callback) override {
        redisUtils_.deleteToken(token, callback);
    }

    void evalScript(const std::string& scriptName,
                   const std::vector<std::string>& keys,
                   const std::vector<std::string>& args,
                   std::function<void(const drogon::nosql::RedisResult&)> callback) override {
        redisUtils_.evalScript(scriptName, keys, args, callback);
    }

private:
    RedisUtils& redisUtils_;
};

} // namespace adapters

#endif
