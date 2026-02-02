#include <drogon/drogon.h>
#include <trantor/utils/Logger.h>

using namespace drogon;

int main()
{
    // 加载配置文件
    app().loadConfigFile("./config.json");
    
    LOG_INFO << "Server starting...";
    app().run();
    
    return 0;
}
