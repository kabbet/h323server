#include "filters/AuthFilter.hpp"
#include <trantor/utils/Logger.h>

void AuthFilter::doFilter(
    const HttpRequestPtr& req,
    FilterCallback&&      fcb,
    FilterChainCallback&& fccb)
{
    const std::string path = req->path();

    // 白名单直接放行
    if (isWhitelisted(path)) {
        fccb();
        return;
    }

    // 1. 取 account_token（URL 参数）
    const std::string accountToken = req->getParameter("account_token");
    if (accountToken.empty()) {
        fcb(makeUnauthorized("Missing account_token"));
        return;
    }

    // 2. 取 SSO_COOKIE_KEY（Cookie）
    const std::string ssoCookie = req->getCookie("SSO_COOKIE_KEY");
    if (ssoCookie.empty()) {
        fcb(makeUnauthorized("Missing SSO_COOKIE_KEY cookie"));
        return;
    }

    // 3. 异步校验 Redis 中的会话
    systemService_->validateSession(
        accountToken,
        ssoCookie,
        [fccb = std::move(fccb)]() mutable {
            // 验证通过，继续处理请求
            fccb();
        },
        [fcb = std::move(fcb)](const std::string& msg, int /*code*/) mutable {
            LOG_WARN << "[AuthFilter] 鉴权失败: " << msg;
            fcb(makeUnauthorized(msg));
        }
    );
}
