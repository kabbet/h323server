#include <drogon/drogon.h>
#include "RedisUtils.hpp"
#include "TokenCleanupService.hpp"
#include <trantor/utils/Logger.h>

using namespace drogon;

int main()
{
    // 加载配置文件
    app().loadConfigFile("./config.json");
    app().setLogLevel(trantor::Logger::kTrace);
     app().registerBeginningAdvice([]() {
        LOG_INFO << "Application starting, initializing components...";

        // 初始化 RedisUtils
        try {
            RedisUtils::instance().Initialize("./lua");
            LOG_INFO << "RedisUtils initialized successfully";
        } catch (const std::exception& e) {
            LOG_ERROR << "Failed to initialize RedisUtils: " << e.what();
            app().quit();
            return;
        }

        // 初始化 Token 清理服务
        try {
            service::TokenCleanupService::instance().initialize();
            LOG_INFO << "TokenCleanupService initialized successfully";
        } catch (const std::exception& e) {
            LOG_ERROR << "Failed to initialize TokenCleanupService: " << e.what();
        }
    });
    
    LOG_INFO << "Server starting...";
    app().run();
    
    return 0;
}
