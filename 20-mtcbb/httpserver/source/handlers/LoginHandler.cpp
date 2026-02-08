#include "handlers/LoginHandler.hpp"
#include <json/value.h>
#include <trantor/utils/Logger.h>

using namespace handlers;
using namespace drogon;

// ==================== LoginHandler ====================

void LoginHandler::handle(
    std::shared_ptr<interfaces::IUserService> userService,
    const std::string& userId,
    const std::string& password,
    ResponseCallback callback)
{
    auto handler = std::shared_ptr<LoginHandler>(
        new LoginHandler(userService, userId, password, std::move(callback))
    );
    handler->start();
}

LoginHandler::LoginHandler(
    std::shared_ptr<interfaces::IUserService> userService,
    const std::string& userId,
    const std::string& password,
    ResponseCallback callback)
    : userService_(userService)
    , userId_(userId)
    , password_(password)
    , callback_(std::move(callback))
{
}

void LoginHandler::start()
{
    LOG_INFO << "User " << userId_ << " attempting login";

    auto self = shared_from_this();
    // 使用注入的 userService
    userService_->authenticateUser(
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
    user_ = user;

    auto self = shared_from_this();
    // 使用注入的 userService
    userService_->createUserToken(
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


