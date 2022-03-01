#include <curl/curl.h>

#include <stdexcept>
#include <string>

#include <lhwsutil_impl/simplehttpclientcurl.h>
#include <lhwsutil/logging.h>

namespace LHWSUtilImplNS
{
    namespace
    {
        struct curlWriteCallbackData
        {
            const std::string& url;
            std::string& dataOut;

            curlWriteCallbackData( const std::string& _url, std::string& _dataOut );

            curlWriteCallbackData() = delete;
        };

        curlWriteCallbackData::curlWriteCallbackData( const std::string& _url, std::string& _dataOut )
        :   url( _url )
        ,   dataOut( _dataOut )
        {
        }

        size_t curlWriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
        {
            bool failed = false;

            // size is always 1, ignore it

            try
            {
                if( nmemb && ptr )
                {
                    curlWriteCallbackData* callbackData( static_cast< curlWriteCallbackData* >( userdata ) );
                    callbackData->dataOut.append( ptr, nmemb );
                }
            }
            catch( const std::exception& e )
            {
                failed = true;
            }
            catch( ... )
            {
                failed = true;
            }

            if( failed )
            {
                // indicate failure by returning something other than nmemb
                if( nmemb )
                {
                    return 0;
                }
                else
                {
                    return 1;
                }
            }
            else
            {
                // return bytes read to indicate success
                return nmemb;
            }
        }

        int debug_callback( CURL *handle,
                            curl_infotype type,
                            char *data,
                            size_t size,
                            void *userptr )
        {
            try
            {
                wsUtilLogSetScope( "debug_callback" );
                if( data && size )
                {
                    std::string debugLine( data, size );
                    if( userptr )
                    {
                        std::ostringstream* oss( static_cast< std::ostringstream* >( userptr ) );
                        if( oss )
                        {
                            *oss << debugLine;
                        }
                    }
                    else
                    {
                        wsUtilLogWithSeverity( LHWSUtilNS::SeverityLevel::debug, debugLine );
                    }
                }
            }
            catch( ... )
            {
            }

            return 0;
        }
    }

    SimpleHttpClientCurl::SimpleHttpClientCurl()
    :   LHWSUtilNS::ISimpleHttpClient()
    ,   curl( nullptr )
    {
        curl = curl_easy_init();
        if( !( curl ) )
        {
            throw std::runtime_error( "curl_easy_init failed" );
        }
    }

    SimpleHttpClientCurl::~SimpleHttpClientCurl()
    {
        if( curl )
        {
            curl_easy_cleanup( curl );
            curl = nullptr;
        }
    }

    int SimpleHttpClientCurl::Get( const std::string& url, std::string& responseBody )
    {
        LHWSUtilNS::HttpRequestParams params;

        return Get( url, params, responseBody );
    }

    int SimpleHttpClientCurl::Get( const std::string& url,
                                   const LHWSUtilNS::HttpRequestParams& params,
                                   std::string& responseBody )
    {
        wsUtilLogSetScope( "SimpleHttpClientCurl.Get" );

        CURLcode rc;
        int ret = 0;
        std::string dataStr;
        curlWriteCallbackData callbackData( url, dataStr );
    
        if( !( curl ) )
        {
            throw std::runtime_error( "curl is null" );
        }

        curl_easy_setopt( curl, CURLOPT_URL, url.c_str() );
        curl_easy_setopt( curl, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, curlWriteCallback );
        curl_easy_setopt( curl, CURLOPT_WRITEDATA, &callbackData );

        LHWSUtilNS::SeverityLevel logLevel( LHWSUtilNS::SeverityLevel::debug );
        std::ostringstream debugOutput;

        if( params.verbose )
        {
            curl_easy_setopt( curl, CURLOPT_VERBOSE, 1L );
            curl_easy_setopt( curl, CURLOPT_DEBUGFUNCTION, debug_callback );
            curl_easy_setopt( curl, CURLOPT_DEBUGDATA, &debugOutput );

            logLevel = LHWSUtilNS::SeverityLevel::info;
        }

        wsUtilLogWithSeverity( logLevel, "getting url=[" << url << "], rc=" << rc );
        rc = curl_easy_perform( curl );
        if( params.verbose )
        {
            wsUtilLogWithSeverity( logLevel, "curl trace[" << debugOutput.str() << "]" );
        }
        curl_easy_reset( curl );

        if( rc == CURLE_OK )
        {
            responseBody = std::move( dataStr );
            ret = 0;
        }
        else
        {
            wsUtilLogWithSeverity( LHWSUtilNS::SeverityLevel::info,
                                   "failed to get url=[" << url << "], rc=" << rc );
            ret = 1;
        }

        return ret;
    }

    namespace
    {
        class CurlSlist
        {
            public:
                CurlSlist();
                ~CurlSlist();

                CurlSlist( const CurlSlist& other ) = delete;
                CurlSlist& operator=( const CurlSlist& other ) = delete;
                CurlSlist( CurlSlist&& other ) = delete;

                void Append( const std::string& val );
                struct curl_slist* Get();

            private:
                struct curl_slist* curlSList;
        };

        CurlSlist::CurlSlist()
        :   curlSList( nullptr )
        {
        }

        CurlSlist::~CurlSlist()
        {
            if( curlSList )
            {
                curl_slist_free_all( curlSList );
                curlSList = nullptr;
            }
        }

        void CurlSlist::Append( const std::string& val )
        {
            curlSList = curl_slist_append( curlSList, val.c_str() );
        }

        struct curl_slist* CurlSlist::Get()
        {
            return curlSList;
        }
    }

    int SimpleHttpClientCurl::Post( const std::string& url,
                                    const std::string& data,
                                    const std::unordered_map< std::string, std::string >& headers,
                                    std::string& responseBody )
    {
        LHWSUtilNS::HttpRequestParams params;

        return Post( url, data, headers, params, responseBody );
    }

    int SimpleHttpClientCurl::Post( const std::string& url,
                                    const std::string& data,
                                    const std::unordered_map< std::string, std::string >& headers,
                                    const LHWSUtilNS::HttpRequestParams& params,
                                    std::string& responseBody )
    {
        wsUtilLogSetScope( "SimpleHttpClientCurl.Get" );

        CURLcode rc;
        int ret = 0;
        std::string dataStr;
        curlWriteCallbackData callbackData( url, dataStr );
    
        if( !( curl ) )
        {
            throw std::runtime_error( "curl is null" );
        }

        curl_easy_setopt( curl, CURLOPT_URL, url.c_str() );
        curl_easy_setopt( curl, CURLOPT_POST, 1L);
        curl_easy_setopt( curl, CURLOPT_POSTFIELDSIZE, data.size() );
        curl_easy_setopt( curl, CURLOPT_POSTFIELDS, data.c_str() );
        curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, curlWriteCallback );
        curl_easy_setopt( curl, CURLOPT_WRITEDATA, &callbackData );

        std::ostringstream debugOutput;
        LHWSUtilNS::SeverityLevel logLevel( LHWSUtilNS::SeverityLevel::debug );

        if( params.verbose )
        {
            curl_easy_setopt( curl, CURLOPT_VERBOSE, 1L );
            curl_easy_setopt( curl, CURLOPT_DEBUGFUNCTION, debug_callback );
            curl_easy_setopt( curl, CURLOPT_DEBUGDATA, &debugOutput );

            logLevel = LHWSUtilNS::SeverityLevel::info;
        }

        CurlSlist curlSList;
        for( auto it = headers.cbegin(); it != headers.cend(); ++it )
        {
            std::ostringstream oss;

            oss << it->first << ": " << it->second;

            curlSList.Append( oss.str() );
        }

        if( curlSList.Get() ) // <=> Append was called
        {
            curl_easy_setopt( curl, CURLOPT_HTTPHEADER, curlSList.Get() );
        }

        wsUtilLogWithSeverity( logLevel, "posting data=[" << data << "] to url=[" << url << "]" );
        rc = curl_easy_perform( curl );
        if( params.verbose )
        {
            wsUtilLogWithSeverity( logLevel, "curl trace[" << debugOutput.str() << "]" );
        }
        curl_easy_reset( curl );

        if( rc == CURLE_OK )
        {
            responseBody = std::move( dataStr );
            ret = 0;
        }
        else
        {
            wsUtilLogWithSeverity( LHWSUtilNS::SeverityLevel::info,
                                   "failed to post url=[" << url << "], rc=" << rc );
            ret = 1;
        }

        return ret;
    }

    std::string SimpleHttpClientCurl::UrlEscape( const std::string& data )
    {
        if( !( curl ) )
        {
            throw std::runtime_error( "curl is null" );
        }

        char* escapedData = nullptr;
        std::string escapedStr;

        try
        {
            escapedData = curl_easy_escape( curl, data.c_str(), data.size() );
            if( escapedData )
            {
                escapedStr.assign( escapedData, strlen( escapedData ) );    

                curl_free( escapedData );
                escapedData = nullptr;
            }
        }
        catch( ... )
        {
            if( escapedData )
            {
                curl_free( escapedData );
                escapedData = nullptr;
            }

            throw;
        }

        return escapedStr;
    }

    SimpleHttpClientCurlFactory::SimpleHttpClientCurlFactory()
    :   LHWSUtilNS::ISimpleHttpClientFactory()
    {
    }

    SimpleHttpClientCurlFactory::~SimpleHttpClientCurlFactory()
    {
    }

    std::unique_ptr< LHWSUtilNS::ISimpleHttpClient > SimpleHttpClientCurlFactory::CreateSimpleHttpClient() const
    {
        return std::unique_ptr< LHWSUtilNS::ISimpleHttpClient >( new SimpleHttpClientCurl() );
    }

    MainGlobalContextCurl::MainGlobalContextCurl()
    {
        curl_global_init(CURL_GLOBAL_ALL);
    }

    MainGlobalContextCurl::~MainGlobalContextCurl()
    {
        curl_global_cleanup();
    }

    GlobalHttpClientCurlFactory::GlobalHttpClientCurlFactory()
    :   globalContext()
    ,   clientCurlFactory()
    {
    }

    GlobalHttpClientCurlFactory::~GlobalHttpClientCurlFactory()
    {
    }

    std::unique_ptr< LHWSUtilNS::ISimpleHttpClient > GlobalHttpClientCurlFactory::CreateSimpleHttpClient() const
    {
        return clientCurlFactory.CreateSimpleHttpClient();
    }
}

namespace LHWSUtilNS
{
    std::shared_ptr< ISimpleHttpClientFactory > GetStandardSimpleHttpClientFactoryOnce()
    {
        static bool initialized( false );

        if( initialized )
        {
            throw std::runtime_error( "standard http client factory should only be created once" );
        }
        else
        {
            initialized = true;
            return std::make_shared< LHWSUtilImplNS::GlobalHttpClientCurlFactory >();
        }
    }
}
