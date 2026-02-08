#define BOOST_TEST_MODULE UserServiceTests
#include <boost/test/included/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "services/UserService.hpp"
#include "mocks/MockUserRepository.hpp"
#include "mocks/MockRedisClient.hpp"
#include "mocks/TestHelpers.hpp"

#include <memory>
#include <string>

using namespace services;
using namespace mocks;
using namespace test_helpers;

// ============================================================================
// 测试夹具：为每个测试用例提供干净的环境
// ============================================================================

struct UserServiceFixture {
    UserServiceFixture() {
        // 创建 Mock 对象
        mockRepo = std::make_shared<MockUserRepository>();
        mockRedis = std::make_shared<MockRedisClient>();
        
        // 创建 UserService 实例
        userService = std::make_shared<UserService>(mockRepo, mockRedis);
        
        // 准备测试数据
        testUser = createTestUser("user123", "password123", true);
        inactiveUser = createTestUser("inactive_user", "password123", false);
    }
    
    ~UserServiceFixture() {
        // 清理
        mockRepo->clear();
        mockRedis->clear();
    }
    
    std::shared_ptr<MockUserRepository> mockRepo;
    std::shared_ptr<MockRedisClient> mockRedis;
    std::shared_ptr<UserService> userService;
    
    drogon_model::myapp::Users testUser;
    drogon_model::myapp::Users inactiveUser;
};

// ============================================================================
// 1. 测试静态方法：verifyPassword
// ============================================================================

BOOST_AUTO_TEST_SUITE(PasswordVerificationTests)

BOOST_AUTO_TEST_CASE(test_verify_password_success) {
    BOOST_TEST_MESSAGE("测试：正确的密码验证");
    
    std::string password = "mypassword";
    std::string hash = "hash_mypassword";
    
    bool result = UserService::verifyPassword(password, hash);
    BOOST_CHECK(result == true);
}

BOOST_AUTO_TEST_CASE(test_verify_password_failure) {
    BOOST_TEST_MESSAGE("测试：错误的密码验证");
    
    std::string password = "wrongpassword";
    std::string hash = "hash_correctpassword";
    
    bool result = UserService::verifyPassword(password, hash);
    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_CASE(test_verify_password_empty) {
    BOOST_TEST_MESSAGE("测试：空密码验证");
    
    std::string password = "";
    std::string hash = "hash_";
    
    bool result = UserService::verifyPassword(password, hash);
    BOOST_CHECK(result == true);
}

BOOST_AUTO_TEST_SUITE_END()

// ============================================================================
// 2. 测试用户认证：authenticateUser
// ============================================================================

BOOST_FIXTURE_TEST_SUITE(AuthenticateUserTests, UserServiceFixture)

BOOST_AUTO_TEST_CASE(test_authenticate_user_success) {
    BOOST_TEST_MESSAGE("测试：用户认证成功");
    
    // 准备数据
    mockRepo->addUser(testUser);
    
    // 执行测试
    ResultCollector<drogon_model::myapp::Users> collector;
    
    userService->authenticateUser(
        "user123",
        "password123",
        [&collector](const drogon_model::myapp::Users& user) {
            collector.setResult(user);
        },
        [&collector](const std::string& error, int code) {
            collector.setError(error, code);
        }
    );
    
    // 验证结果
    BOOST_CHECK(collector.hasResult());
    BOOST_CHECK(!collector.hasError());
    BOOST_CHECK_EQUAL(collector.getResult().getValueOfUserId(), "user123");
}

BOOST_AUTO_TEST_CASE(test_authenticate_user_not_found) {
    BOOST_TEST_MESSAGE("测试：用户不存在");
    
    // 不添加用户到 Mock Repository
    
    ResultCollector<drogon_model::myapp::Users> collector;
    
    userService->authenticateUser(
        "nonexistent",
        "password123",
        [&collector](const drogon_model::myapp::Users& user) {
            collector.setResult(user);
        },
        [&collector](const std::string& error, int code) {
            collector.setError(error, code);
        }
    );
    
    // 验证结果
    BOOST_CHECK(!collector.hasResult());
    BOOST_CHECK(collector.hasError());
    BOOST_CHECK_EQUAL(collector.getErrorCode(), 401);
    BOOST_CHECK_EQUAL(collector.getError(), "Invalid userId or password");
}

BOOST_AUTO_TEST_CASE(test_authenticate_user_wrong_password) {
    BOOST_TEST_MESSAGE("测试：密码错误");
    
    mockRepo->addUser(testUser);
    
    ResultCollector<drogon_model::myapp::Users> collector;
    
    userService->authenticateUser(
        "user123",
        "wrongpassword",
        [&collector](const drogon_model::myapp::Users& user) {
            collector.setResult(user);
        },
        [&collector](const std::string& error, int code) {
            collector.setError(error, code);
        }
    );
    
    // 验证结果
    BOOST_CHECK(!collector.hasResult());
    BOOST_CHECK(collector.hasError());
    BOOST_CHECK_EQUAL(collector.getErrorCode(), 401);
    BOOST_CHECK_EQUAL(collector.getError(), "Invalid userId or password");
}

BOOST_AUTO_TEST_CASE(test_authenticate_user_inactive) {
    BOOST_TEST_MESSAGE("测试：用户账户未激活");
    
    mockRepo->addUser(inactiveUser);
    
    ResultCollector<drogon_model::myapp::Users> collector;
    
    userService->authenticateUser(
        "inactive_user",
        "password123",
        [&collector](const drogon_model::myapp::Users& user) {
            collector.setResult(user);
        },
        [&collector](const std::string& error, int code) {
            collector.setError(error, code);
        }
    );
    
    // 验证结果
    BOOST_CHECK(!collector.hasResult());
    BOOST_CHECK(collector.hasError());
    BOOST_CHECK_EQUAL(collector.getErrorCode(), 403);
    BOOST_CHECK_EQUAL(collector.getError(), "User account is inactive");
}

BOOST_AUTO_TEST_CASE(test_authenticate_user_database_error) {
    BOOST_TEST_MESSAGE("测试：数据库错误");
    
    mockRepo->setFindUserShouldFail(true);
    
    ResultCollector<drogon_model::myapp::Users> collector;
    
    userService->authenticateUser(
        "user123",
        "password123",
        [&collector](const drogon_model::myapp::Users& user) {
            collector.setResult(user);
        },
        [&collector](const std::string& error, int code) {
            collector.setError(error, code);
        }
    );
    
    // 验证结果
    BOOST_CHECK(!collector.hasResult());
    BOOST_CHECK(collector.hasError());
    BOOST_CHECK_EQUAL(collector.getErrorCode(), 500);
    BOOST_CHECK_EQUAL(collector.getError(), "Database error");
}

BOOST_AUTO_TEST_SUITE_END()

// ============================================================================
// 3. 测试 Token 创建：createUserToken
// ============================================================================

BOOST_FIXTURE_TEST_SUITE(CreateUserTokenTests, UserServiceFixture)

BOOST_AUTO_TEST_CASE(test_create_token_success) {
    BOOST_TEST_MESSAGE("测试：Token 创建成功");
    
    ResultCollector<std::string> collector;
    
    userService->createUserToken(
        "user123",
        [&collector](const std::string& token) {
            collector.setResult(token);
        },
        [&collector](const std::string& error, int code) {
            collector.setError(error, code);
        }
    );
    
    // 验证结果
    BOOST_CHECK(collector.hasResult());
    BOOST_CHECK(!collector.hasError());
    BOOST_CHECK(!collector.getResult().empty());
    
    // 验证 Token 被保存到数据库和 Redis
    BOOST_CHECK(mockRepo->hasToken(collector.getResult()));
    BOOST_CHECK(mockRedis->hasToken(collector.getResult()));
}

BOOST_AUTO_TEST_CASE(test_create_token_database_failure) {
    BOOST_TEST_MESSAGE("测试：数据库保存失败");
    
    mockRepo->setSaveTokenShouldFail(true);
    
    ResultCollector<std::string> collector;
    
    userService->createUserToken(
        "user123",
        [&collector](const std::string& token) {
            collector.setResult(token);
        },
        [&collector](const std::string& error, int code) {
            collector.setError(error, code);
        }
    );
    
    // 验证结果
    BOOST_CHECK(!collector.hasResult());
    BOOST_CHECK(collector.hasError());
    BOOST_CHECK_EQUAL(collector.getErrorCode(), 500);
    BOOST_CHECK_EQUAL(collector.getError(), "Failed to create session");
}

BOOST_AUTO_TEST_CASE(test_create_token_redis_failure_but_db_success) {
    BOOST_TEST_MESSAGE("测试：Redis 保存失败但数据库成功");
    
    mockRedis->setShouldFail(true);
    
    ResultCollector<std::string> collector;
    
    userService->createUserToken(
        "user123",
        [&collector](const std::string& token) {
            collector.setResult(token);
        },
        [&collector](const std::string& error, int code) {
            collector.setError(error, code);
        }
    );
    
    // 验证结果 - 即使 Redis 失败，仍然返回成功
    BOOST_CHECK(collector.hasResult());
    BOOST_CHECK(!collector.hasError());
    BOOST_CHECK(!collector.getResult().empty());
    
    // 验证只保存到数据库
    BOOST_CHECK(mockRepo->hasToken(collector.getResult()));
    BOOST_CHECK(!mockRedis->hasToken(collector.getResult()));
}

BOOST_AUTO_TEST_SUITE_END()

// ============================================================================
// 4. 测试 Token 验证：validateToken
// ============================================================================

BOOST_FIXTURE_TEST_SUITE(ValidateTokenTests, UserServiceFixture)

BOOST_AUTO_TEST_CASE(test_validate_token_from_redis_success) {
    BOOST_TEST_MESSAGE("测试：从 Redis 验证 Token 成功");
    
    // 预设 Redis 中的 Token
    mockRedis->setTokenData("valid_token", "user123");
    
    SimpleResultCollector collector;
    
    userService->validateToken(
        "valid_token",
        "user123",
        [&collector]() {
            collector.setSuccess();
        },
        [&collector](const std::string& error, int code) {
            collector.setError(error, code);
        }
    );
    
    // 验证结果
    BOOST_CHECK(collector.hasSuccess());
    BOOST_CHECK(!collector.hasError());
}

BOOST_AUTO_TEST_CASE(test_validate_token_wrong_user) {
    BOOST_TEST_MESSAGE("测试：Token 不属于该用户");
    
    mockRedis->setTokenData("valid_token", "user123");
    
    SimpleResultCollector collector;
    
    userService->validateToken(
        "valid_token",
        "user456",  // 不同的用户
        [&collector]() {
            collector.setSuccess();
        },
        [&collector](const std::string& error, int code) {
            collector.setError(error, code);
        }
    );
    
    // 验证结果
    BOOST_CHECK(!collector.hasSuccess());
    BOOST_CHECK(collector.hasError());
    BOOST_CHECK_EQUAL(collector.getErrorCode(), 403);
    BOOST_CHECK_EQUAL(collector.getError(), "Unauthorized");
}

BOOST_AUTO_TEST_CASE(test_validate_token_from_database_success) {
    BOOST_TEST_MESSAGE("测试：Redis 中没有但数据库中有效");
    
    // 不在 Redis 中，但在数据库中
    auto token = createTestToken("db_token", "user123", 3600);
    mockRepo->addToken(token);
    
    SimpleResultCollector collector;
    
    userService->validateToken(
        "db_token",
        "user123",
        [&collector]() {
            collector.setSuccess();
        },
        [&collector](const std::string& error, int code) {
            collector.setError(error, code);
        }
    );
    
    // 验证结果
    BOOST_CHECK(collector.hasSuccess());
    BOOST_CHECK(!collector.hasError());
}

BOOST_AUTO_TEST_CASE(test_validate_token_expired) {
    BOOST_TEST_MESSAGE("测试：Token 已过期");
    
    // 创建过期的 Token
    auto expiredToken = createExpiredToken("expired_token", "user123");
    mockRepo->addToken(expiredToken);
    
    SimpleResultCollector collector;
    
    userService->validateToken(
        "expired_token",
        "user123",
        [&collector]() {
            collector.setSuccess();
        },
        [&collector](const std::string& error, int code) {
            collector.setError(error, code);
        }
    );
    
    // 验证结果
    BOOST_CHECK(!collector.hasSuccess());
    BOOST_CHECK(collector.hasError());
    BOOST_CHECK_EQUAL(collector.getErrorCode(), 401);
    BOOST_CHECK_EQUAL(collector.getError(), "Token expired");
}

BOOST_AUTO_TEST_CASE(test_validate_token_not_found) {
    BOOST_TEST_MESSAGE("测试：Token 不存在");
    
    SimpleResultCollector collector;
    
    userService->validateToken(
        "nonexistent_token",
        "user123",
        [&collector]() {
            collector.setSuccess();
        },
        [&collector](const std::string& error, int code) {
            collector.setError(error, code);
        }
    );
    
    // 验证结果
    BOOST_CHECK(!collector.hasSuccess());
    BOOST_CHECK(collector.hasError());
    BOOST_CHECK_EQUAL(collector.getErrorCode(), 401);
    BOOST_CHECK_EQUAL(collector.getError(), "Invalid or expired token");
}

BOOST_AUTO_TEST_SUITE_END()

// ============================================================================
// 5. 测试获取用户信息：getUserInfo
// ============================================================================

BOOST_FIXTURE_TEST_SUITE(GetUserInfoTests, UserServiceFixture)

BOOST_AUTO_TEST_CASE(test_get_user_info_success) {
    BOOST_TEST_MESSAGE("测试：获取用户信息成功");
    
    mockRepo->addUser(testUser);
    
    ResultCollector<drogon_model::myapp::Users> collector;
    
    userService->getUserInfo(
        "user123",
        [&collector](const drogon_model::myapp::Users& user) {
            collector.setResult(user);
        },
        [&collector](const std::string& error, int code) {
            collector.setError(error, code);
        }
    );
    
    // 验证结果
    BOOST_CHECK(collector.hasResult());
    BOOST_CHECK(!collector.hasError());
    BOOST_CHECK_EQUAL(collector.getResult().getValueOfUserId(), "user123");
    BOOST_CHECK_EQUAL(collector.getResult().getValueOfUsername(), "TestUser_user123");
}

BOOST_AUTO_TEST_CASE(test_get_user_info_not_found) {
    BOOST_TEST_MESSAGE("测试：用户不存在");
    
    ResultCollector<drogon_model::myapp::Users> collector;
    
    userService->getUserInfo(
        "nonexistent",
        [&collector](const drogon_model::myapp::Users& user) {
            collector.setResult(user);
        },
        [&collector](const std::string& error, int code) {
            collector.setError(error, code);
        }
    );
    
    // 验证结果
    BOOST_CHECK(!collector.hasResult());
    BOOST_CHECK(collector.hasError());
    BOOST_CHECK_EQUAL(collector.getErrorCode(), 404);
    BOOST_CHECK_EQUAL(collector.getError(), "User not found");
}

BOOST_AUTO_TEST_CASE(test_get_user_info_database_error) {
    BOOST_TEST_MESSAGE("测试：数据库错误");
    
    mockRepo->setFindUserShouldFail(true);
    
    ResultCollector<drogon_model::myapp::Users> collector;
    
    userService->getUserInfo(
        "user123",
        [&collector](const drogon_model::myapp::Users& user) {
            collector.setResult(user);
        },
        [&collector](const std::string& error, int code) {
            collector.setError(error, code);
        }
    );
    
    // 验证结果
    BOOST_CHECK(!collector.hasResult());
    BOOST_CHECK(collector.hasError());
    BOOST_CHECK_EQUAL(collector.getErrorCode(), 500);
    BOOST_CHECK_EQUAL(collector.getError(), "Database error");
}

BOOST_AUTO_TEST_SUITE_END()

// ============================================================================
// 6. 综合场景测试
// ============================================================================

BOOST_FIXTURE_TEST_SUITE(IntegrationTests, UserServiceFixture)

BOOST_AUTO_TEST_CASE(test_full_authentication_flow) {
    BOOST_TEST_MESSAGE("测试：完整的认证流程");
    
    // 1. 添加用户
    mockRepo->addUser(testUser);
    
    // 2. 认证用户
    ResultCollector<drogon_model::myapp::Users> authCollector;
    userService->authenticateUser(
        "user123",
        "password123",
        [&authCollector](const drogon_model::myapp::Users& user) {
            authCollector.setResult(user);
        },
        [&authCollector](const std::string& error, int code) {
            authCollector.setError(error, code);
        }
    );
    
    BOOST_REQUIRE(authCollector.hasResult());
    
    // 3. 创建 Token
    ResultCollector<std::string> tokenCollector;
    userService->createUserToken(
        "user123",
        [&tokenCollector](const std::string& token) {
            tokenCollector.setResult(token);
        },
        [&tokenCollector](const std::string& error, int code) {
            tokenCollector.setError(error, code);
        }
    );
    
    BOOST_REQUIRE(tokenCollector.hasResult());
    std::string token = tokenCollector.getResult();
    
    // 4. 验证 Token
    SimpleResultCollector validateCollector;
    userService->validateToken(
        token,
        "user123",
        [&validateCollector]() {
            validateCollector.setSuccess();
        },
        [&validateCollector](const std::string& error, int code) {
            validateCollector.setError(error, code);
        }
    );
    
    BOOST_CHECK(validateCollector.hasSuccess());
    
    // 5. 获取用户信息
    ResultCollector<drogon_model::myapp::Users> infoCollector;
    userService->getUserInfo(
        "user123",
        [&infoCollector](const drogon_model::myapp::Users& user) {
            infoCollector.setResult(user);
        },
        [&infoCollector](const std::string& error, int code) {
            infoCollector.setError(error, code);
        }
    );
    
    BOOST_CHECK(infoCollector.hasResult());
    BOOST_CHECK_EQUAL(infoCollector.getResult().getValueOfUserId(), "user123");
}

BOOST_AUTO_TEST_SUITE_END()
