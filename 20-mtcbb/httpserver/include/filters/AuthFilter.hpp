#ifndef AUTHFILTER_HPP
#define AUTHFILTER_HPP

#include <drogon/HttpFilter.h>
#include "interfaces/ISystemService.hpp"
#include <memory>

using namespace drogon;

/**
 * AuthFilter：Drogon 过滤器，应用于所有需要鉴权的接口
 *
 * 放行条件：
 *   1. URL 参数中存在有效的 account_token
 *   2. Cookie 中存在有效的 SSO_COOKIE_KEY
 *   3. 两者在 Redis 中匹配一致
 *
 * 使用方式（在 Controller 中声明）：
 *   ADD_FILTER(AuthFilter);
 *
 * 白名单
 *  /api/v1/system/token
 *  /api/v1/system/version
 *  /internal/*             <- 内部接口由网络保护，不走Cookie 鉴权
 */
class AuthFilter : public HttpFilter<AuthFilter> {
public:
    explicit AuthFilter(std::shared_ptr<interfaces::ISystemService> systemService)
        : systemService_(std::move(systemService)) {}

    void doFilter(const HttpRequestPtr& req,
                  FilterCallback&&      fcb,
                  FilterChainCallback&& fccb) override;

private:
    std::shared_ptr<interfaces::ISystemService> systemService_;

    // 不需要鉴权的白名单路径
    static bool isWhitelisted(const std::string& path) {
        return path == "/api/v1/system/token"
            || path == "/api/v1/system/version"
            || path.rfind("/internal/", 0) == 0;  // /internal/* 全部放行
    }

    static HttpResponsePtr makeUnauthorized(const std::string& msg) {
        Json::Value body;
        body["success"]    = 0;
        body["error_code"] = 401;
        body["error_msg"]  = msg;
        auto resp = HttpResponse::newHttpJsonResponse(body);
        resp->setStatusCode(k401Unauthorized);
        return resp;
    }
};

#endif // AUTHFILTER_HPP
