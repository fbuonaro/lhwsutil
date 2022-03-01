#ifndef __LHWSUTIL_IMPL_JWTVALIDATOR_H__
#define __LHWSUTIL_IMPL_JWTVALIDATOR_H__

#include <jwt.h> // C include, contains extern "C"

#include <memory>
#include <string>
#include <unordered_set>

#include <rapidjson/document.h>

#include <lhwsutil/ijwtvalidator.h>

namespace LHWSUtilImplNS
{
    class ValidJwt : public LHWSUtilNS::IValidJwt
    {
        public:
            ValidJwt( jwt_t** lpJwt );
            ValidJwt( jwt_t* lpJwt, bool copyJwt );
            ~ValidJwt();

            ValidJwt( const ValidJwt& other ) = delete;
            ValidJwt& operator=( const ValidJwt& other ) = delete;
            ValidJwt( ValidJwt&& other ) = delete;
            ValidJwt() = delete;

            int GetGrantBoolValue( const std::string& grant,
                                   bool& valueOut ) const;
            int GetGrantIntValue( const std::string& grant,
                                  long& valueOut ) const;
            int GetGrantJsonValue( const std::string& grant,
                                   std::string& valueOut ) const;
            int GetGrantStrValue( const std::string& grant,
                                  std::string& valueOut ) const;
            void ToString( std::string& out, bool prettyPrint ) const;

        private:
            jwt_t* jwt;
            bool owning;
            int _errno;

            int getJwtErrno() const;
    };

    class ValidJwtJson : public LHWSUtilNS::IValidJwt
    {
        public:
            ValidJwtJson( const rapidjson::Value& _jsonValue );
            ~ValidJwtJson();

            ValidJwtJson( const ValidJwtJson& other ) = delete;
            ValidJwtJson& operator=( const ValidJwtJson& other ) = delete;
            ValidJwtJson( ValidJwtJson&& other ) = delete;
            ValidJwtJson() = delete;

            int GetGrantBoolValue( const std::string& grant,
                                   bool& valueOut ) const;
            int GetGrantIntValue( const std::string& grant,
                                  long& valueOut ) const;
            int GetGrantJsonValue( const std::string& grant,
                                   std::string& valueOut ) const;
            int GetGrantStrValue( const std::string& grant,
                                  std::string& valueOut ) const;
            void ToString( std::string& out, bool prettyPrint ) const;

        private:
            rapidjson::Document jsonValue;
    };


    class JwtValidator : public LHWSUtilNS::IJwtValidator
    {
        public:
            JwtValidator();

            // 1) decode b64EncodedJwt -> jwt
            // 2) getUrl jwt.iss -> issuer
            // 3) getUrl issuer.jwks_uri -> jwk
            // 4) cross reference issuer.id_token_signing_alg_values_supported
            //    and issuer.id_token_encryption_alg_values_supported
            //    with jwt and jwt
            // 5) libjwt -> valid/invalid
            std::unique_ptr< LHWSUtilNS::IValidJwt > ValidateIntoJwt( const std::string& b64UrlEncodedJwt ) const;

            std::unique_ptr< LHWSUtilNS::IValidJwt > IntrospectJwt( const std::string& b64UrlEncodedJwt ) const;
    };

    class JwtValidatorFactory : public LHWSUtilNS::IJwtValidatorFactory
    {
        public:
            JwtValidatorFactory();
            ~JwtValidatorFactory();

            std::unique_ptr< LHWSUtilNS::IJwtValidator > CreateJwtValidator() const;
    };
}

#endif
