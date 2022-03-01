#ifndef __LHWSUTIL_IMPL_JWTISSUERCACHE_H__
#define __LHWSUTIL_IMPL_JWTISSUERCACHE_H__

#include <lhwsutil/ijwtissuercache.h>

#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace LHWSUtilImplNS
{
    class JwtIssuer : public LHWSUtilNS::IJwtIssuer
    {
        public:
            JwtIssuer( const std::string& url );
            ~JwtIssuer();

            const std::string& GetUrl() const;
            bool AlgIsSupported( const std::string& alg ) const;
            const std::string& GetKeyPemForAlg( const std::string& alg ) const;
            const std::string& GetClientAuthzBearerToken() const;
            const std::string& GetOpenIdConfiguration() const;

            void SetKeyPemForAlg( const std::string& alg, const std::string& keyPem );
            void SetClientAuthzBearerToken( const std::string& _clientAuthzBearerToken );
            void SetOpenIdConfiguration( const std::string _openIdConfiguration );

        private:
            std::string url;
            std::unordered_map< std::string, std::string > algToKeyPem;
            std::string clientAuthzBearerToken;
            std::string openIdConfiguration;
    };

    class JwtIssuerCache : public LHWSUtilNS::IJwtIssuerCache
    {
        public:
            JwtIssuerCache();
            ~JwtIssuerCache();

            void LoadIssuer( const LHWSUtilNS::JwtIssuerCacheParams& cacheParams );
            bool IssuerIsLoaded( const std::string& iss ) const;
            std::shared_ptr< LHWSUtilNS::IJwtIssuer > GetIssuer( const std::string& iss );

        private:
            mutable std::mutex cacheMutex;
            std::unordered_map< std::string, std::shared_ptr< JwtIssuer > > issToJwtIssuer;
            std::unordered_map< std::string, LHWSUtilNS::JwtIssuerCacheParams > pendingIssToCacheParams;

            int reloadIssuer( const LHWSUtilNS::JwtIssuerCacheParams& cacheParams );
    };

    int FillJwtIssuerFromEndpoints( const std::unordered_set< std::string >& algsToFetch,
                                     JwtIssuer& jwtIssuer );
}

#endif
