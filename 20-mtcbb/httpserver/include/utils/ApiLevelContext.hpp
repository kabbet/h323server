#ifndef APILEVELCONTEXT_HPP
#define APILEVELCONTEXT_HPP

#include <drogon/HttpRequest.h>

/**
 * ApiLevelContext
 * 将解析后的客户端 API-Level 挂载到Drogon请求的attributes 中，
 * Controller / handler 通过 getApiLevel()读取，无需重复解析请求头
 *
 * Usage:
 *  // 在 Filter 中写入
 *  ApiLevelContext::set(req, clientLevel);
 *  // 在 Controller 中写入
 *  int level = ApiLevelContext::get(req);
 */

class ApiLevelContext {
public:
    static constexpr const char* kAttrKey = "Api_level";
    static constexpr int kDefaultLevel = 1; // 默认参数
    static constexpr int kPlatformLevel = 1; // 当前平台支持

    static void set(const drogon::HttpRequestPtr& req, int level)
    {
        req->attributes()->insert(kAttrKey, level);
    }
    int get(const drogon::HttpRequestPtr& req)
    {
        auto attrs = req->attributes();
        if (!attrs->find(kAttrKey)) {
            return kDefaultLevel;
        }
        return attrs->get<int>(kAttrKey);
    }

    static int effectiveLevel(int requestLevel)
    {
        return std::min(requestLevel, kPlatformLevel);
    }
};

#endif