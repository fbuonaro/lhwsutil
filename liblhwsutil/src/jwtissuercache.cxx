#include <rapidjson/document.h>

#include <lhwsutil_impl/jwtissuercache.h>
#include <lhwsutil_impl/jwtutils.h>

#include <lhwsutil/isimplehttpclient.h>
#include <lhwsutil/logging.h>

#include <lhsslutil/base64.h>

#include <stdexcept>

namespace LHWSUtilImplNS
{
    JwtIssuer::JwtIssuer( const std::string& _url )
        : LHWSUtilNS::IJwtIssuer()
        , url( _url )
        , algToKeyPem()
        , clientAuthzBearerToken()
    {
    }

    JwtIssuer::~JwtIssuer()
    {
    }

    const std::string& JwtIssuer::GetUrl() const
    {
        return url;
    }

    bool JwtIssuer::AlgIsSupported( const std::string& alg ) const
    {
        if ( algToKeyPem.find( alg ) != algToKeyPem.cend() )
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    const std::string& JwtIssuer::GetKeyPemForAlg( const std::string& alg ) const
    {
        auto it = algToKeyPem.find( alg );
        if ( it != algToKeyPem.cend() )
        {
            return it->second;
        }
        else
        {
            std::ostringstream oss;
            oss << "alg=[" << alg << "] unsupported by issuer=[" << url << "]";
            throw std::runtime_error( oss.str() );
        }
    }

    const std::string& JwtIssuer::GetClientAuthzBearerToken() const
    {
        return clientAuthzBearerToken;
    }

    void JwtIssuer::SetKeyPemForAlg( const std::string& alg, const std::string& keyPem )
    {
        wsUtilLogSetScope( "JwtIssuerCache.SetKeyPem" );

        wsUtilLogTrace( "iss=" << url << " setting alg=" << alg << " to key=" << keyPem );

        algToKeyPem.emplace( alg, keyPem );
    }

    void JwtIssuer::SetClientAuthzBearerToken( const std::string& _clientAuthzBearerToken )
    {
        clientAuthzBearerToken = _clientAuthzBearerToken;
    }

    const std::string& JwtIssuer::GetOpenIdConfiguration() const
    {
        return openIdConfiguration;
    }

    void JwtIssuer::SetOpenIdConfiguration( const std::string _openIdConfiguration )
    {
        openIdConfiguration = _openIdConfiguration;
    }

    JwtIssuerCache::JwtIssuerCache()
        : LHWSUtilNS::IJwtIssuerCache()
        , cacheMutex()
        , issToJwtIssuer()
        , pendingIssToCacheParams()
    {
    }

    JwtIssuerCache::~JwtIssuerCache()
    {
    }

    void JwtIssuerCache::LoadIssuer( const LHWSUtilNS::JwtIssuerCacheParams& cacheParams )
    {
        const std::lock_guard<std::mutex> lock( cacheMutex );

        if ( cacheParams.iss.empty() )
        {
            throw std::runtime_error( "cacheParams.iss is empty" );
        }

        auto it = issToJwtIssuer.find( cacheParams.iss );
        if ( it != issToJwtIssuer.end() )
        {
            std::ostringstream oss;

            oss << "issuer=[" << cacheParams.iss << "] is already in the cache";

            throw std::runtime_error( oss.str() );
        }

        int rc = reloadIssuer( cacheParams );
        if ( rc != 0 )
        {
            pendingIssToCacheParams.emplace( cacheParams.iss, cacheParams );
        }
    }

    // assume lock held
    int JwtIssuerCache::reloadIssuer( const LHWSUtilNS::JwtIssuerCacheParams& cacheParams )
    {
        wsUtilLogSetScope( "JwtIssuerCache.reloadIssuer" );

        int ret = 0;
        std::unordered_set< std::string > algsToFetch; // TODO - case insensitive

        auto jwtIssuer( std::make_shared< JwtIssuer >( cacheParams.iss ) );
        if ( !( jwtIssuer ) )
        {
            wsUtilLogFatal( "failed to allocate JwtIssuer for iss=[" << cacheParams.iss << "]" );

            return 1;
        }

        if ( cacheParams.clientAuthzBearerToken.size() )
        {
            wsUtilLogTrace( "using client authz bearer token=[" << cacheParams.clientAuthzBearerToken << "]" );
            jwtIssuer->SetClientAuthzBearerToken( cacheParams.clientAuthzBearerToken );
        }

        for ( auto itAlgToKeyPem = cacheParams.algToKeyPem.cbegin();
            itAlgToKeyPem != cacheParams.algToKeyPem.cend();
            ++itAlgToKeyPem )
        {
            if ( itAlgToKeyPem->second.empty() )
            {
                algsToFetch.emplace( itAlgToKeyPem->first );
            }
            else
            {
                jwtIssuer->SetKeyPemForAlg( itAlgToKeyPem->first, itAlgToKeyPem->second );
            }
        }

        if ( cacheParams.pulldownOpenIdConfiguration )
        {
            int rc = FillJwtIssuerFromEndpoints( algsToFetch, *jwtIssuer );
            if ( rc == 0 )
            {
                (void)issToJwtIssuer.emplace( cacheParams.iss, jwtIssuer );
            }
            else
            {
                wsUtilLogError( "failed to fill JwtIssuer for iss=[" << cacheParams.iss << "], rc=" << rc );

                ret = 2;
            }
        }
        else if ( algsToFetch.size() )
        {
            wsUtilLogError( "load is false but empty keys exist for iss=[" << cacheParams.iss << "]" );

            ret = 3;
        }

        return ret;
    }

    bool JwtIssuerCache::IssuerIsLoaded( const std::string& iss ) const
    {
        const std::lock_guard<std::mutex> lock( cacheMutex );
        auto it = issToJwtIssuer.find( iss );
        if ( it != issToJwtIssuer.cend() )
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    std::shared_ptr< LHWSUtilNS::IJwtIssuer > JwtIssuerCache::GetIssuer( const std::string& iss )
    {
        const std::lock_guard<std::mutex> lock( cacheMutex );
        auto it = issToJwtIssuer.find( iss );
        if ( it != issToJwtIssuer.cend() )
        {
            return it->second;
        }
        else
        {
            bool pending = false;
            auto itPending = pendingIssToCacheParams.find( iss );
            if ( itPending != pendingIssToCacheParams.end() )
            {
                pending = true;

                int rc = reloadIssuer( itPending->second );
                if ( rc == 0 )
                {
                    it = issToJwtIssuer.find( iss );
                    if ( it != issToJwtIssuer.cend() )
                    {
                        pendingIssToCacheParams.erase( itPending );

                        return it->second;
                    }
                }
            }

            std::ostringstream oss;

            oss << "issuer=[" << iss << "] is not loaded";
            if ( pending )
            {
                oss << " but is still pending";
            }

            throw std::runtime_error( oss.str() );
        }
    }

    int FillJwtIssuerFromEndpoints( const std::unordered_set< std::string >& algsToFetch, JwtIssuer& jwtIssuer )
    {
        wsUtilLogSetScope( "FillJwtIssuerFromEndpoints" );

        int rc = 0;
        rapidjson::ParseResult parsedOkay;
        std::unordered_map< std::string, std::string > algToKeyPem;

        std::shared_ptr< LHWSUtilNS::ISimpleHttpClientFactory > simpleHttpClientFactory(
            LHMiscUtilNS::Singleton< LHWSUtilNS::ISimpleHttpClientFactory >::GetInstance() );
        if ( !simpleHttpClientFactory )
        {
            wsUtilLogFatal( "failed to get simpleHttpClientFactory" );

            return 1;
        }

        std::unique_ptr< LHWSUtilNS::ISimpleHttpClient > simpleHttpClient(
            simpleHttpClientFactory->CreateSimpleHttpClient() );
        if ( !simpleHttpClient )
        {
            wsUtilLogFatal( "failed to create simpleHttpClient" );

            return 2;
        }

        std::string issOidConfigUrl = jwtIssuer.GetUrl() + "/.well-known/openid-configuration";
        std::string issOidConfigStr;
        rc = simpleHttpClient->Get( issOidConfigUrl, issOidConfigStr );
        if ( rc != 0 || issOidConfigStr.empty() )
        {
            wsUtilLogError( "failed to get openid-configuration url["
                << issOidConfigUrl << "], body=[" << issOidConfigStr << "], rc=" << rc );

            return 3;
        }

        wsUtilLogInfo( "parsing openid-configuration["
            << issOidConfigStr << "] for issuer=[" << issOidConfigUrl << "]" );

        rapidjson::Document issOidConfigJson;
        parsedOkay = issOidConfigJson.Parse( issOidConfigStr.c_str() );
        if ( !( parsedOkay ) )
        {
            wsUtilLogError( "failed to parse openid-configuration" );

            return 4;
        }

        if ( !( issOidConfigJson.IsObject() &&
            issOidConfigJson.HasMember( "jwks_uri" ) &&
            issOidConfigJson[ "jwks_uri" ].IsString() ) )
        {
            wsUtilLogError( "missing or invalid jwks_uri" );

            return 5;
        }

        std::string issJwksUrl( issOidConfigJson[ "jwks_uri" ].GetString(),
            issOidConfigJson[ "jwks_uri" ].GetStringLength() );
        std::string issJwksStr;
        rc = simpleHttpClient->Get( issJwksUrl, issJwksStr );
        if ( rc != 0 || issJwksStr.empty() )
        {
            wsUtilLogError( "failed to get jwks url["
                << issJwksUrl << "], body=[" << issJwksStr << "], rc=" << rc );

            return 6;
        }

        wsUtilLogInfo( "parsing jwks["
            << issJwksStr << "] for issuer=[" << issOidConfigUrl << "]" );

        rapidjson::Document issJwksJson;
        parsedOkay = issJwksJson.Parse( issJwksStr.c_str() );
        if ( !( parsedOkay ) )
        {
            wsUtilLogError( "failed to parse jwks" );

            return 6;
        }

        if ( !( issJwksJson.IsObject() ) )
        {
            wsUtilLogError( "jwks json is not an object" );

            return 7;
        }

        if ( issJwksJson.HasMember( "keys" ) &&
            issJwksJson[ "keys" ].IsArray() )
        {
            // jwk set
            const rapidjson::Value& keys( issJwksJson[ "keys" ] );
            for ( auto itKey = keys.Begin(); itKey != keys.End(); ++itKey )
            {
                const rapidjson::Value& keyJwkJson( *itKey );
                if ( !( keyJwkJson.IsObject() ) )
                {
                    wsUtilLogError( "keys json item is missing alg member" );

                    return 8;
                }

                if ( !( keyJwkJson.HasMember( "alg" ) ) )
                {
                    wsUtilLogError( "keys json item is missing alg member" );

                    return 80;
                }

                if ( !( keyJwkJson.HasMember( "use" ) ) )
                {
                    wsUtilLogError( "keys json item is missing alg member" );

                    return 81;
                }


                // TODO - case insensitive
                std::string alg( keyJwkJson[ "alg" ].GetString(),
                    keyJwkJson[ "alg" ].GetStringLength() );
                std::string keyUse( keyJwkJson[ "use" ].GetString(),
                    keyJwkJson[ "use" ].GetStringLength() );
                if ( ( keyUse == "sig" ) && algsToFetch.count( alg ) )
                {
                    std::string keyPem;
                    rc = FillKeyStrFromJwkJson( alg, keyJwkJson, keyPem );
                    if ( rc == 0 )
                    {
                        algToKeyPem.emplace( alg, keyPem );
                    }
                }
            }
        }
        else
        {
            if ( !( issJwksJson.HasMember( "alg" ) ) )
            {
                wsUtilLogError( "jwks json is missing alg and keys members" );

                return 90;
            }

            // TODO - case insensitive
            std::string alg( issJwksJson[ "alg" ].GetString(),
                issJwksJson[ "alg" ].GetStringLength() );
            if ( !( algsToFetch.count( alg ) ) )
            {
                wsUtilLogError( "only alg present is not in algsToFetch" );

                return 9;
            }

            std::string keyPem;
            rc = FillKeyStrFromJwkJson( alg, issJwksJson, keyPem );
            if ( rc == 0 )
            {
                algToKeyPem.emplace( alg, keyPem );
            }
        }

        if ( algToKeyPem.empty() && algsToFetch.size() )
        {
            wsUtilLogError( "failed to fetch any alg key pems" );

            return 10;
        }

        jwtIssuer.SetOpenIdConfiguration( issOidConfigStr );

        for ( auto itAlgToKeyPem = algToKeyPem.cbegin(); itAlgToKeyPem != algToKeyPem.cend(); ++itAlgToKeyPem )
        {
            jwtIssuer.SetKeyPemForAlg( itAlgToKeyPem->first, itAlgToKeyPem->second );
        }

        return 0;
    }
}

namespace LHWSUtilNS
{
    std::shared_ptr< IJwtIssuerCache > GetStandardJwtIssuerCache()
    {
        return std::shared_ptr< IJwtIssuerCache >( new LHWSUtilImplNS::JwtIssuerCache() );
    }

    int AuthzBearerTokenForClientIdSecret( const std::string& clientId,
        const std::string& clientSecret,
        std::string& authzBearerTokenOut )
    {
        wsUtilLogSetScope( "AuthzBearerTokenForClientIdSecret" )

            int rc = 0;
        std::ostringstream oss;
        std::string b64EncodedStr;

        auto simpleHttpClientFactory(
            LHMiscUtilNS::Singleton< LHWSUtilNS::ISimpleHttpClientFactory >::GetInstance() );
        if ( !simpleHttpClientFactory )
        {
            wsUtilLogFatal( "failed to get simpleHttpClientFactory" );

            return 1;
        }

        auto simpleHttpClient( simpleHttpClientFactory->CreateSimpleHttpClient() );
        if ( !simpleHttpClient )
        {
            wsUtilLogFatal( "failed to create simpleHttpClient" );

            return 2;
        }

        oss << simpleHttpClient->UrlEscape( clientId ) << ":" << simpleHttpClient->UrlEscape( clientSecret );

        rc = LHSSLUtilNS::EncodeStrToB64( oss.str(), b64EncodedStr );
        if ( rc != 0 )
        {
            return 3;
        }

        authzBearerTokenOut = std::move( b64EncodedStr );

        return 0;
    }
}
