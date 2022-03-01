#ifndef __LHWSUTIL_IMPL_SIMPLEHTTPCLIENTCURL_H__
#define __LHWSUTIL_IMPL_SIMPLEHTTPCLIENTCURL_H__

#include <curl/curl.h>

#include <memory>
#include <string>

#include <lhwsutil/isimplehttpclient.h>

namespace LHWSUtilImplNS
{
    class SimpleHttpClientCurl : public LHWSUtilNS::ISimpleHttpClient
    {
        public:
            SimpleHttpClientCurl();
            ~SimpleHttpClientCurl();

            SimpleHttpClientCurl( const SimpleHttpClientCurl& other ) = delete;
            SimpleHttpClientCurl& operator=( const SimpleHttpClientCurl& other ) = delete;
            SimpleHttpClientCurl( SimpleHttpClientCurl&& other ) = delete;

            int Get( const std::string& url, std::string& responseBody );
            int Get( const std::string& url,
                     const LHWSUtilNS::HttpRequestParams& params,
                     std::string& responseBody );

            int Post( const std::string& url,
                      const std::string& data,
                      const std::unordered_map< std::string, std::string >& headers,
                      std::string& responseBody );
            int Post( const std::string& url,
                      const std::string& data,
                      const std::unordered_map< std::string, std::string >& headers,
                      const LHWSUtilNS::HttpRequestParams& params,
                      std::string& responseBody );

            std::string UrlEscape( const std::string& data );

        private:
            CURL* curl;
    };

    class SimpleHttpClientCurlFactory : public LHWSUtilNS::ISimpleHttpClientFactory
    {
        public:
            SimpleHttpClientCurlFactory();
            ~SimpleHttpClientCurlFactory();

            std::unique_ptr< LHWSUtilNS::ISimpleHttpClient > CreateSimpleHttpClient() const;
    };

    // Not thread safe, only one meant to be created per process
    class MainGlobalContextCurl
    {
        public:
            MainGlobalContextCurl();
            ~MainGlobalContextCurl();

            MainGlobalContextCurl( const MainGlobalContextCurl& other ) = delete;
            MainGlobalContextCurl& operator=( const MainGlobalContextCurl& other ) = delete;
            MainGlobalContextCurl( MainGlobalContextCurl&& other ) = delete;
    };

    class GlobalHttpClientCurlFactory : public LHWSUtilNS::ISimpleHttpClientFactory
    {
        public:
            GlobalHttpClientCurlFactory();
            ~GlobalHttpClientCurlFactory();

            GlobalHttpClientCurlFactory( const GlobalHttpClientCurlFactory& other ) = delete;
            GlobalHttpClientCurlFactory& operator=( const GlobalHttpClientCurlFactory& other ) = delete;
            GlobalHttpClientCurlFactory( GlobalHttpClientCurlFactory&& other ) = delete;

            std::unique_ptr< LHWSUtilNS::ISimpleHttpClient > CreateSimpleHttpClient() const;

        private:
            MainGlobalContextCurl globalContext;
            SimpleHttpClientCurlFactory clientCurlFactory;
    };

}

#endif
