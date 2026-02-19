#ifndef APILEVELFILTER_HPP
#define APILEVELFILTER_HPP

#include "utils/ApiLevelContext.hpp"
#include <drogon/HttpFilter.h>
#include <trantor/utils/Logger.h>

namespace drogon {
/**
 * ApiLevelFilter
 * Responsibility:
 *  1. 读取请求头中的API-LEVEL 字段
 *  2. 解析并校验合法性 （必须为正整数)
 *  3. 将有效 level写入请求 attributes, 供后续 Controller 读取
 *  4. 本身不拦截请求，始终放行（版本路由由业务层处理)
 * 执行顺序:
 *  ApiLevelFilter -> AuthFilter -> Controller
 */

class ApiLevelFilter : public HttpFilter<ApiLevelFilter> {
public:
    void doFilter(const HttpRequestPtr& req, FilterCallback&& fcb, FilterChainCallback&& fccb) override
    {
        const std::string levelStr = req->getHeader("API-Level");
        if (levelStr.empty()) {
            // 未携带时使用默认值，不拦截
            ApiLevelContext::set(req, ApiLevelContext::kDefaultLevel);
            LOG_TRACE << "[ApiLevelFilter] did not contain API-LEVEL, use default value" << ApiLevelContext::kDefaultLevel;
            fccb();
            return;
        }

        // 解析为整数，非法值退回到默认
        int level = ApiLevelContext::kDefaultLevel;
        try {
            level = std::stoi(levelStr);
            if (level <= 0) {
                throw std::out_of_range("API-Level must be positive");
            }
        } catch (...) {
            LOG_WARN << "【ApiLevelFilter] illegal API-LEVEL value: " << levelStr
                     << ", revert to default value" << ApiLevelContext::kDefaultLevel;
            level = ApiLevelContext::kDefaultLevel;
        }
        ApiLevelContext::set(req, level);
        LOG_TRACE << "[ApiLevelFilter] API-Level=" << level
                  << " (effective=" << ApiLevelContext::effectiveLevel(level) << ")";
        fccb();
    }
};
}

#endif