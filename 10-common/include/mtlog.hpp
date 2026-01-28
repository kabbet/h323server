#ifndef MTLOG_HPP
#define MTLOG_HPP

#include <boost/log/sources/severity_logger.hpp>
#include <cassert>
#include <iostream>
#include <fstream>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/support/date_time.hpp>

namespace logging = boost::log;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace keywords = boost::log::keywords;

namespace MTLOG
{
    using boost::shared_ptr;
    typedef sinks::synchronous_sink<sinks::text_ostream_backend> text_sink;

    // 日志级别
    enum severity_level
    {
        normal,
        notification,
        warning,
        error,
        critical
    };

    class mtlog
    {
    public:
        // 获取单例
        static mtlog& instance() {
            static mtlog inst;
            return inst;
        }

        // 禁止拷贝和赋值
        mtlog(const mtlog&) = delete;
        mtlog& operator=(const mtlog&) = delete;

        // 初始化（必须在使用前调用一次）
        bool Init();
        
        // 设置日志级别
        bool setLogLvl(severity_level lvl);
        
        // 获取 logger（供外部使用）
        static src::severity_logger<severity_level>& getLogger() {
            return instance().m_lg;
        }

        // 测试函数
        bool log_test(std::string ans);

    private:
        mtlog();  // 私有构造函数
        ~mtlog();
        
        bool generateSinks();
        bool setDefaultFormatter();

    private:
        src::severity_logger<severity_level> m_lg;
        shared_ptr<text_sink> m_pSink;
        bool m_initialized;
    };

    // 便捷的日志宏 
    #define LOG_NORMAL   BOOST_LOG_SEV(MTLOG::mtlog::getLogger(), MTLOG::normal) \
                         << boost::log::add_value("Function", BOOST_CURRENT_FUNCTION)
    
    #define LOG_NOTIFY   BOOST_LOG_SEV(MTLOG::mtlog::getLogger(), MTLOG::notification) \
                         << boost::log::add_value("Function", BOOST_CURRENT_FUNCTION)
    
    #define LOG_WARNING  BOOST_LOG_SEV(MTLOG::mtlog::getLogger(), MTLOG::warning) \
                         << boost::log::add_value("Function", BOOST_CURRENT_FUNCTION)
    
    #define LOG_ERROR    BOOST_LOG_SEV(MTLOG::mtlog::getLogger(), MTLOG::error) \
                         << boost::log::add_value("Function", BOOST_CURRENT_FUNCTION)
    
    #define LOG_CRITICAL BOOST_LOG_SEV(MTLOG::mtlog::getLogger(), MTLOG::critical) \
                         << boost::log::add_value("Function", BOOST_CURRENT_FUNCTION)

    // severity_level 输出操作符
    template <typename CharT, typename TraitsT>
    inline std::basic_ostream<CharT, TraitsT>& operator<<(
        std::basic_ostream<CharT, TraitsT>& strm, severity_level lvl)
    {
        static const char* const str[] = {
            "normal",
            "notification",
            "warning",
            "error",
            "critical"
        };
        if (static_cast<std::size_t>(lvl) < (sizeof(str) / sizeof(*str)))
            strm << str[lvl];
        else
            strm << static_cast<int>(lvl);
        return strm;
    }

    // 可变参数日志函数
    template <typename T, typename... Args>
    void writeLog(severity_level level, T first, Args... rest)
    {
        BOOST_LOG_SEV(mtlog::getLogger(), level) << first;
        
        if constexpr (sizeof...(rest) > 0) {
            writeLog(level, rest...);
        }
    }

    // 默认使用 normal 级别
    template <typename T, typename... Args>
    void writeLog(T first, Args... rest)
    {
        writeLog(normal, first, rest...);
    }
}

#endif // MTLOG_HPP
