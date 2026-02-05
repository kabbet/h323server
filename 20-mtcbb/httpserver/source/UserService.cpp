// source/UserService.cpp
#include "UserService.hpp"
#include "RedisUtils.hpp"
#include <drogon/HttpAppFramework.h>
#include <drogon/orm/Criteria.h>
#include <drogon/orm/Mapper.h>
#include <drogon/utils/Utilities.h>
#include <optional>
#include <trantor/utils/Date.h>
#include <trantor/utils/Logger.h>

using namespace service;
using namespace drogon::orm;
using namespace drogon_model::myapp;

bool UserService::verifyPassword(const std::string& password, const std::string& hash)
{
    return ("hash_" + password) == hash;
}

void UserService::authenticateUser(const std::string& userId, const std::string& password,
                                   UserCallback onSuccess, ErrorCallback onError)
{
    auto          dbClient = drogon::app().getDbClient();
    Mapper<Users> userMapper(dbClient);

    userMapper.findBy(
        Criteria(Users::Cols::_user_id, CompareOperator::EQ, userId),
        [onSuccess, onError, password, userId](std::vector<Users> users) {
            if (users.empty()) {
                LOG_WARN << "User " << userId << " not found";
                onError("Invalid userId or password", 401);
                return;
            }

            auto& user = users[0];

            if (!user.getValueOfIsActive()) {
                LOG_WARN << "User " << userId << " is inactive";
                onError("User account is inactive", 403);
                return;
            }

            if (!UserService::verifyPassword(password, user.getValueOfPasswordHash())) {
                LOG_WARN << "Invalid password for user " << userId;
                onError("Invalid userId or password", 401);
                return;
            }

            onSuccess(user);
        },
        [onError](const DrogonDbException& e) {
            LOG_ERROR << "Database error: " << e.base().what();
            onError("Database error", 500);
        });
}

void UserService::createUserToken(const std::string& userId, TokenCallback onSuccess,
                                  ErrorCallback onError)
{
    std::string token = drogon::utils::getUuid();

    UserTokens newToken;
    newToken.setUserId(userId);
    newToken.setToken(token);

    auto now       = trantor::Date::now();
    auto expiresAt = now.after(60);
    newToken.setExpiresAt(expiresAt);

    auto               dbClient = drogon::app().getDbClient();
    Mapper<UserTokens> tokenMapper(dbClient);

    tokenMapper.insert(
        newToken,
        [onSuccess, onError, token, userId](UserTokens savedToken) {
            int expireSeconds = 60;
            RedisUtils::instance().saveToken(
                token, userId, expireSeconds, [onSuccess, onError, token](bool redisSuccess) {
                    if (redisSuccess) {
                        LOG_INFO << "Token saved to both PostgreSQL and Redis: " << token;
                        onSuccess(token);
                    }
                    else {
                        LOG_WARN << "Token saved to PostgreSQL but failed to save to Redis: "
                                 << token;
                        onSuccess(token);
                    };
                });
        },
        [onError](const DrogonDbException& e) {
            LOG_ERROR << "Failed to save token: " << e.base().what();
            onError("Failed to create session", 500);
        });
}

void UserService::validateToken(const std::string& token, const std::string& userId,
                                std::function<void()> onSuccess, ErrorCallback onError)
{
    RedisUtils::instance().getTokenInfo(
        token, [onSuccess, onError, token, userId](std::optional<std::string> redisUserId) {
            if (redisUserId.has_value()) {
                if (redisUserId.value() == userId) {
                    LOG_INFO << "Token validated from Redis: " << token;
                    onSuccess();
                }
                else {
                    LOG_WARN << "Token does blong to user (Redis): " << userId;
                    onError("Unauthorized", 403);
                }
                return;
            }
            LOG_INFO << "Token not in Redis, checking PostgreSQL: " << token;
            auto               dbClient = drogon::app().getDbClient();
            Mapper<UserTokens> tokenMapper(dbClient);

            tokenMapper.findBy(
                Criteria(UserTokens::Cols::_token, CompareOperator::EQ, token),
                [onSuccess, onError, userId](std::vector<UserTokens> tokens) {
                    if (tokens.empty()) {
                        LOG_WARN << "Invalid token";
                        onError("Invalid or expired token", 401);
                        return;
                    }

                    auto& tokenRecord = tokens[0];
                    auto  now         = trantor::Date::now();

                    if (tokenRecord.getValueOfExpiresAt() < now) {
                        LOG_WARN << "Token expired";
                        onError("Token expired", 401);
                        return;
                    }

                    if (tokenRecord.getValueOfUserId() != userId) {
                        LOG_WARN << "Token does not belong to user " << userId;
                        onError("Unauthorized", 403);
                        return;
                    }

                    onSuccess();
                },
                [onError](const DrogonDbException& e) {
                    LOG_ERROR << "Database error: " << e.base().what();
                    onError("Database error", 500);
                });
        });
}

void UserService::getUserInfo(const std::string& userId, UserCallback onSuccess,
                              ErrorCallback onError)
{
    auto          dbClient = drogon::app().getDbClient();
    Mapper<Users> userMapper(dbClient);

    userMapper.findBy(
        Criteria(Users::Cols::_user_id, CompareOperator::EQ, userId),
        [onSuccess, onError](std::vector<Users> users) {
            if (users.empty()) {
                LOG_WARN << "User not found";
                onError("User not found", 404);
                return;
            }
            onSuccess(users[0]);
        },
        [onError](const DrogonDbException& e) {
            LOG_ERROR << "Database error: " << e.base().what();
            onError("Database error", 500);
        });
}
