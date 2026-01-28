#include "mtlog.hpp"
#include "boost/log/attributes/named_scope.hpp"
#include "boost/log/sources/global_logger_storage.hpp"
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/severity_logger.hpp>

namespace MTLOG {
mtlog::mtlog()
    : m_initialized(false)
{}

mtlog::~mtlog() {}

bool mtlog::Init()
{
    if (m_initialized) {
        return true;   // 已经初始化过了
    }

    m_pSink = boost::make_shared<text_sink>();
    generateSinks();
    logging::core::get()->add_sink(m_pSink);
    setDefaultFormatter();
    setLogLvl(severity_level::normal);

    m_initialized = true;
    return true;
}

bool mtlog::generateSinks()
{
    text_sink::locked_backend_ptr pBackend = m_pSink->locked_backend();

    // 输出到 stderr
    shared_ptr<std::ostream> pStream(&std::clog, boost::null_deleter());
    pBackend->add_stream(pStream);

    // 输出到文件
    shared_ptr<std::ofstream> pStream2(new std::ofstream("mtlog.log", std::ios::app));
    if (!pStream2->is_open()) {
        std::cerr << "Failed to open log file: mtlog.log" << std::endl;
        return false;
    }
    pBackend->add_stream(pStream2);

    // 自动刷新
    pBackend->auto_flush(true);

    return true;
}

bool mtlog::setDefaultFormatter()
{
    m_pSink->set_formatter(
        expr::stream
        << expr::attr<unsigned int>("RecordID") << " ["
        << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
        << "] [" << expr::attr<severity_level>("Severity") << "] ["
        << expr::attr<boost::posix_time::time_duration>("Uptime") << "] ["
        << expr::if_(expr::has_attr("Tag"))[expr::stream << expr::attr<std::string>("Tag") << "] ["]
        << expr::if_(
               expr::has_attr("Function"))[expr::stream << expr::attr<std::string>("Function")]
        << "] " << expr::smessage);
    // RecordID
    attrs::counter<unsigned int> RecordID(1);
    logging::core::get()->add_global_attribute("RecordID", RecordID);
    // TimeStamp
    attrs::local_clock TimeStamp;
    logging::core::get()->add_global_attribute("TimeStamp", TimeStamp);
    // Uptime
    logging::core::get()->add_global_attribute("Uptime", attrs::timer());
    // 不需要添加 Scope 了
    return true;
}

bool mtlog::setLogLvl(severity_level lvl)
{
    m_pSink->set_filter(
        expr::attr<severity_level>("Severity").or_default(normal) >= lvl ||
        expr::begins_with(expr::attr<std::string>("Tag").or_default(std::string()), "IMPORTANT"));
    return true;
}

bool mtlog::log_test(std::string ans)
{
    BOOST_LOG_FUNC();

    BOOST_LOG_SEV(m_lg, normal) << "Test message a";
    BOOST_LOG_SEV(m_lg, notification) << "Test message b";
    BOOST_LOG_SEV(m_lg, warning) << "Test message c";
    BOOST_LOG_SEV(m_lg, error) << "Test message d";
    BOOST_LOG_SEV(m_lg, critical) << ans;

    return true;
}
}   // namespace MTLOG

