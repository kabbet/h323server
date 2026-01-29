#include <drogon/HttpAppFramework.h>
#include "RestfulApi.hpp"

int main() {
    // 配置 Drogon
    drogon::app().addListener("0.0.0.0", 8080);
    drogon::app().setLogLevel(trantor::Logger::kDebug);
    
    // 启动应用
    drogon::app().run();
    return 0;
}
