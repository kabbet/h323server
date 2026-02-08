// source/LoginHandler.cpp
#include "GetInfoHandler.hpp"
#include "UserService.hpp"
#include <json/value.h>
#include <trantor/utils/Logger.h>

using namespace handlers;
using namespace drogon;

// ==================== GetInfoHandler ====================

void GetInfoHandler::handle(
        std::shared_ptr<interfaces::IUserService> userService,
        const std::string& userId,
        const std::string& token,
        ResponseCallback callback)
{
    auto handler = std::shared_ptr<GetInfoHandler>(
        new GetInfoHandler(userService ,userId, token, std::move(callback))
    );
    handler->start();
}

GetInfoHandler::GetInfoHandler(
        std::shared_ptr<interfaces::IUserService> userService,
        const std::string& userId,
        const std::string& token,
        ResponseCallback callback)
        : userService_(userService),
        userId_(userId),
        token_(token),
        callback_(std::move(callback))
{
}

void GetInfoHandler::start()
{
    LOG_INFO << "Getting info for user " << userId_;
    
    auto self = shared_from_this();
    userService_->validateToken(
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
    userService_->getUserInfo(
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
    ret["created_at"] = user.getValueOfCreateAt().toDbStringLocal();
    
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
