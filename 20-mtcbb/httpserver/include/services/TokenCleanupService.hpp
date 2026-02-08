// include/TokenCleanupService.hpp
#ifndef TOKENCLEANUPSERVICE_HPP
#define TOKENCLEANUPSERVICE_HPP

#include <string>
#include <drogon/HttpAppFramework.h>
#include "models/UserTokens.h"

namespace service {

class TokenCleanupService {
public:
    static TokenCleanupService& instance();
    
    // 初始化清理服务（在应用启动时调用）
    void initialize();
    
    // 处理 token 过期事件
    void handleTokenExpiration(const std::string& token);
    
private:
    TokenCleanupService() = default;
    
    // 从 PostgreSQL 删除过期的 token
    void deleteTokenFromDatabase(const std::string& token);
};

} // namespace service

#endif
