// source/TokenCleanupService.cpp
#include "TokenCleanupService.hpp"
#include "RedisUtils.hpp"
#include <drogon/orm/Mapper.h>
#include <drogon/orm/Criteria.h>
#include <trantor/utils/Logger.h>

using namespace service;
using namespace drogon::orm;
using namespace drogon_model::myapp;

TokenCleanupService& TokenCleanupService::instance()
{
    static TokenCleanupService inst;
    return inst;
}

void TokenCleanupService::initialize()
{
    LOG_INFO << "Initializing TokenCleanupService...";
    
    // 订阅 Redis token 过期事件
    RedisUtils::instance().subscribeTokenExpiration(
        [this](const std::string& expiredToken) {
            this->handleTokenExpiration(expiredToken);
        }
    );
    
    LOG_INFO << "TokenCleanupService initialized successfully";
}

void TokenCleanupService::handleTokenExpiration(const std::string& token)
{
    LOG_INFO << "Handling token expiration: " << token;
    
    // 从 PostgreSQL 删除 token
    deleteTokenFromDatabase(token);
}

void TokenCleanupService::deleteTokenFromDatabase(const std::string& token)
{
    auto dbClient = drogon::app().getDbClient();
    Mapper<UserTokens> tokenMapper(dbClient);
    
    try {
        // 删除 token
        tokenMapper.deleteBy(
            Criteria(UserTokens::Cols::_token, CompareOperator::EQ, token)
        );
        
        LOG_INFO << "Token deleted from PostgreSQL: " << token;
    } 
    catch (const DrogonDbException& e) {
        LOG_ERROR << "Failed to delete token from PostgreSQL: " 
                  << token << ", error: " << e.base().what();
    }
}
