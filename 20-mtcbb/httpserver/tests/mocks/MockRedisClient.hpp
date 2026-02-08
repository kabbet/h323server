#ifndef MOCKREDISCLIENT_HPP
#define MOCKREDISCLIENT_HPP

#include "interfaces/IRedisClient.hpp"
#include <map>
#include <chrono>

namespace mocks {

/**
 * @brief Mock RedisClient for testing
 * 模拟 Redis 操作，支持 Token 存储和过期
 */
class MockRedisClient : public interfaces::IRedisClient {
public:
    MockRedisClient() = default;

    // ========== 预设行为的方法 ==========

    /**
     * @brief 设置操作是否应该失败
     */
    void setShouldFail(bool shouldFail) {
        shouldFail_ = shouldFail;
    }

    /**
     * @brief 预设一个 Token 数据
     */
    void setTokenData(const std::string& token, const std::string& userId) {
        tokenData_[token] = userId;
    }

    /**
     * @brief 清空所有数据
     */
    void clear() {
        data_.clear();
        hashData_.clear();
        tokenData_.clear();
        shouldFail_ = false;
    }

    /**
     * @brief 检查 token 是否存在
     */
    bool hasToken(const std::string& token) const {
        return tokenData_.find(token) != tokenData_.end();
    }

    /**
     * @brief 获取 token 数量
     */
    size_t getTokenCount() const {
        return tokenData_.size();
    }

    // ========== IRedisClient 接口实现 ==========

    void set(
        const std::string& key,
        const std::string& value,
        std::function<void(bool)> callback,
        int expireSeconds = 0) override 
    {
        if (shouldFail_) {
            callback(false);
            return;
        }
        data_[key] = value;
        callback(true);
    }

    void get(
        const std::string& key,
        std::function<void(std::optional<std::string>)> callback) override 
    {
        auto it = data_.find(key);
        if (it != data_.end()) {
            callback(it->second);
        } else {
            callback(std::nullopt);
        }
    }

    void del(
        const std::string& key,
        std::function<void(bool)> callback) override 
    {
        auto it = data_.find(key);
        if (it != data_.end()) {
            data_.erase(it);
            callback(true);
        } else {
            callback(false);
        }
    }

    void hset(
        const std::string& key,
        const std::string& field,
        const std::string& value,
        std::function<void(bool)> callback) override 
    {
        if (shouldFail_) {
            callback(false);
            return;
        }
        hashData_[key][field] = value;
        callback(true);
    }

    void hget(
        const std::string& key,
        const std::string& field,
        std::function<void(std::optional<std::string>)> callback) override 
    {
        auto keyIt = hashData_.find(key);
        if (keyIt != hashData_.end()) {
            auto fieldIt = keyIt->second.find(field);
            if (fieldIt != keyIt->second.end()) {
                callback(fieldIt->second);
                return;
            }
        }
        callback(std::nullopt);
    }

    void hgetall(
        const std::string& key,
        std::function<void(std::map<std::string, std::string>)> callback) override 
    {
        auto it = hashData_.find(key);
        if (it != hashData_.end()) {
            callback(it->second);
        } else {
            callback(std::map<std::string, std::string>());
        }
    }

    void saveToken(
        const std::string& token,
        const std::string& userId,
        int expireSeconds,
        std::function<void(bool)> callback) override 
    {
        if (shouldFail_) {
            callback(false);
            return;
        }
        tokenData_[token] = userId;
        callback(true);
    }

    void getTokenInfo(
        const std::string& token,
        std::function<void(std::optional<std::string>)> callback) override 
    {
        auto it = tokenData_.find(token);
        if (it != tokenData_.end()) {
            callback(it->second);
        } else {
            callback(std::nullopt);
        }
    }

    void deleteToken(
        const std::string& token,
        std::function<void(bool)> callback) override 
    {
        auto it = tokenData_.find(token);
        if (it != tokenData_.end()) {
            tokenData_.erase(it);
            callback(true);
        } else {
            callback(false);
        }
    }

    void evalScript(
        const std::string& scriptName,
        const std::vector<std::string>& keys,
        const std::vector<std::string>& args,
        std::function<void(const drogon::nosql::RedisResult&)> callback) override 
    {
        // Mock implementation - 简单返回空结果
        // 如果需要测试 Lua 脚本，可以在这里添加逻辑
    }

private:
    std::map<std::string, std::string> data_;
    std::map<std::string, std::map<std::string, std::string>> hashData_;
    std::map<std::string, std::string> tokenData_;  // token -> userId
    bool shouldFail_ = false;
};

} // namespace mocks

#endif
