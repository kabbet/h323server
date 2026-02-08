#ifndef IUSERSERVICE_HPP
#define IUSERSERVICE_HPP

#include <functional>
#include <string>
#include "models/Users.h"

namespace interfaces {

class IUserService {
public:
    virtual ~IUserService() = default;

    using UserCallback = std::function<void(const drogon_model::myapp::Users&)>;
    using TokenCallback = std::function<void(const std::string&)>;
    using ErrorCallback = std::function<void(const std::string&, int)>;
    using ValidateCallback = std::function<void()>;

    // 用户认证
    virtual void authenticateUser(
        const std::string& userId,
        const std::string& password,
        UserCallback onSuccess,
        ErrorCallback onError) = 0;

    // 创建 Token
    virtual void createUserToken(
        const std::string& userId,
        TokenCallback onSuccess,
        ErrorCallback onError) = 0;

    // 验证 Token
    virtual void validateToken(
        const std::string& token,
        const std::string& userId,
        ValidateCallback onSuccess,
        ErrorCallback onError) = 0;

    // 获取用户信息
    virtual void getUserInfo(
        const std::string& userId,
        UserCallback onSuccess,
        ErrorCallback onError) = 0;
};

} // namespace interfaces

#endif
