#ifndef __LHWSUTIL_ISIMPLEHTTPCLIENT_H__
#define __LHWSUTIL_ISIMPLEHTTPCLIENT_H__

#include <memory>
#include <string>
#include <unordered_map>

namespace LHWSUtilNS
{
    struct HttpRequestParams
    {
        HttpRequestParams();

        bool verbose;
    };

    class ISimpleHttpClient
    {
        public:
            ISimpleHttpClient();
            virtual ~ISimpleHttpClient();

            virtual int Get( const std::string& url, std::string& responseBody ) = 0;
            virtual int Get( const std::string& url,
                             const HttpRequestParams& params,
                             std::string& responseBody ) = 0;

            virtual int Post( const std::string& url,
                              const std::string& data,
                              const std::unordered_map< std::string, std::string >& headers,
                              std::string& responseBody ) = 0;
            virtual int Post( const std::string& url,
                              const std::string& data,
                              const std::unordered_map< std::string, std::string >& headers,
                              const HttpRequestParams& params,
                              std::string& responseBody ) = 0;

            virtual std::string UrlEscape( const std::string& data ) = 0;
    };

    class ISimpleHttpClientFactory
    {
        public:
            ISimpleHttpClientFactory();
            virtual ~ISimpleHttpClientFactory();

            virtual std::unique_ptr< ISimpleHttpClient > CreateSimpleHttpClient() const = 0;
    };

    std::shared_ptr< ISimpleHttpClientFactory > GetStandardSimpleHttpClientFactoryOnce();
}

#include <lhmiscutil/singleton.h>

namespace LHMiscUtilNS
{
    EnableClassAsSingleton( LHWSUtilNS::ISimpleHttpClientFactory, SingletonCanBeSet::WhenEmpty );
}

#endif
