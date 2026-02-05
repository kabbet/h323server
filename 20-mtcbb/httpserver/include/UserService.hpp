// include/UserService.hpp
#ifndef USERSERVICE_HPP
#define USERSERVICE_HPP
#include <string>
#include <functional>
#include <drogon/orm/DbClient.h>
#include "models/Users.h"
#include "models/UserTokens.h"

namespace service {

class UserService {
public:
    using UserCallback = std::function<void(const drogon_model::myapp::Users&)>;
    using TokenCallback = std::function<void(const std::string&)>;
    using ErrorCallback = std::function<void(const std::string&, int)>;

    // 异步回调风格的接口
    static void authenticateUser(
        const std::string& userId,
        const std::string& password,
        UserCallback onSuccess,
        ErrorCallback onError
    );

    static void createUserToken(
        const std::string& userId,
        TokenCallback onSuccess,
        ErrorCallback onError
    );

    static void validateToken(
        const std::string& token,
        const std::string& userId,
        std::function<void()> onSuccess,
        ErrorCallback onError
    );

    static void getUserInfo(
        const std::string& userId,
        UserCallback onSuccess,
        ErrorCallback onError
    );

private:
    static bool verifyPassword(const std::string& password, const std::string& hash);
};

} // namespace service
#endif
