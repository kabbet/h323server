#include <drogon/drogon.h>
#include "RedisUtils.hpp"
#include "TokenCleanupService.hpp"
#include "ServiceContainer.hpp"
#include <trantor/utils/Logger.h>

using namespace drogon;

int main()
{
    app().loadConfigFile("./config.json");
    app().setLogLevel(trantor::Logger::kTrace);
    app().registerBeginningAdvice([]() {
        LOG_INFO << "Application starting, initializing components...";

        // 1. 初始化 RedisUtils
        try {
            RedisUtils::instance().Initialize("./lua");
            LOG_INFO << "RedisUtils initialized successfully";
        } catch (const std::exception& e) {
            LOG_ERROR << "Failed to initialize RedisUtils: " << e.what();
            app().quit();
            return;
        }

        // 2. 初始化 Token 清理服务
        try {
            service::TokenCleanupService::instance().initialize();
            LOG_INFO << "TokenCleanupService initialized successfully";
        } catch (const std::exception& e) {
            LOG_ERROR << "Failed to initialize TokenCleanupService: " << e.what();
        }

        // 3. ✅ 移到这里：此时 DB 连接池已就绪，getDbClient() 不会崩溃
        try {
            ServiceContainer::instance().initialize();
            LOG_INFO << "ServiceContainer initialized successfully";
        } catch (const std::exception& e) {
            LOG_ERROR << "Failed to initialize ServiceContainer: " << e.what();
            app().quit();
            return;
        }
    });

    LOG_INFO << "Server starting...";
    app().run();

    return 0;
}