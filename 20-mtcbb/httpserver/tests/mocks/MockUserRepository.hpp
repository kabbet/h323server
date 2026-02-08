#ifndef MOCKUSERREPOSITORY_HPP
#define MOCKUSERREPOSITORY_HPP

#include "interfaces/IUserRepository.hpp"
#include <map>
#include <vector>

namespace mocks {

/**
 * @brief Mock UserRepository for testing
 * 模拟数据库操作，支持预设返回值和错误
 */
class MockUserRepository : public interfaces::IUserRepository {
public:
    MockUserRepository() = default;

    // ========== 预设数据的方法 ==========
    
    /**
     * @brief 添加一个测试用户到 Mock 数据库
     */
    void addUser(const drogon_model::myapp::Users& user) {
        users_[user.getValueOfUserId()] = user;
    }

    /**
     * @brief 添加一个测试 Token 到 Mock 数据库
     */
    void addToken(const drogon_model::myapp::UserTokens& token) {
        tokens_[token.getValueOfToken()] = token;
    }

    /**
     * @brief 设置 findUserById 是否应该失败
     */
    void setFindUserShouldFail(bool shouldFail) {
        findUserShouldFail_ = shouldFail;
    }

    /**
     * @brief 设置 saveToken 是否应该失败
     */
    void setSaveTokenShouldFail(bool shouldFail) {
        saveTokenShouldFail_ = shouldFail;
    }

    /**
     * @brief 设置 findTokenByValue 是否应该失败
     */
    void setFindTokenShouldFail(bool shouldFail) {
        findTokenShouldFail_ = shouldFail;
    }

    /**
     * @brief 清空所有数据
     */
    void clear() {
        users_.clear();
        tokens_.clear();
        findUserShouldFail_ = false;
        saveTokenShouldFail_ = false;
        findTokenShouldFail_ = false;
    }

    // ========== IUserRepository 接口实现 ==========

    void findUserById(
        const std::string& userId,
        UserCallback onSuccess,
        ErrorCallback onError) override 
    {
        if (findUserShouldFail_) {
            onError(std::runtime_error("Mock database error"));
            return;
        }

        auto it = users_.find(userId);
        if (it != users_.end()) {
            onSuccess(it->second);
        } else {
            onSuccess(std::nullopt);
        }
    }

    void saveToken(
        const drogon_model::myapp::UserTokens& token,
        std::function<void(bool)> onSuccess,
        ErrorCallback onError) override 
    {
        if (saveTokenShouldFail_) {
            onError(std::runtime_error("Mock save token error"));
            return;
        }

        tokens_[token.getValueOfToken()] = token;
        onSuccess(true);
    }

    void findTokenByValue(
        const std::string& token,
        TokenCallback onSuccess,
        ErrorCallback onError) override 
    {
        if (findTokenShouldFail_) {
            onError(std::runtime_error("Mock find token error"));
            return;
        }

        auto it = tokens_.find(token);
        if (it != tokens_.end()) {
            onSuccess(it->second);
        } else {
            onSuccess(std::nullopt);
        }
    }

    void deleteToken(
        const std::string& token,
        std::function<void(bool)> onSuccess,
        ErrorCallback onError) override 
    {
        auto it = tokens_.find(token);
        if (it != tokens_.end()) {
            tokens_.erase(it);
            onSuccess(true);
        } else {
            onSuccess(false);
        }
    }

    // ========== 测试辅助方法 ==========

    /**
     * @brief 检查 token 是否被保存
     */
    bool hasToken(const std::string& token) const {
        return tokens_.find(token) != tokens_.end();
    }

    /**
     * @brief 获取保存的 token 数量
     */
    size_t getTokenCount() const {
        return tokens_.size();
    }

private:
    std::map<std::string, drogon_model::myapp::Users> users_;
    std::map<std::string, drogon_model::myapp::UserTokens> tokens_;
    
    bool findUserShouldFail_ = false;
    bool saveTokenShouldFail_ = false;
    bool findTokenShouldFail_ = false;
};

} // namespace mocks

#endif
