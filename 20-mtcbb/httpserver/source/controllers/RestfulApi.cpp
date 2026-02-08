#include "RestfulApi.hpp"
#include "LoginHandler.hpp"
#include "GetInfoHandler.hpp"
#include "ServiceContainer.hpp"
#include <json/value.h>

using namespace api::v1;

void User::login(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr&)>&& callback)
{
    auto jsonPtr = req->getJsonObject();
    if (!jsonPtr) {
        Json::Value ret;
        ret["error"] = "Invalid JSON format";
        auto resp = HttpResponse::newHttpJsonResponse(ret);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    auto userId = (*jsonPtr).get("userId", "").asString();
    auto password = (*jsonPtr).get("password", "").asString();

    if (userId.empty() || password.empty()) {
        Json::Value ret;
        ret["error"] = "userId and password are required";
        auto resp = HttpResponse::newHttpJsonResponse(ret);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    // 从容器获取服务（依赖注入）
    auto userService = ServiceContainer::instance().getUserService();

    // 委托给 Handler 处理
    handlers::LoginHandler::handle(
        userService,
        userId,
        password,
        std::move(callback)
    );
}

void User::getInfo(const HttpRequestPtr& req,
                   std::function<void(const HttpResponsePtr&)>&& callback,
                   std::string userId,
                   const std::string& token) const
{
    if (userId.empty() || token.empty()) {
        Json::Value ret;
        ret["error"] = "userId and token are required";
        auto resp = HttpResponse::newHttpJsonResponse(ret);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    // 从容器获取服务
    auto userService = ServiceContainer::instance().getUserService();

    // 委托给 Handler 处理
    handlers::GetInfoHandler::handle(
        userService,
        userId,
        token,
        std::move(callback)
    );
}