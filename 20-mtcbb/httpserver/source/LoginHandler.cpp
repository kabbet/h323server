// source/LoginHandler.cpp
#include "LoginHandler.hpp"
#include "UserService.hpp"
#include <json/value.h>
#include <trantor/utils/Logger.h>

using namespace handler;
using namespace drogon;

// ==================== LoginHandler ====================

void LoginHandler::handle(const std::string& userId, 
                         const std::string& password,
                         ResponseCallback callback)
{
    auto handler = std::shared_ptr<LoginHandler>(
        new LoginHandler(userId, password, std::move(callback))
    );
    handler->start();
}

LoginHandler::LoginHandler(const std::string& userId, 
                          const std::string& password,
                          ResponseCallback callback)
    : userId_(userId)
    , password_(password)
    , callback_(std::move(callback))
{
}

void LoginHandler::start()
{
    LOG_INFO << "User " << userId_ << " attempting login";
    
    auto self = shared_from_this();
    service::UserService::authenticateUser(
        userId_,
        password_,
        [self](const auto& user) { 
            self->onUserAuthenticated(user); 
        },
        [self](const auto& error, int code) { 
            self->onAuthenticationFailed(error, code); 
        }
    );
}

void LoginHandler::onUserAuthenticated(const drogon_model::myapp::Users& user)
{
    user_ = user;  // 保存用户信息供后续使用
    
    auto self = shared_from_this();
    service::UserService::createUserToken(
        userId_,
        [self](const std::string& token) { 
            self->onTokenCreated(token); 
        },
        [self](const auto& error, int code) { 
            self->onTokenCreationFailed(error, code); 
        }
    );
}

void LoginHandler::onAuthenticationFailed(const std::string& error, int statusCode)
{
    sendErrorResponse(error, statusCode);
}

void LoginHandler::onTokenCreated(const std::string& token)
{
    LOG_INFO << "User " << userId_ << " logged in successfully";
    
    Json::Value ret;
    ret["result"] = "ok";
    ret["token"] = token;
    ret["user_id"] = user_.getValueOfUserId();
    ret["username"] = user_.getValueOfUsername();
    ret["expires_in"] = 60;
    
    callback_(HttpResponse::newHttpJsonResponse(ret));
}

void LoginHandler::onTokenCreationFailed(const std::string& error, int statusCode)
{
    sendErrorResponse(error, statusCode);
}

void LoginHandler::sendErrorResponse(const std::string& error, int statusCode)
{
    Json::Value ret;
    ret["error"] = error;
    auto resp = HttpResponse::newHttpJsonResponse(ret);
    resp->setStatusCode(static_cast<HttpStatusCode>(statusCode));
    callback_(resp);
}

// ==================== GetInfoHandler ====================

void GetInfoHandler::handle(const std::string& userId,
                           const std::string& token,
                           ResponseCallback callback)
{
    auto handler = std::shared_ptr<GetInfoHandler>(
        new GetInfoHandler(userId, token, std::move(callback))
    );
    handler->start();
}

GetInfoHandler::GetInfoHandler(const std::string& userId,
                              const std::string& token,
                              ResponseCallback callback)
    : userId_(userId)
    , token_(token)
    , callback_(std::move(callback))
{
}

void GetInfoHandler::start()
{
    LOG_INFO << "Getting info for user " << userId_;
    
    auto self = shared_from_this();
    service::UserService::validateToken(
        token_,
        userId_,
        [self]() { 
            self->onTokenValidated(); 
        },
        [self](const auto& error, int code) { 
            self->onTokenValidationFailed(error, code); 
        }
    );
}

void GetInfoHandler::onTokenValidated()
{
    auto self = shared_from_this();
    service::UserService::getUserInfo(
        userId_,
        [self](const auto& user) { 
            self->onUserInfoRetrieved(user); 
        },
        [self](const auto& error, int code) { 
            self->onUserInfoFailed(error, code); 
        }
    );
}

void GetInfoHandler::onTokenValidationFailed(const std::string& error, int statusCode)
{
    sendErrorResponse(error, statusCode);
}

void GetInfoHandler::onUserInfoRetrieved(const drogon_model::myapp::Users& user)
{
    LOG_INFO << "Successfully retrieved info for user " << userId_;
    
    Json::Value ret;
    ret["result"] = "ok";
    ret["user_id"] = user.getValueOfUserId();
    ret["user_name"] = user.getValueOfUsername();
    ret["gender"] = user.getValueOfGender();
    ret["email"] = user.getValueOfEmail();
    ret["created_at"] = user.getValueOfCreatedAt().toDbStringLocal();
    
    callback_(HttpResponse::newHttpJsonResponse(ret));
}

void GetInfoHandler::onUserInfoFailed(const std::string& error, int statusCode)
{
    sendErrorResponse(error, statusCode);
}

void GetInfoHandler::sendErrorResponse(const std::string& error, int statusCode)
{
    Json::Value ret;
    ret["error"] = error;
    auto resp = HttpResponse::newHttpJsonResponse(ret);
    resp->setStatusCode(static_cast<HttpStatusCode>(statusCode));
    callback_(resp);
}
