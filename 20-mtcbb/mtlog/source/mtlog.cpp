#include "mtlog.hpp"

namespace MTLOG
{

    mtlog::mtlog() {}

    mtlog::~mtlog() {}

    bool mtlog::Init()
    {
        m_pSink = boost::make_shared<text_sink>();
        generateSinks();
        logging::core::get()->add_sink(m_pSink);
        setDefaultFormatter();
        setLogLvl(severity_level::normal);
        return true;
    }

    bool mtlog::generateSinks()
    {
        text_sink::locked_backend_ptr pBackend = m_pSink->locked_backend();
        shared_ptr<std::ostream> pStream(&std::clog, boost::null_deleter());
        pBackend->add_stream(pStream);

        // We can add more than one stream to the sink bcakend
        shared_ptr<std::ofstream> pStream2(new std::ofstream("mtlog.log"));
        assert(pStream2->is_open());
        pBackend->add_stream(pStream2);
        return true;
    }

    bool mtlog::setDefaultFormatter()
    {
        m_pSink->set_formatter(expr::stream
                               << expr::attr<unsigned int>("RecordID")
                               << " [" << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%d.%m.%Y %H:%M:%S.%f")
                               << "] [" << expr::attr<severity_level>("Severity")
                               << "] [" << expr::attr<boost::posix_time::time_duration>("Uptime")
                               << "] ["
                               << expr::if_(expr::has_attr("Tag"))
                                      [expr::stream << expr::attr<std::string>("Tag")
                                                    << "] ["]
                               << expr::format_named_scope("Scope", keywords::format = "%n", keywords::iteration = expr::reverse) << "] "
                               << expr::smessage);

        // RecordID
        attrs::counter<unsigned int> RecordID(1);
        logging::core::get()->add_global_attribute("RecordID", RecordID);

        // TimeStamp
        attrs::local_clock TimeStamp;
        logging::core::get()->add_global_attribute("TimeStamp", TimeStamp);

        // Add an up time stopwatch
        BOOST_LOG_SCOPED_THREAD_ATTR("Uptime", attrs::timer());

        // Scope
        attrs::named_scope Scope;
        logging::core::get()->add_thread_attribute("Scope", Scope);

        return true;
    }
    bool mtlog::setLogLvl(severity_level lvl)
    {
        m_pSink->set_filter(
            expr::attr<severity_level>("Severity").or_default(lvl) >= warning || expr::begins_with(expr::attr<std::string>("Tag").or_default(std::string()), "IMPORTANT"));
        return true;
    }

}
