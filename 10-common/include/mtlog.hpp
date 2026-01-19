#ifndef MTLOG_HPP
#define MTLOG_HPP

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

    // Here we define our application severity levels
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
        mtlog();
        ~mtlog();
        bool Init();
        bool setLogLvl(severity_level lvl);
        bool generateSinks();
        bool setDefaultFormatter();

    private:
        mtlog(const mtlog &) = delete;
        const mtlog operator=(const mtlog &) = delete;

    private:
        shared_ptr<text_sink> m_pSink;
        src::logger m_lg;
    };

    template <typename CharT, typename TraitsT>
    inline std::basic_ostream<CharT, TraitsT> &operator<<(std::basic_ostream<CharT, TraitsT> &strm, severity_level lvl)
    {
        static const char *const str[] =
            {
                "normal",
                "notification",
                "warning",
                "error",
                "critical"};
        if (static_cast<std::size_t>(lvl) < (sizeof(str) / sizeof(*str)))
            strm << str[lvl];
        else
            strm << static_cast<int>(lvl);
        return strm;
    }

}
#endif // MTLOG_HPP