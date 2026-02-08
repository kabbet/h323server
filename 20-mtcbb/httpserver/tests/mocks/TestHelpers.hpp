#ifndef TESTHELPERS_HPP
#define TESTHELPERS_HPP

#include "models/Users.h"
#include "models/UserTokens.h"
#include <trantor/utils/Date.h>
#include <string>

namespace test_helpers {

/**
 * @brief 创建测试用户
 */
inline drogon_model::myapp::Users createTestUser(
    const std::string& userId,
    const std::string& password = "test123",
    bool isActive = true)
{
    drogon_model::myapp::Users user;
    user.setUserId(userId);
    user.setUsername("TestUser_" + userId);
    user.setPasswordHash("hash_" + password);  // 简单的密码哈希模拟
    user.setIsActive(isActive);
    user.setEmail(userId + "@test.com");
    user.setGender(1);
    user.setCreateAt(trantor::Date::now());
    user.setUpdateAt(trantor::Date::now());
    return user;
}

/**
 * @brief 创建测试 Token
 */
inline drogon_model::myapp::UserTokens createTestToken(
    const std::string& token,
    const std::string& userId,
    int expiresInSeconds = 3600)
{
    drogon_model::myapp::UserTokens tokenObj;
    tokenObj.setToken(token);
    tokenObj.setUserId(userId);
    
    auto now = trantor::Date::now();
    tokenObj.setCreatedAt(now);
    tokenObj.setExpiresAt(now.after(expiresInSeconds));
    
    return tokenObj;
}

/**
 * @brief 创建过期的测试 Token
 */
inline drogon_model::myapp::UserTokens createExpiredToken(
    const std::string& token,
    const std::string& userId)
{
    drogon_model::myapp::UserTokens tokenObj;
    tokenObj.setToken(token);
    tokenObj.setUserId(userId);
    
    auto now = trantor::Date::now();
    tokenObj.setCreatedAt(now.after(-3600));  // 1小时前创建
    tokenObj.setExpiresAt(now.after(-1));      // 1秒前过期
    
    return tokenObj;
}

/**
 * @brief 测试结果收集器
 * 用于异步回调的结果收集
 */
template<typename T>
class ResultCollector {
public:
    bool hasResult() const { return hasResult_; }
    bool hasError() const { return hasError_; }
    
    const T& getResult() const { return result_; }
    const std::string& getError() const { return errorMsg_; }
    int getErrorCode() const { return errorCode_; }
    
    void setResult(const T& result) {
        result_ = result;
        hasResult_ = true;
    }
    
    void setError(const std::string& error, int code) {
        errorMsg_ = error;
        errorCode_ = code;
        hasError_ = true;
    }
    
    void reset() {
        hasResult_ = false;
        hasError_ = false;
        errorMsg_.clear();
        errorCode_ = 0;
    }

private:
    T result_;
    std::string errorMsg_;
    int errorCode_ = 0;
    bool hasResult_ = false;
    bool hasError_ = false;
};

/**
 * @brief 简单的结果收集器（无返回值）
 */
class SimpleResultCollector {
public:
    bool hasSuccess() const { return hasSuccess_; }
    bool hasError() const { return hasError_; }
    
    const std::string& getError() const { return errorMsg_; }
    int getErrorCode() const { return errorCode_; }
    
    void setSuccess() {
        hasSuccess_ = true;
    }
    
    void setError(const std::string& error, int code) {
        errorMsg_ = error;
        errorCode_ = code;
        hasError_ = true;
    }
    
    void reset() {
        hasSuccess_ = false;
        hasError_ = false;
        errorMsg_.clear();
        errorCode_ = 0;
    }

private:
    std::string errorMsg_;
    int errorCode_ = 0;
    bool hasSuccess_ = false;
    bool hasError_ = false;
};

} // namespace test_helpers

#endif
