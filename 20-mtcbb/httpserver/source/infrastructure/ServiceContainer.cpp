#include "ServiceContainer.hpp"
#include "UserService.hpp"
#include "UserRepository.hpp"
#include "RedisClientAdapter.hpp"
#include "RedisUtils.hpp"
#include "SystemService.hpp"
#include <drogon/HttpAppFramework.h>

void ServiceContainer::initialize()
{
    LOG_INFO << "Initializing ServiceContainer...";

    auto dbClient = drogon::app().getDbClient();
    userRepo_     = std::make_shared<repositories::UserRepository>(dbClient);

    auto redisAdapter = std::make_shared<adapters::RedisClientAdapter>(
        RedisUtils::instance()
    );
    redisClient_ = redisAdapter;

    userService_ = std::make_shared<services::UserService>(userRepo_, redisAdapter);

    services::SystemService::LicenseMap licenses = {
        {"your_software_key", "your_software_secret"},
    };
    systemService_ = std::make_shared<services::SystemService>(
        userRepo_, redisAdapter, std::move(licenses)
    );

    LOG_INFO << "ServiceContainer initialized successfully";
}