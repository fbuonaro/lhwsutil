#ifndef __LHWSUTIL_LOGGING_H__
#define __LHWSUTIL_LOGGING_H__

#include <boost/log/attributes.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/trivial.hpp>

#define LHWSUTIL_LOGGER_CHANNEL_NAME_GENERIC    "LHWSUtil"

#define LHWSUTIL_LOGGER_RECORD_ATTR_NAME_SCOPE  "Scope"

#define LHWSUTIL_VERBOSE_LOGGING

namespace LHWSUtilNS
{
    typedef boost::log::trivial::severity_level SeverityLevel;
    typedef boost::log::sources::severity_channel_logger_mt< SeverityLevel, std::string > LoggerType;
}

// Singleton logger
BOOST_LOG_GLOBAL_LOGGER( LHWSUtilLoggerGeneric, LHWSUtilNS::LoggerType );

// call at top of every function
#define wsUtilLogSetScope( scopeName ) \
    BOOST_LOG_NAMED_SCOPE( scopeName )

#define wsUtilLogSetTag( tag ) \
    BOOST_LOG_SCOPED_THREAD_TAG( "Tag", tag )

#define wsUtilLog( msg ) \
    BOOST_LOG( LHWSUtilLoggerGeneric::get() ) << msg

#define wsUtilLogTrace( msg ) \
    BOOST_LOG_SEV( LHWSUtilLoggerGeneric::get(), LHWSUtilNS::SeverityLevel::trace ) << msg

#define wsUtilLogDebug( msg ) \
    BOOST_LOG_SEV( LHWSUtilLoggerGeneric::get(), LHWSUtilNS::SeverityLevel::debug ) << msg

#define wsUtilLogInfo( msg ) \
    BOOST_LOG_SEV( LHWSUtilLoggerGeneric::get(), LHWSUtilNS::SeverityLevel::info ) << msg

#define wsUtilLogError( msg ) \
    BOOST_LOG_SEV( LHWSUtilLoggerGeneric::get(), LHWSUtilNS::SeverityLevel::error ) << msg

#define wsUtilLogFatal( msg ) \
    BOOST_LOG_SEV( LHWSUtilLoggerGeneric::get(), LHWSUtilNS::SeverityLevel::fatal ) << msg

#define wsUtilLogWithSeverity( severity, msg ) \
    BOOST_LOG_SEV( LHWSUtilLoggerGeneric::get(), severity ) << msg

#endif
