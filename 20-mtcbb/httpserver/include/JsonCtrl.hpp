#ifndef JSONCTRL_HPP
#define JSONCTRL_HPP

#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpSimpleController.h>

namespace drogon
{
    class JsonCtrl : public HttpSimpleController<JsonCtrl>
    {
        public:
            virtual void asyncHandleHttpRequest(
                    const HttpRequestPtr& req,
                    std::function<void(const HttpResponsePtr&)> && callback
                    ) override;
            PATH_LIST_BEGIN
            PATH_ADD("/json", Get);
            PATH_LIST_END
    };
}

#endif
