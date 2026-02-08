#ifndef USERSERVICE_HPP
#define USERSERVICE_HPP

#include "interfaces/IUserService.hpp"
#include "interfaces/IUserRepository.hpp"
#include "interfaces/IRedisClient.hpp"
#include <memory>

namespace services {

class UserService : public interfaces::IUserService {
public:
    // 构造函数注入依赖
    UserService(
        std::shared_ptr<interfaces::IUserRepository> userRepo,
        std::shared_ptr<interfaces::IRedisClient> redisClient)
        : userRepo_(userRepo)
        , redisClient_(redisClient) {}

    void authenticateUser(
        const std::string& userId,
        const std::string& password,
        UserCallback onSuccess,
        ErrorCallback onError) override;

    void createUserToken(
        const std::string& userId,
        TokenCallback onSuccess,
        ErrorCallback onError) override;

    void validateToken(
        const std::string& token,
        const std::string& userId,
        ValidateCallback onSuccess,
        ErrorCallback onError) override;

    void getUserInfo(
        const std::string& userId,
        UserCallback onSuccess,
        ErrorCallback onError) override;

    // 静态辅助方法（可以独立测试）
    static bool verifyPassword(const std::string& password, const std::string& hash);

private:
    std::shared_ptr<interfaces::IUserRepository> userRepo_;
    std::shared_ptr<interfaces::IRedisClient> redisClient_;
};

} // namespace services

#endif
