#include "RestfulApi.hpp"
#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <drogon/utils/Utilities.h>
#include <json/value.h>

using namespace api::v1;

void User::login(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback)
{
        // 从 POST 请求体中获取参数
    auto userId = req->getParameter("userId");
    auto password = req->getParameter("password");
    LOG_INFO << "User" << userId << "login";

    Json::Value ret;
    ret["result"] = "ok";
    ret["token"]  = utils::getUuid();
    auto resp     = HttpResponse::newHttpJsonResponse(ret);
    callback(resp);
}  

void User::getInfo(const HttpRequestPtr& req,
        std::function<void (const HttpResponsePtr&)> && callback,
        std::string userId, const std::string token) const
{
    LOG_INFO<< "User" << userId << "get his infomation";

    Json::Value ret;
    ret["result"] = "ok";
    ret["user_name"] = "Jack";
    ret["user_id"] = userId;
    ret["gender"] = 1;
    auto resp = HttpResponse::newHttpJsonResponse(ret);
    callback(resp);
}
