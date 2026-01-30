#include "RestfulApi.hpp"
#include <drogon/CacheMap.h>
#include <drogon/HttpRequest.h>
#include <drogon/HttpSimpleController.h>
#include <drogon/HttpResponse.h>

namespace drogon
{
    void BenchmarkCtrl::asyncHandleHttpRequest(
            const HttpRequestPtr &,
            std::function<void(const HttpResponsePtr &)> &&callback)
    {
        auto resp = HttpResponse::newHttpResponse();
        resp->setBody("<p>Hello, world!</p>");
        resp->setExpiredTime(0);
        callback(resp);
    }
}
