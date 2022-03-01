#ifndef __LHWSUTIL_IJWTISSUERCACHE_H__
#define __LHWSUTIL_IJWTISSUERCACHE_H__

#include <memory>
#include <string>
#include <unordered_map>

namespace LHWSUtilNS
{
    class IJwtIssuer
    {
        public:
            IJwtIssuer();
            virtual ~IJwtIssuer();

            virtual const std::string& GetUrl() const = 0;
            virtual bool AlgIsSupported( const std::string& alg ) const = 0;
            virtual const std::string& GetKeyPemForAlg( const std::string& alg ) const = 0;
            virtual const std::string& GetClientAuthzBearerToken() const = 0;
            virtual const std::string& GetOpenIdConfiguration() const = 0;
    };

    struct JwtIssuerCacheParams
    {
        JwtIssuerCacheParams();

        std::string iss;
        std::string clientAuthzBearerToken;
        std::unordered_map< std::string, std::string > algToKeyPem;
        bool pulldownOpenIdConfiguration;
        // pulldownOpenIdConfiguration && algToKeyPem[ alg ].empty => fetch jwk url and generate pem dynamically at load
    };

    class IJwtIssuerCache
    {
        public:
            IJwtIssuerCache();
            virtual ~IJwtIssuerCache();

            virtual void LoadIssuer( const JwtIssuerCacheParams& cacheParams ) = 0;
            virtual bool IssuerIsLoaded( const std::string& iss ) const = 0;
            virtual std::shared_ptr< IJwtIssuer > GetIssuer( const std::string& iss ) = 0;
    };

    std::shared_ptr< IJwtIssuerCache > GetStandardJwtIssuerCache();


    int AuthzBearerTokenForClientIdSecret( const std::string& clientId,
                                           const std::string& clientSecret,
                                           std::string& authzBearerTokenOut );
}

#include <lhmiscutil/singleton.h>

namespace LHMiscUtilNS
{
    EnableClassAsSingleton( LHWSUtilNS::IJwtIssuerCache, SingletonCanBeSet::WhenEmpty );
}

#endif
