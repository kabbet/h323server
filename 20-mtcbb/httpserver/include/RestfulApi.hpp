#ifndef RESTFULAPI_HPP
#define RESTFULAPI_HPP

#include <drogon/HttpSimpleController.h>

namespace drogon {
class BenchmarkCtrl : public HttpSimpleController<BenchmarkCtrl>
{
    virtual void asyncHandleHttpRequest(
        const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) override;
    PATH_LIST_BEGIN
    PATH_ADD("benchmark", Get);
    PATH_LIST_END
};
}   // namespace drogon


#endif
