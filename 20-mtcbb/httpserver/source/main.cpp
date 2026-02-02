#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <drogon/drogon.h>
#include <trantor/utils/Logger.h>

using namespace drogon;

int main()
{
    app()
        .setLogPath(".")
        .setLogLevel(trantor::Logger::kTrace)
        .addListener("0.0.0.0", 8848)
        .setThreadNum(0)
        .registerSyncAdvice([](const HttpRequestPtr& req) -> HttpResponsePtr {
            const auto& path = req->path();
            if (path.length() == 1 && path[0] == '/') {
                auto response = HttpResponse::newHttpResponse();
                response->setBody("<p>Hello, world!</p>");
                return response;
            }
            return nullptr;
        })
        .run();
    return 0;
}
