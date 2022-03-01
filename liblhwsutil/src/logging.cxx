#include <lhwsutil/logging.h>

#include <boost/log/expressions/keyword.hpp>

BOOST_LOG_GLOBAL_LOGGER_INIT( LHWSUtilLoggerGeneric, LHWSUtilNS::LoggerType )
{
    LHWSUtilNS::LoggerType logger(
        boost::log::keywords::channel = LHWSUTIL_LOGGER_CHANNEL_NAME_GENERIC,
        boost::log::keywords::severity = LHWSUtilNS::SeverityLevel::trace );

    return logger;
}
