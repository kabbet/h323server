# UserService Boost.Test 单元测试套件

## 📋 目录结构

```
boost_test_project/
├── CMakeLists.txt              # CMake 配置文件
├── build.sh                    # 构建脚本
├── run_tests.sh                # 测试运行脚本
├── README.md                   # 本文档
└── tests/
    ├── test_user_service.cpp   # 主测试文件
    └── mocks/
        ├── MockUserRepository.hpp  # Mock 数据库
        ├── MockRedisClient.hpp     # Mock Redis
        └── TestHelpers.hpp         # 测试辅助工具
```

## 🚀 快速开始

### 1. 前置条件

确保已安装以下依赖：

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y \
    cmake \
    g++ \
    libboost-test-dev \
    libdrogon-dev

# CentOS/RHEL
sudo yum install -y \
    cmake \
    gcc-c++ \
    boost-devel \
    drogon-devel
```

### 2. 配置项目路径

编辑 `CMakeLists.txt`，修改第 28 行的项目路径：

```cmake
set(PROJECT_INCLUDE_DIR "/path/to/your/project/include" CACHE PATH "Project include directory")
```

将其改为你的项目实际路径，例如：

```cmake
set(PROJECT_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../../include" CACHE PATH "Project include directory")
```

### 3. 构建测试

```bash
./build.sh
```

第一次构建或清理后重新构建：

```bash
./build.sh Debug clean
```

Release 模式构建：

```bash
./build.sh Release
```

### 4. 运行测试

```bash
./run_tests.sh
```

然后选择：
- `1` - 详细输出运行所有测试
- `2` - 简洁输出运行所有测试
- `3` - 运行特定测试套件
- `7` - 列出所有测试用例

或者直接运行：

```bash
# 所有测试（详细）
./build/bin/user_service_tests --log_level=all --report_level=detailed

# 运行特定测试套件
./build/bin/user_service_tests --run_test=AuthenticateUserTests

# 运行特定测试用例
./build/bin/user_service_tests --run_test=AuthenticateUserTests/test_authenticate_user_success
```

## 📊 测试覆盖

### 1. PasswordVerificationTests (3 个测试)
- ✅ `test_verify_password_success` - 正确密码验证
- ✅ `test_verify_password_failure` - 错误密码验证
- ✅ `test_verify_password_empty` - 空密码验证

### 2. AuthenticateUserTests (5 个测试)
- ✅ `test_authenticate_user_success` - 用户认证成功
- ✅ `test_authenticate_user_not_found` - 用户不存在
- ✅ `test_authenticate_user_wrong_password` - 密码错误
- ✅ `test_authenticate_user_inactive` - 账户未激活
- ✅ `test_authenticate_user_database_error` - 数据库错误

### 3. CreateUserTokenTests (3 个测试)
- ✅ `test_create_token_success` - Token 创建成功
- ✅ `test_create_token_database_failure` - 数据库保存失败
- ✅ `test_create_token_redis_failure_but_db_success` - Redis 失败但数据库成功

### 4. ValidateTokenTests (5 个测试)
- ✅ `test_validate_token_from_redis_success` - 从 Redis 验证成功
- ✅ `test_validate_token_wrong_user` - Token 不属于该用户
- ✅ `test_validate_token_from_database_success` - 从数据库验证成功
- ✅ `test_validate_token_expired` - Token 已过期
- ✅ `test_validate_token_not_found` - Token 不存在

### 5. GetUserInfoTests (3 个测试)
- ✅ `test_get_user_info_success` - 获取用户信息成功
- ✅ `test_get_user_info_not_found` - 用户不存在
- ✅ `test_get_user_info_database_error` - 数据库错误

### 6. IntegrationTests (1 个测试)
- ✅ `test_full_authentication_flow` - 完整认证流程测试

**总计：20 个测试用例**

## 🔧 高级用法

### 只运行失败的测试

```bash
./build/bin/user_service_tests --log_level=test_suite --catch_system_errors=yes
```

### 生成测试报告

```bash
# XML 格式
./build/bin/user_service_tests \
    --log_format=XML \
    --log_sink=test_report.xml \
    --report_level=detailed

# JUnit 格式（可集成 CI/CD）
./build/bin/user_service_tests \
    --log_format=JUNIT \
    --log_sink=junit_report.xml
```

### 使用 CTest

```bash
cd build
ctest -V                    # 详细输出
ctest -R AuthenticateUser   # 运行匹配的测试
ctest --rerun-failed        # 重新运行失败的测试
```

### 内存泄漏检测

```bash
valgrind --leak-check=full ./build/bin/user_service_tests
```

### 代码覆盖率（需要 gcov）

```bash
# 重新编译，启用覆盖率
mkdir -p build_coverage
cd build_coverage
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_FLAGS="--coverage" \
      ..
make

# 运行测试
./bin/user_service_tests

# 生成报告
gcov ../source/services/UserService.cpp
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_report
```

## 📝 编写新测试

### 1. 创建新的测试套件

```cpp
BOOST_FIXTURE_TEST_SUITE(NewFeatureTests, UserServiceFixture)

BOOST_AUTO_TEST_CASE(test_new_feature) {
    BOOST_TEST_MESSAGE("测试：新功能");
    
    // 准备数据
    mockRepo->addUser(testUser);
    
    // 执行测试
    ResultCollector<SomeType> collector;
    userService->someMethod(
        "param",
        [&collector](const SomeType& result) {
            collector.setResult(result);
        },
        [&collector](const std::string& error, int code) {
            collector.setError(error, code);
        }
    );
    
    // 验证结果
    BOOST_CHECK(collector.hasResult());
    BOOST_CHECK_EQUAL(collector.getResult().someField(), expectedValue);
}

BOOST_AUTO_TEST_SUITE_END()
```

### 2. 使用参数化测试

```cpp
namespace bdata = boost::unit_test::data;

BOOST_DATA_TEST_CASE(
    test_multiple_passwords,
    bdata::make({"pass1", "pass2", "pass3"}),
    password)
{
    // 测试代码
}
```

## 🐛 调试技巧

### 1. 添加调试信息

```cpp
BOOST_TEST_MESSAGE("Current state: " << someVariable);
```

### 2. 条件断点

```cpp
if (someCondition) {
    BOOST_TEST_CHECKPOINT("Reached critical section");
}
```

### 3. 使用 GDB

```bash
gdb ./build/bin/user_service_tests
(gdb) run --run_test=SpecificTest
(gdb) bt  # 查看堆栈
```

## 🔗 持续集成示例

### GitHub Actions

```yaml
name: Unit Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      
      - name: Install Dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y libboost-test-dev libdrogon-dev
      
      - name: Build Tests
        run: ./build.sh Release
      
      - name: Run Tests
        run: |
          cd build
          ctest --output-on-failure
      
      - name: Generate Report
        run: |
          ./build/bin/user_service_tests \
            --log_format=XML \
            --log_sink=test_report.xml
      
      - name: Upload Results
        uses: actions/upload-artifact@v2
        with:
          name: test-results
          path: test_report.xml
```

## 📚 参考资料

- [Boost.Test 官方文档](https://www.boost.org/doc/libs/release/libs/test/)
- [Drogon 框架文档](https://drogon.org/)
- [Google Mock 指南](https://github.com/google/googletest/blob/main/docs/gmock_for_dummies.md)

## ❓ 常见问题

### Q: 编译时找不到 Drogon 头文件
A: 确保在 CMakeLists.txt 中正确设置了 `PROJECT_INCLUDE_DIR`

### Q: 测试运行时段错误
A: 检查 Mock 对象是否正确初始化，确保回调函数正确捕获变量

### Q: 如何跳过某些测试？
A: 使用 `BOOST_AUTO_TEST_CASE_EXPECTED_FAILURES` 或命令行参数 `--run_test=!TestName`

## 📧 支持

如有问题，请提交 Issue 或联系项目维护者。
