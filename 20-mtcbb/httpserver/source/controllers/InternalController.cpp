#include "controllers/InternalController.hpp"
#include <drogon/HttpClient.h>
#include <json/json.h>
#include <trantor/utils/Logger.h>

namespace drogon {
// Jetty  网关推送接口地址
const std::string Internal::kGatewayPublishUrl = "http://127.0.0.1:8081/internal/event/publish";

HttpResponsePtr Internal::makeSuccess(Json::Value extra)
{
    extra["success"] = 1;
    auto resp        = HttpResponse::newHttpJsonResponse(extra);
    resp->setStatusCode(k200OK);
    return resp;
}

HttpResponsePtr Internal::makeError(int errorCode, const std::string& msg, HttpStatusCode status)
{
    Json::Value body;
    body["success"]    = 0;
    body["error_code"] = errorCode;
    body["error_msg"]  = msg;
    auto resp          = HttpResponse::newHttpJsonResponse(body);
    resp->setStatusCode(status);
    return resp;
}

// ── GET /internal/session/validate ────────────
// 网关握手时调用，验证 account_token + sso_cookie
// 返回：{"valid": true, "domain_id": "xxx", "username": "xxx"}
void Internal::validateSession(const HttpRequestPtr&                         req,
                               std::function<void(const HttpResponsePtr&)>&& callback)
{
    const std::string accountToken = req->getParameter("account_token");
    const std::string ssoCookie    = req->getParameter("sso_cookie");

    if (accountToken.empty() || ssoCookie.empty()) {
        Json::Value body;
        body["valid"]     = false;
        body["error_msg"] = "Missing account_token or sso_cookie";
        callback(HttpResponse::newHttpJsonResponse(body));
        return;
    }

    getService()->validateSession(
        accountToken,
        ssoCookie,
        [callback](const interfaces::ISystemService::SessionInfo& info) {
            // ✅ 验证成功，返回 domain_id 和 username 给 Jetty 网关
            Json::Value body;
            body["valid"]     = true;
            body["domain_id"] = info.domainId;
            body["username"]  = info.username;
            LOG_INFO << "[Internal] Session validated: username=" << info.username
                     << ", domainId=" << info.domainId;
            callback(HttpResponse::newHttpJsonResponse(body));
        },
        [callback](const std::string& msg, int /*code*/) {
            Json::Value body;
            body["valid"]     = false;
            body["error_msg"] = msg;
            LOG_WARN << "[Internal] Session validation failed: " << msg;
            callback(HttpResponse::newHttpJsonResponse(body));
        });
}

// ── POST /internal/event/publish ──────────────
// C++ 业务层产生事件后调用，转发给 Jetty 网关
// Body: {"channel": "/userdomains/.../confs/xxx", "method": "update"}
void Internal::publishEvent(const HttpRequestPtr&                         req,
                            std::function<void(const HttpResponsePtr&)>&& callback)
{
    // 解析请求体
    auto jsonBody = req->getJsonObject();
    if (!jsonBody) {
        callback(makeError(400, "Invalid JSON body"));
        return;
    }

    const std::string channel = (*jsonBody)["channel"].asString();
    const std::string method  = (*jsonBody)["method"].asString();

    if (channel.empty() || method.empty()) {
        callback(makeError(400, "Missing channel or method"));
        return;
    }
    if (method != "update" && method != "delete") {
        callback(makeError(400, "method must be 'update' or 'delete'"));
        return;
    }

    LOG_INFO << "[Internal] Publishing event: channel=" << channel << ", method=" << method;

    // 异步转发给 Jetty 网关
    auto httpClient = drogon::HttpClient::newHttpClient("http://127.0.0.1:8081");
    auto forwardReq = drogon::HttpRequest::newHttpJsonRequest(*jsonBody);
    forwardReq->setMethod(drogon::Post);
    forwardReq->setPath("/internal/event/publish");

    httpClient->sendRequest(
        forwardReq,
        [callback, channel](drogon::ReqResult result, const drogon::HttpResponsePtr& resp) {
            if (result != drogon::ReqResult::Ok) {
                LOG_ERROR << "[Internal] Forward to gateway failed, channel=" << channel;
                callback(makeError(502, "Gateway unreachable"));
                return;
            }
            LOG_INFO << "[Internal] Event forwarded to gateway: channel=" << channel;
            callback(makeSuccess());
        });
}


}   // namespace drogon
