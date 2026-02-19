#ifndef ISYSTEMSERVICE_HPP
#define ISYSTEMSERVICE_HPP

#include <functional>
#include <string>

namespace interfaces {
class ISystemService {
public:
    virtual ~ISystemService() = default;
    using TokenCallback = std::function<void(const std::string& accountToken)>;
    using LoginCallback = std::function<void(const std::string& username,
        const std::string& ssoCookie)>;
    using ErrorCallback = std::function<void(const std::string& message, int errorCode)>;
    using SuccessCallback = std::function<void()>;

    /** 鉴权
     * 验证 oath_consumer_key + oauth_consumer_secret
     * 成功后返回 account_token
     */
    virtual void registerLicense(
        const std::string& consumerKey,
        const std::string& consumerSecret,
        TokenCallback onSuccess,
        ErrorCallback onError)
        = 0;

    /** 用户登陆
     * 验证 account_token有效性，再验证用户名/密码
     * 成功后生成 SSO_COOOKIE_KEY并返回
     */
    virtual void loginUser(
        const std::string& accountToken,
        const std::string& username,
        const std::string& password,
        LoginCallback onSuccess,
        ErrorCallback onError)
        = 0;

    /** Token 保活: 刷新 account_token存活时间 */
    virtual void keepAlive(
        const std::string& accountToken,
        const std::string& ssoCookie,
        SuccessCallback onSuccess,
        ErrorCallback onError)
        = 0;

    /** 校验请求携带的 account_token + SSO_COOKIE_KEY 是否合法
     * 供 Auth Filter 调用
     */
    virtual void validateSession(
        const std::string& accountToken,
        const std::string& ssoCookie,
        SuccessCallback onSuccess,
        ErrorCallback onError)
        = 0;
};
}

#endif // ISYSTEMSERVICE_HPP
