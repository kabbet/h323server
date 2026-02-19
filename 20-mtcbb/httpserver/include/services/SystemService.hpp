#ifndef SYSTEMSERVICE_HPP
#define SYSTEMSERVICE_HPP

#include "interfaces/IRedisClient.hpp"
#include "interfaces/ISystemService.hpp"
#include "interfaces/IUserRepository.hpp"
#include <memory>
#include <unordered_map>

namespace services {
// Redis Key 前缀规范
// account_token:{token}   → value: consumerKey（软件级别，TTL: 24h）
// sso:{ssoCookie}         → value: accountToken:username（用户级别，TTL: 2h）
class SystemService : public interfaces::ISystemService {
    public:
        // License 数据源: 先使用Map替代
        using LicenseMap = std::unordered_map<std::string, std::string>; // key -> secret

        SystemService(
            std::shared_ptr<interfaces::IUserRepository> userRepo,
            std::shared_ptr<interfaces::IRedisClient>    redisClient,
            LicenseMap                                   licenses
        )
        : userRepo_(userRepo),
        redisClient_(redisClient),
        licenses_(std::move(licenses)) {}

        void registerLicense(
            const std::string& consumerKey,
            const std::string& consumerSecret,
            TokenCallback      onSuccess,
            ErrorCallback      onError
        ) override;

        void loginUser(
            const std::string& accountToken,
            const std::string& username,
            const std::string& password,
            LoginCallback      onSuccess,
            ErrorCallback      onError
        ) override;

        void keepAlive(
            const std::string& accountToken,
            const std::string& ssoCookie,
            SuccessCallback    onSuccess,
            ErrorCallback      onError
        ) override;

        void validateSession(
            const std::string& accountToken,
            const std::string& ssoCookie,
            SuccessCallback    onSuccess,
            ErrorCallback      onError
        ) override;

        // 工具方法
        static std::string generateToken();     // 生成 32 位随机 hex token
        static bool verifyPassword(const std::string& password, const std::string& hash);
    
    private:
        static constexpr int kAccountTokenTTL = 86400;  // 24-hour
        static constexpr int kSsoCookieTTL = 7200;      // 2-hour

        // Redis key 构造
        static std::string accountTokenKey(const std::string& token) {
            return "account_token:" + token;
        }

        static std::string ssoKey(const std::string& cookie) {
            return "sso:" + cookie;
        }

        std::shared_ptr<interfaces::IUserRepository> userRepo_;
        std::shared_ptr<interfaces::IRedisClient>    redisClient_;
        LicenseMap                                   licenses_;
};
}

#endif // SYSTEMSERVICE_HPP