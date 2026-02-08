#include "services/UserService.hpp"
#include <drogon/utils/Utilities.h>
#include <trantor/utils/Date.h>
#include <trantor/utils/Logger.h>

using namespace services;
using namespace drogon_model::myapp;

bool UserService::verifyPassword(const std::string& password, const std::string& hash) {
    return ("hash_" + password) == hash;
}

void UserService::authenticateUser(
    const std::string& userId,
    const std::string& password,
    UserCallback onSuccess,
    ErrorCallback onError)
{
    LOG_INFO << "Authenticating user: " << userId;

    // 使用注入的 repository
    userRepo_->findUserById(
        userId,
        [this, password, onSuccess, onError](std::optional<Users> userOpt) {
            if (!userOpt.has_value()) {
                LOG_WARN << "User not found";
                onError("Invalid userId or password", 401);
                return;
            }

            auto& user = userOpt.value();

            if (!user.getValueOfIsActive()) {
                LOG_WARN << "User account is inactive";
                onError("User account is inactive", 403);
                return;
            }

            if (!verifyPassword(password, user.getValueOfPasswordHash())) {
                LOG_WARN << "Invalid password";
                onError("Invalid userId or password", 401);
                return;
            }

            LOG_INFO << "User authenticated successfully";
            onSuccess(user);
        },
        [onError](const std::exception& e) {
            LOG_ERROR << "Database error: " << e.what();
            onError("Database error", 500);
        }
    );
}

void UserService::createUserToken(
    const std::string& userId,
    TokenCallback onSuccess,
    ErrorCallback onError)
{
    std::string token = drogon::utils::getUuid();

    UserTokens newToken;
    newToken.setUserId(userId);
    newToken.setToken(token);

    auto now = trantor::Date::now();
    auto expiresAt = now.after(60);
    newToken.setExpiresAt(expiresAt);

    // 先保存到数据库
    userRepo_->saveToken(
        newToken,
        [this, token, userId, onSuccess, onError](bool dbSuccess) {
            if (!dbSuccess) {
                onError("Failed to create session", 500);
                return;
            }

            // 再保存到 Redis
            int expireSeconds = 60;
            redisClient_->saveToken(
                token,
                userId,
                expireSeconds,
                [token, onSuccess](bool redisSuccess) {
                    if (redisSuccess) {
                        LOG_INFO << "Token saved to both DB and Redis";
                    } else {
                        LOG_WARN << "Token saved to DB but failed to save to Redis";
                    }
                    onSuccess(token);
                }
            );
        },
        [onError](const std::exception& e) {
            LOG_ERROR << "Failed to save token: " << e.what();
            onError("Failed to create session", 500);
        }
    );
}

void UserService::validateToken(
    const std::string& token,
    const std::string& userId,
    ValidateCallback onSuccess,
    ErrorCallback onError)
{
    // 先检查 Redis
    redisClient_->getTokenInfo(
        token,
        [this, token, userId, onSuccess, onError](std::optional<std::string> redisUserId) {
            if (redisUserId.has_value()) {
                if (redisUserId.value() == userId) {
                    LOG_INFO << "Token validated from Redis";
                    onSuccess();
                } else {
                    LOG_WARN << "Token does not belong to user (Redis)";
                    onError("Unauthorized", 403);
                }
                return;
            }

            // Redis 中没有，检查数据库
            LOG_INFO << "Token not in Redis, checking database";
            userRepo_->findTokenByValue(
                token,
                [userId, onSuccess, onError](std::optional<UserTokens> tokenOpt) {
                    if (!tokenOpt.has_value()) {
                        LOG_WARN << "Invalid token";
                        onError("Invalid or expired token", 401);
                        return;
                    }

                    auto& tokenRecord = tokenOpt.value();
                    auto now = trantor::Date::now();

                    if (tokenRecord.getValueOfExpiresAt() < now) {
                        LOG_WARN << "Token expired";
                        onError("Token expired", 401);
                        return;
                    }

                    if (tokenRecord.getValueOfUserId() != userId) {
                        LOG_WARN << "Token does not belong to user";
                        onError("Unauthorized", 403);
                        return;
                    }

                    onSuccess();
                },
                [onError](const std::exception& e) {
                    LOG_ERROR << "Database error: " << e.what();
                    onError("Database error", 500);
                }
            );
        }
    );
}

void UserService::getUserInfo(
    const std::string& userId,
    UserCallback onSuccess,
    ErrorCallback onError)
{
    userRepo_->findUserById(
        userId,
        [onSuccess, onError](std::optional<Users> userOpt) {
            if (!userOpt.has_value()) {
                LOG_WARN << "User not found";
                onError("User not found", 404);
                return;
            }
            onSuccess(userOpt.value());
        },
        [onError](const std::exception& e) {
            LOG_ERROR << "Database error: " << e.what();
            onError("Database error", 500);
        }
    );
}
