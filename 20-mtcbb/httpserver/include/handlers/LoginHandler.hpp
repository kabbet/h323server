#ifndef LOGINHANDLER_HPP
#define LOGINHANDLER_HPP

#include <drogon/HttpResponse.h>
#include <functional>
#include <string>
#include <memory>
#include "models/Users.h"
#include "interfaces/IUserService.hpp"

namespace handlers {

class LoginHandler : public std::enable_shared_from_this<LoginHandler> {
public:
    using ResponseCallback = std::function<void(const drogon::HttpResponsePtr&)>;

    // 依赖注入
    explicit LoginHandler(std::shared_ptr<interfaces::IUserService> userService)
        : userService_(userService) {}

    static void handle(
        std::shared_ptr<interfaces::IUserService> userService,
        const std::string& userId,
        const std::string& password,
        ResponseCallback callback);

private:
    LoginHandler(
        std::shared_ptr<interfaces::IUserService> userService,
        const std::string& userId,
        const std::string& password,
        ResponseCallback callback);

    void start();
    void onUserAuthenticated(const drogon_model::myapp::Users& user);
    void onAuthenticationFailed(const std::string& error, int statusCode);
    void onTokenCreated(const std::string& token);
    void onTokenCreationFailed(const std::string& error, int statusCode);
    void sendErrorResponse(const std::string& error, int statusCode);

    std::shared_ptr<interfaces::IUserService> userService_;  // 注入的依赖
    std::string userId_;
    std::string password_;
    ResponseCallback callback_;
    drogon_model::myapp::Users user_;
};

}

#endif
