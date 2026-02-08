#ifndef SERVICECONTAINER_HPP
#define SERVICECONTAINER_HPP

#include <memory>
#include "interfaces/IUserService.hpp"
#include "interfaces/IUserRepository.hpp"
#include "interfaces/IRedisClient.hpp"

class ServiceContainer {
public:
    static ServiceContainer& instance() {
        static ServiceContainer inst;
        return inst;
    }

    void initialize();

    std::shared_ptr<interfaces::IUserService> getUserService() {
        return userService_;
    }

    std::shared_ptr<interfaces::IUserRepository> getUserRepository() {
        return userRepo_;
    }

    std::shared_ptr<interfaces::IRedisClient> getRedisClient() {
        return redisClient_;
    }

    // 用于测试：设置 Mock 对象
    void setUserService(std::shared_ptr<interfaces::IUserService> service) {
        userService_ = service;
    }

    void setUserRepository(std::shared_ptr<interfaces::IUserRepository> repo) {
        userRepo_ = repo;
    }

    void setRedisClient(std::shared_ptr<interfaces::IRedisClient> client) {
        redisClient_ = client;
    }

private:
    ServiceContainer() = default;

    std::shared_ptr<interfaces::IUserService> userService_;
    std::shared_ptr<interfaces::IUserRepository> userRepo_;
    std::shared_ptr<interfaces::IRedisClient> redisClient_;
};

#endif
