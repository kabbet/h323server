#include "services/SystemService.hpp"
#include <iomanip>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <optional>
#include <sstream>
#include <trantor/utils/Logger.h>

namespace services {
/** 生成 32位随机 hex token */
std::string SystemService::generateToken()
{
    unsigned char buf[16];
    RAND_bytes(buf, sizeof(buf));

    std::ostringstream oss;
    for (auto b : buf) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)b;
    }
    return oss.str();
}

/** 验证密码: （bcrypt hash比对 SHA256 */
bool SystemService::verifyPassword(const std::string& password, const std::string& hash)
{
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()),
        password.size(), digest);

    std::ostringstream oss;
    for (auto b : digest) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)b;
    }
    return oss.str() == hash;
}

/** 鉴权
 * POST /api/v1/system/token
 * Body: oauth_consumer_key = xxx&oauth_consumer_secret=xxx
 */
void SystemService::registerLicense(
    const std::string& consumerKey,
    const std::string& consumerSecret,
    TokenCallback onSuccess,
    ErrorCallback onError)
{
    // 1. 检验 License
    auto it = licenses_.find(consumerKey);
    if (it == licenses_.end() || it->second != consumerSecret) {
        LOG_WARN << "[SystemService] License validate failed key = " << consumerKey;
        onError("Invalid oauth_consumer_key or secret", 1001);
        return;
    }

    // 2. 生成 account_token
    const std::string token = generateToken();
    const std::string redisKey = accountTokenKey(token);

    // 3. 存入redis,value存入 consumerKey, 便于跟踪
    redisClient_->set(redisKey, consumerKey, [token, onSuccess, onError](bool ok) {
        if (!ok) {
            LOG_ERROR << "[SsystemService] account_Token write Redis failed";
            onError("Internal server error" , 500);
            return ;
        }

        LOG_INFO << "[SystemService] account_token generate success: " << token;
        onSuccess(token); }, kAccountTokenTTL);
}

/** 用户登陆
 * POST: /api/v1/system/login
 * Body: account_token=xxx&username=xxx&password=xxx
 */

void SystemService::loginUser(
    const std::string& accountToken,
    const std::string& username,
    const std::string& password,
    LoginCallback onSuccess,
    ErrorCallback onError)
{
    // 1. 验证account_token 是否存在于Redis
    redisClient_->get(
        accountTokenKey(accountToken),
        [this, accountToken, username, password,
            onSuccess, onError](std::optional<std::string> val) {
            if (!val.has_value()) {
                LOG_WARN << "[SystemService] account_Token invalid or expired";
                onError("Invalid or expired account_Token", 1002);
                return;
            }

            // 2. 验证用户名+密码（查数据库)
            userRepo_->findUserById(
                username,
                [this, accountToken, username, password,
                    onSuccess, onError](std::optional<drogon_model::myapp::Users> userOpt) {
                    if (!userOpt.has_value()) {
                        onError("User not found", 1003);
                        return;
                    }
                    const auto& user = *userOpt;

                    // check account status
                    if (!user.getValueOfIsActive()) {
                        onError("Account is disabled", 1004);
                        return;
                    }

                    // validate password
                    if (!verifyPassword(password, user.getValueOfPasswordHash())) {
                        onError("Invalid password", 1005);
                        return;
                    }

                    // 3. 生成 SSO_COOKIE_KEY
                    const std::string ssoCookie = generateToken();
                    // value format: accuontToken:username, For Auth Filter double validate
                    const std::string ssoValue = accountToken + ":" + username;
                    redisClient_->set(
                        ssoKey(ssoCookie),
                        ssoValue,
                        [ssoCookie, username, onSuccess, onError](bool ok) {
                            if (!ok) {
                                LOG_ERROR << "[SystemService] SSO Cookie write Redis failed";
                                onError("Internal server error", 500);
                                return;
                            }
                            LOG_INFO << "[SystemService] User Login Success: " << username;
                            onSuccess(username, ssoCookie);
                        },
                        kSsoCookieTTL);
                },
                [onError](const std::exception& e) {
                    LOG_ERROR << "[SystemService] Query user has exception: " << e.what();
                    onError("Data error", 500);
                });
        });
}

/** 保活: 刷新两个Token的TTL */
void SystemService::keepAlive(
    const std::string& accountToken,
    const std::string& ssoCookie,
    SuccessCallback onSuccess,
    ErrorCallback onError)
{
    // Refresh account_token TTL
    redisClient_->get(
        accountTokenKey(accountToken),
        [this, accountToken, ssoCookie, onSuccess, onError](std::optional<std::string> val) {
            if (!val.has_value()) {
                onError("Invalid account_Token", 1002);
                return;
            }
            redisClient_->set(
                accountTokenKey(accountToken), *val,
                [this, ssoCookie, onSuccess, onError](bool ok) {
                    if (!ok) {
                        onError("Internal server error", 500);
                        return;
                    }
                    // 刷新 sso cookie ttl
                    redisClient_->get(
                        ssoKey(ssoCookie),
                        [this, ssoCookie, onSuccess, onError](std::optional<std::string> ssoVal) {
                            if (!ssoVal.has_value()) {
                                onError("Invalid SSO cookie", 1006);
                                return;
                            }
                            redisClient_->set(
                                ssoKey(ssoCookie), *ssoVal,
                                [onSuccess, onError](bool ok) {
                                    ok ? onSuccess() : onError("Internal Server error", 500);
                                },
                                kSsoCookieTTL);
                        });
                },
                kAccountTokenTTL);
        }

    );
}

/** 会议验证: 供 Auth Filter 调用 */
void SystemService::validateSession(
    const std::string& accountToken,
    const std::string& ssoCookie,
    SuccessCallback onSuccess,
    ErrorCallback onError)
{
    redisClient_->get(
        ssoKey(ssoCookie),
        [accountToken, onSuccess, onError](std::optional<std::string> val) {
            if (!val.has_value()) {
                onError("Invalid or expired session", 401);
                return;
            }

            // 验证sso cookie 对应的accountToken一致
            // ssoValue格式: "accountToken:username"
            const auto colonPos = val->find(':');
            if (colonPos == std::string::npos) {
                onError("Corrupted session data", 500);
                return;
            }
            const std::string storedToken = val->substr(0, colonPos);
            if (storedToken != accountToken) {
                onError("Token mismatch", 401);
                return;
            }
            onSuccess();
        });
}

}