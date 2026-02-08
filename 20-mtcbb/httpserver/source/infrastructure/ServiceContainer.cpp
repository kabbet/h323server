#include "ServiceContainer.hpp"
#include "UserService.hpp"
#include "UserRepository.hpp"
#include "RedisClientAdapter.hpp"
#include "RedisUtils.hpp"
#include <drogon/HttpAppFramework.h>

void ServiceContainer::initialize()
{
    LOG_INFO << "Initializing ServiceContainer...";

    // 创建数据库客户端
    auto dbClient = drogon::app().getDbClient();

    // 创建 Repository
    userRepo_ = std::make_shared<repositories::UserRepository>(dbClient);

    // 创建 Redis 客户端适配器
    redisClient_ = std::make_shared<adapters::RedisClientAdapter>(
        RedisUtils::instance()
    );

    // 创建 UserService（注入依赖）
    userService_ = std::make_shared<services::UserService>(
        userRepo_,
        redisClient_
    );

    LOG_INFO << "ServiceContainer initialized successfully";
}