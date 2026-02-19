#include "SystemController.hpp"
#include "ServiceContainer.hpp"
#include <json/value.h>

static constexpr int     kApiLevel   = 1;
static const std::string kApiVersion = "V1.0.0.0";

using namespace api::v1::system;

HttpResponsePtr System::makeSuccess(Json::Value extra)
{
    extra["success"] = 1;
    auto resp = HttpResponse::newHttpJsonResponse(extra);
    resp->setStatusCode(k200OK);
    return resp;
}

HttpResponsePtr System::makeError(int errorCode, const std::string& msg,
                                   HttpStatusCode httpStatus)
{
    Json::Value body;
    body["success"]    = 0;
    body["error_code"] = errorCode;
    body["error_msg"]  = msg;
    auto resp = HttpResponse::newHttpJsonResponse(body);
    resp->setStatusCode(httpStatus);
    return resp;
}

// POST /api/v1/system/token — 软件鉴权，返回 account_token
void System::getAccountToken(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    const std::string& consumerKey    = req->getParameter("oauth_consumer_key");
    const std::string& consumerSecret = req->getParameter("oauth_consumer_secret");

    if (consumerKey.empty() || consumerSecret.empty()) {
        callback(makeError(1000, "Missing oauth_consumer_key or oauth_consumer_secret"));
        return;
    }

    // ✅ 通过 getService() 懒加载，不使用成员变量
    getService()->registerLicense(
        consumerKey,
        consumerSecret,
        [callback](const std::string& accountToken) {
            Json::Value body;
            body["account_token"] = accountToken;
            callback(makeSuccess(body));
        },
        [callback](const std::string& msg, int code) {
            callback(makeError(code, msg));
        });
}

// POST /api/v1/system/login — 用户登录，Set-Cookie: SSO_COOKIE_KEY=xxx
void System::login(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    const std::string accountToken = req->getParameter("account_token"); // ✅ 修正大小写
    const std::string username     = req->getParameter("username");
    const std::string password     = req->getParameter("password");

    if (accountToken.empty() || username.empty() || password.empty()) {
        callback(makeError(1000, "Missing required parameters"));
        return;
    }

    getService()->loginUser(
        accountToken,
        username,
        password,
        [callback](const std::string& uname, const std::string& ssoCookie) {
            Json::Value body;
            body["username"] = uname;
            auto resp = makeSuccess(body);
            // ✅ 修正：补上 = 号
            resp->addHeader("Set-Cookie",
                "SSO_COOKIE_KEY=" + ssoCookie + "; Path=/; HttpOnly");
            callback(resp);
        },
        [callback](const std::string& msg, int code) {
            callback(makeError(code, msg));
        });
}

// GET /api/v1/system/version — 返回 API-Level 及平台版本
void System::getVersion(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) const
{
    (void)req;
    Json::Value body;
    body["api_level"] = kApiLevel;
    body["version"]   = kApiVersion;
    callback(makeSuccess(body));
}