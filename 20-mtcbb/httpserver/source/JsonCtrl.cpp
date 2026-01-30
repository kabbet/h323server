#include "JsonCtrl.hpp"
#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>

namespace drogon
{
    void JsonCtrl::asyncHandleHttpRequest(
            const HttpRequestPtr&,
            std::function<void(const HttpResponsePtr &)>&& callback
            )
    {
        Json::Value ret;
        ret["message"] = "hello, world";
        auto resp = HttpResponse::newHttpJsonResponse(ret);
        callback(resp);
    }
}
