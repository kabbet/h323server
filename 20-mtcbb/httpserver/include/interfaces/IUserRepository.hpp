#ifndef IUSERREPOSITORY_HPP
#define IUSERREPOSITORY_HPP

#include <functional>
#include <optional>
#include <string>
#include <vector>
#include "models/Users.h"
#include "models/UserTokens.h"

namespace interfaces {

class IUserRepository {
public:
    virtual ~IUserRepository() = default;

    using UserCallback = std::function<void(std::optional<drogon_model::myapp::Users>)>;
    using UsersCallback = std::function<void(std::vector<drogon_model::myapp::Users>)>;
    using TokenCallback = std::function<void(std::optional<drogon_model::myapp::UserTokens>)>;
    using ErrorCallback = std::function<void(const std::exception&)>;

    // 查询用户
    virtual void findUserById(
        const std::string& userId,
        UserCallback onSuccess,
        ErrorCallback onError) = 0;

    // 保存 Token
    virtual void saveToken(
        const drogon_model::myapp::UserTokens& token,
        std::function<void(bool)> onSuccess,
        ErrorCallback onError) = 0;

    // 查找 Token
    virtual void findTokenByValue(
        const std::string& token,
        TokenCallback onSuccess,
        ErrorCallback onError) = 0;

    // 删除 Token
    virtual void deleteToken(
        const std::string& token,
        std::function<void(bool)> onSuccess,
        ErrorCallback onError) = 0;
};

} // namespace interfaces

#endif
