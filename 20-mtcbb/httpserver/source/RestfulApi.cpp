#include "RestfulApi.hpp"
#include <boost/token_functions.hpp>
#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpTypes.h>
#include <drogon/orm/Exception.h>
#include <drogon/utils/Utilities.h>
#include <json/value.h>
#include <trantor/utils/Date.h>

#include "models/UserTokens.h"
#include "models/Users.h"
#include <drogon/orm/Criteria.h>
#include <drogon/orm/Mapper.h>
#include <trantor/utils/Logger.h>

using namespace api::v1;
using namespace drogon::orm;
using namespace drogon_model::myapp;

bool verifyPassword(const std::string& password, const std::string& hash)
{
    return ("hash_" + password) == hash;
}

std::string hashPassword(const std::string& password)
{
    return "hash_" + password;
}


void User::login(const HttpRequestPtr& req, 
                 std::function<void(const HttpResponsePtr&)>&& callback)
{
    // 从 POST 请求体中获取参数
    auto jsonPtr = req->getJsonObject();
    if (!jsonPtr) {
        Json::Value ret;
        ret["error"] = "Invalid JSON format";
        auto resp = HttpResponse::newHttpJsonResponse(ret);
        resp->setStatusCode(k400BadRequest);
    }

    auto userId = (*jsonPtr).get("userId", "") .asString();
    auto password = (*jsonPtr).get("password", "") .asString();
    
    LOG_INFO << "User " << userId  << " password: " << password << " attempting login";

    
    // 参数验证
    if (userId.empty() || password.empty()) {
        Json::Value ret;
        ret["error"] = "userId and password are required";
        auto resp = HttpResponse::newHttpJsonResponse(ret);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }
    
    // 获取数据库客户端
    auto dbClient = drogon::app().getDbClient();
    Mapper<Users> userMapper(dbClient);
    
    // 使用 ORM 查询用户
    userMapper.findBy(
        Criteria(Users::Cols::_user_id, CompareOperator::EQ, userId),
        [callback, password, userId, dbClient](std::vector<Users> users) {
            // 用户不存在
            if (users.empty()) {
                LOG_WARN << "User " << userId << " not found";
                Json::Value ret;
                ret["error"] = "Invalid userId or password";
                auto resp = HttpResponse::newHttpJsonResponse(ret);
                resp->setStatusCode(k401Unauthorized);
                callback(resp);
                return;
            }
            
            auto& user = users[0];
            
            // 检查用户是否激活
            if (!user.getValueOfIsActive()) {
                LOG_WARN << "User " << userId << " is inactive";
                Json::Value ret;
                ret["error"] = "User account is inactive";
                auto resp = HttpResponse::newHttpJsonResponse(ret);
                resp->setStatusCode(k403Forbidden);
                callback(resp);
                return;
            }
            
            // 验证密码
            if (!verifyPassword(password, user.getValueOfPasswordHash())) {
                LOG_WARN << "Invalid password for user " << userId;
                Json::Value ret;
                ret["error"] = "Invalid userId or password";
                auto resp = HttpResponse::newHttpJsonResponse(ret);
                resp->setStatusCode(k401Unauthorized);
                callback(resp);
                return;
            }
            
            // 登录成功，生成 token
            std::string token = drogon::utils::getUuid();
            
            // 将 token 保存到数据库
            UserTokens newToken;
            newToken.setUserId(userId);
            newToken.setToken(token);
            
            // 设置过期时间（24小时后）
            auto now = trantor::Date::now();
            auto expiresAt = now.after(24 * 3600); // 24小时
            newToken.setExpiresAt(expiresAt);
            
            Mapper<UserTokens> tokenMapper(dbClient);
            tokenMapper.insert(
                newToken,
                [callback, token, user](UserTokens savedToken) {
                    LOG_INFO << "User " << user.getValueOfUserId() << " logged in successfully";
                    
                    Json::Value ret;
                    ret["result"] = "ok";
                    ret["token"] = token;
                    ret["user_id"] = user.getValueOfUserId();
                    ret["username"] = user.getValueOfUsername();
                    ret["expires_in"] = 86400; // 24小时（秒）
                    
                    auto resp = HttpResponse::newHttpJsonResponse(ret);
                    callback(resp);
                },
                [callback](const DrogonDbException& e) {
                    LOG_ERROR << "Failed to save token: " << e.base().what();
                    Json::Value ret;
                    ret["error"] = "Failed to create session";
                    auto resp = HttpResponse::newHttpJsonResponse(ret);
                    resp->setStatusCode(k500InternalServerError);
                    callback(resp);
                }
            );
        },
        [callback](const DrogonDbException& e) {
            LOG_ERROR << "Database error: " << e.base().what();
            Json::Value ret;
            ret["error"] = "Database error";
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(k500InternalServerError);
            callback(resp);
        }
    );
}



void User::getInfo(const HttpRequestPtr& req,
                   std::function<void(const HttpResponsePtr&)>&& callback,
                   std::string userId, 
                   const std::string& token) const
{
    LOG_INFO << "Getting info for user " << userId << " with token " << token;
    
    // 参数验证
    if (userId.empty() || token.empty()) {
        Json::Value ret;
        ret["error"] = "userId and token are required";
        auto resp = HttpResponse::newHttpJsonResponse(ret);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }
    
    auto dbClient = drogon::app().getDbClient();
    Mapper<UserTokens> tokenMapper(dbClient);
    
    // 验证 token
    tokenMapper.findBy(
        Criteria(UserTokens::Cols::_token, CompareOperator::EQ, token),
        [callback, userId, dbClient](std::vector<UserTokens> tokens) {
            // Token 不存在
            if (tokens.empty()) {
                LOG_WARN << "Invalid token";
                Json::Value ret;
                ret["error"] = "Invalid or expired token";
                auto resp = HttpResponse::newHttpJsonResponse(ret);
                resp->setStatusCode(k401Unauthorized);
                callback(resp);
                return;
            }
            
            auto& tokenRecord = tokens[0];
            
            // 检查 token 是否过期
            auto now = trantor::Date::now();
            if (tokenRecord.getValueOfExpiresAt() < now) {
                LOG_WARN << "Token expired";
                Json::Value ret;
                ret["error"] = "Token expired";
                auto resp = HttpResponse::newHttpJsonResponse(ret);
                resp->setStatusCode(k401Unauthorized);
                callback(resp);
                return;
            }
            
            // 检查 token 是否属于该用户
            if (tokenRecord.getValueOfUserId() != userId) {
                LOG_WARN << "Token does not belong to user " << userId;
                Json::Value ret;
                ret["error"] = "Unauthorized";
                auto resp = HttpResponse::newHttpJsonResponse(ret);
                resp->setStatusCode(k403Forbidden);
                callback(resp);
                return;
            }
            
            // Token 验证通过，获取用户信息
            Mapper<Users> userMapper(dbClient);
            userMapper.findBy(
                Criteria(Users::Cols::_user_id, CompareOperator::EQ, userId),
                [callback](std::vector<Users> users) {
                    if (users.empty()) {
                        LOG_WARN << "User not found";
                        Json::Value ret;
                        ret["error"] = "User not found";
                        auto resp = HttpResponse::newHttpJsonResponse(ret);
                        resp->setStatusCode(k404NotFound);
                        callback(resp);
                        return;
                    }
                    
                    auto& user = users[0];
                    
                    LOG_INFO << "Successfully retrieved info for user " << user.getValueOfUserId();
                    
                    Json::Value ret;
                    ret["result"] = "ok";
                    ret["user_id"] = user.getValueOfUserId();
                    ret["user_name"] = user.getValueOfUsername();
                    ret["gender"] = user.getValueOfGender();
                    ret["email"] = user.getValueOfEmail();
                    ret["created_at"] = user.getValueOfCreatedAt().toDbStringLocal();
                    
                    auto resp = HttpResponse::newHttpJsonResponse(ret);
                    callback(resp);
                },
                [callback](const DrogonDbException& e) {
                    LOG_ERROR << "Database error: " << e.base().what();
                    Json::Value ret;
                    ret["error"] = "Database error";
                    auto resp = HttpResponse::newHttpJsonResponse(ret);
                    resp->setStatusCode(k500InternalServerError);
                    callback(resp);
                }
            );
        },
        [callback](const DrogonDbException& e) {
            LOG_ERROR << "Database error: " << e.base().what();
            Json::Value ret;
            ret["error"] = "Database error";
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(k500InternalServerError);
            callback(resp);
        }
    );
}
