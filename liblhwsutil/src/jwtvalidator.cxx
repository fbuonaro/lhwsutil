#include <errno.h>

#include <jwt.h> // C

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <memory>
#include <stdexcept>
#include <unordered_map>

#include <lhmiscutil/singleton.h>

#include <lhwsutil/ijwtvalidator.h>
#include <lhwsutil/ijwtissuercache.h>
#include <lhwsutil/isimplehttpclient.h>
#include <lhwsutil/logging.h>

#include <lhwsutil_impl/jwtvalidator.h>
#include <lhwsutil_impl/jwtutils.h>

namespace LHWSUtilNS
{
    std::shared_ptr< IJwtValidatorFactory > GetStandardJwtValidatorFactory()
    {
        return std::make_shared< LHWSUtilImplNS::JwtValidatorFactory >();
    }
}

namespace LHWSUtilImplNS
{
    namespace
    {
        int getKeyForJwt( const jwt_t* jwtIn, jwt_key_t* keyOut )
        {
            wsUtilLogSetScope( "getKeyForJwt" );

            wsUtilLogTrace( "getting key" );

            try
            {

                // idk why this accepts const when all of the jwt_get_grant* functions
                // accept non-const wtf
                ValidJwt jwt( const_cast<jwt_t*>( jwtIn ), false );
                int rc = 0;
                std::string iss;

                rc = jwt.GetGrantStrValue( "iss", iss );
                if ( rc || iss.empty() )
                {
                    wsUtilLogError( "missing 'iss'" );
                    return 1;
                }

                jwt_alg_t jwtAlgType = jwt_get_alg( jwtIn );
                const char* jwtAlg( jwt_alg_str( jwtAlgType ) );
                if ( !( jwtAlg ) )
                {
                    wsUtilLogError( "invalid 'alg'" );
                    return 2;
                }

                std::string alg( jwtAlg );

                auto jwtIssuerCache(
                    LHMiscUtilNS::Singleton< LHWSUtilNS::IJwtIssuerCache >::GetInstance() );
                if ( !jwtIssuerCache )
                {
                    wsUtilLogError( "failed to fetch issuer cache" );
                    return 3;
                }

                auto jwtIssuer = jwtIssuerCache->GetIssuer( iss );
                if ( !( jwtIssuer->AlgIsSupported( alg ) ) )
                {
                    wsUtilLogError( "unsupported alg=[" << alg << "]" );
                    return 4;
                }

                const std::string& keyPem( jwtIssuer->GetKeyPemForAlg( alg ) );

                wsUtilLogTrace( "using key=[" << keyPem << "]" );

                keyOut->jwt_key = reinterpret_cast<const unsigned char*>( keyPem.c_str() );
                keyOut->jwt_key_len = keyPem.size();

                return 0;
            }
            catch ( const std::exception& e )
            {
                wsUtilLogError( "exception e=[" << e.what() << "]" );
                return -1;
            }
            catch ( ... )
            {
                wsUtilLogError( "unknown exception]" );
                return -2;
            }
        }
    }

    ValidJwt::ValidJwt( jwt_t** lpJwt )
        : LHWSUtilNS::IValidJwt()
        , jwt( nullptr )
        , owning( false )
    {
        if ( lpJwt && *lpJwt )
        {
            jwt = *lpJwt;
            *lpJwt = nullptr;
            owning = true;
        }
        else
        {
            throw std::runtime_error( "jwt is null" );
        }
    }

    ValidJwt::ValidJwt( jwt_t* _jwt, bool copyJwt )
        : LHWSUtilNS::IValidJwt()
        , jwt( nullptr )
        , owning( false )
    {
        if ( _jwt )
        {
            if ( copyJwt )
            {
                jwt = jwt_dup( _jwt );
                owning = true;
            }
            else
            {
                jwt = _jwt;
            }
        }
        else
        {
            throw std::runtime_error( "jwt is null" );
        }
    }

    ValidJwt::~ValidJwt()
    {
        if ( jwt && owning )
        {
            jwt_free( jwt );
            jwt = nullptr;
        }
    }

    int ValidJwt::GetGrantBoolValue( const std::string& grant,
        bool& valueOut ) const
    {
        if ( grant.empty() )
        {
            return -1;
        }

        int jwtBool = jwt_get_grant_bool( jwt, grant.c_str() );
        int ret = getJwtErrno();
        if ( ret == 0 )
        {
            valueOut = jwtBool;
        }

        return ret;
    }

    int ValidJwt::GetGrantIntValue( const std::string& grant,
        long& valueOut ) const
    {
        if ( grant.empty() )
        {
            return -1;
        }

        long jwtInt = jwt_get_grant_int( jwt, grant.c_str() );
        int ret = getJwtErrno();
        if ( ret == 0 )
        {
            valueOut = jwtInt;
        }

        return ret;
    }

    int ValidJwt::GetGrantStrValue( const std::string& grant,
        std::string& valueOut ) const
    {
        if ( grant.empty() )
        {
            return -1;
        }

        const char* jwtStr = jwt_get_grant( jwt, grant.c_str() );
        int ret = 0;
        if ( jwtStr )
        {
            valueOut = jwtStr;
        }
        else
        {
            ret = 1;
        }

        return ret;
    }

    int ValidJwt::GetGrantJsonValue( const std::string& grant,
        std::string& valueOut ) const
    {
        if ( grant.empty() )
        {
            return -1;
        }

        char* jwtJson = jwt_get_grants_json( jwt, grant.c_str() );
        int ret = 0;
        if ( jwtJson )
        {
            valueOut = jwtJson;
            jwt_free_str( jwtJson );
        }
        else
        {
            ret = 1;
        }

        return ret;
    }

    void ValidJwt::ToString( std::string& out, bool prettyPrint ) const
    {
        char* jwtStr = jwt_dump_str( jwt, prettyPrint );
        if ( jwtStr )
        {
            out = jwtStr;
            jwt_free_str( jwtStr );
        }
    }


    int ValidJwt::getJwtErrno() const
    {
        return errno;
    }

    ValidJwtJson::ValidJwtJson( const rapidjson::Value& _jsonValue )
    {
        jsonValue.CopyFrom( _jsonValue, jsonValue.GetAllocator() );
        if ( !( jsonValue.IsObject() ) )
        {
            throw std::runtime_error( "json value is not a valid object" );
        }
    }

    ValidJwtJson::~ValidJwtJson()
    {
    }

    int ValidJwtJson::GetGrantBoolValue( const std::string& grant,
        bool& valueOut ) const
    {
        if ( grant.empty() )
        {
            return -1;
        }

        int ret = 0;

        if ( jsonValue.HasMember( grant.c_str() ) && jsonValue[ grant.c_str() ].IsBool() )
        {
            valueOut = jsonValue[ grant.c_str() ].GetBool();
        }
        else
        {
            ret = 1;
        }

        return ret;
    }

    int ValidJwtJson::GetGrantIntValue( const std::string& grant,
        long& valueOut ) const
    {
        if ( grant.empty() )
        {
            return -1;
        }

        int ret = 0;

        if ( jsonValue.HasMember( grant.c_str() ) && jsonValue[ grant.c_str() ].IsInt() )
        {
            valueOut = jsonValue[ grant.c_str() ].GetInt();
        }
        else
        {
            ret = 1;
        }

        return ret;
    }

    int ValidJwtJson::GetGrantJsonValue( const std::string& grant,
        std::string& valueOut ) const
    {
        if ( grant.empty() )
        {
            return -1;
        }

        int ret = 0;

        if ( jsonValue.HasMember( grant.c_str() ) )
        {
            rapidjson::StringBuffer buffer;
            rapidjson::Writer< rapidjson::StringBuffer > writer( buffer );
            jsonValue[ grant.c_str() ].Accept( writer );

            valueOut.assign( buffer.GetString(), buffer.GetSize() );
        }
        else
        {
            ret = 1;
        }

        return 0;
    }

    int ValidJwtJson::GetGrantStrValue( const std::string& grant,
        std::string& valueOut ) const
    {
        if ( grant.empty() )
        {
            return -1;
        }

        int ret = 0;

        if ( jsonValue.HasMember( grant.c_str() ) && jsonValue[ grant.c_str() ].IsString() )
        {
            valueOut.assign( jsonValue[ grant.c_str() ].GetString(),
                jsonValue[ grant.c_str() ].GetStringLength() );
        }
        else
        {
            ret = 1;
        }

        return ret;
    }

    void ValidJwtJson::ToString( std::string& out, bool prettyPrint ) const
    {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer< rapidjson::StringBuffer > writer( buffer );
        jsonValue.Accept( writer );

        out.assign( buffer.GetString(), buffer.GetSize() );
    }


    JwtValidator::JwtValidator()
        : LHWSUtilNS::IJwtValidator()
    {
    }

    std::unique_ptr< LHWSUtilNS::IValidJwt > JwtValidator::ValidateIntoJwt( const std::string& b64UrlEncodedJwt ) const
    {
        wsUtilLogSetScope( "JwtValidator.ValidateIntoJwt" );

        int rc = 0;
        jwt_t* jwt = nullptr;

        if ( b64UrlEncodedJwt.empty() )
        {
            wsUtilLogFatal( "jwt is empty" );
            return nullptr;
        }

        rc = jwt_decode_2( &jwt, b64UrlEncodedJwt.c_str(), &getKeyForJwt );
        if ( rc != 0 )
        {
            wsUtilLogInfo( "failed to decode, rc=" << rc );

            return nullptr;
        }

        return std::unique_ptr< LHWSUtilNS::IValidJwt >( new ValidJwt( &jwt ) );
    }

    std::unique_ptr< LHWSUtilNS::IValidJwt > JwtValidator::IntrospectJwt( const std::string& b64UrlEncodedJwt ) const
    {
        wsUtilLogSetScope( "JwtValidator.IntrospectJwt" );

        int rc = 0;
        std::string clientAuthzBearerToken;
        std::unordered_map< std::string, std::string > headers;
        std::string postData = "token_type_hint=requesting_party_token&token=" + b64UrlEncodedJwt;
        std::string decodedHeaderJsonStr;
        std::string decodedPayloadJsonStr;
        std::string b64UrlEncodedSignature;
        std::string iss;
        std::string responseBody;
        std::string introspectionEndpoint;


        auto simpleHttpClientFactory(
            LHMiscUtilNS::Singleton< LHWSUtilNS::ISimpleHttpClientFactory >::GetInstance() );
        if ( !simpleHttpClientFactory )
        {
            wsUtilLogFatal( "failed to get simpleHttpClientFactory" );

            return nullptr;
        }

        auto simpleHttpClient( simpleHttpClientFactory->CreateSimpleHttpClient() );
        if ( !simpleHttpClient )
        {
            wsUtilLogFatal( "failed to create simpleHttpClient" );

            return nullptr;
        }

        rc = DecomposeAndDecodeJwtStr( b64UrlEncodedJwt,
            decodedHeaderJsonStr,
            decodedPayloadJsonStr,
            b64UrlEncodedSignature );
        if ( rc != 0 )
        {
            return nullptr;
        }

        rapidjson::Document headerJson;
        rapidjson::ParseResult parsedOkay = headerJson.Parse( decodedHeaderJsonStr.c_str() );
        if ( !( parsedOkay ) )
        {
            wsUtilLogError( "failed to parse header json[" << decodedHeaderJsonStr << "]" );

            return nullptr;
        }

        rapidjson::Document payloadJson;
        parsedOkay = payloadJson.Parse( decodedPayloadJsonStr.c_str() );
        if ( !( parsedOkay ) )
        {
            wsUtilLogError( "failed to parse payload json[" << decodedPayloadJsonStr << "]" );

            return nullptr;
        }

        if ( !( payloadJson.IsObject() &&
            payloadJson.HasMember( "iss" ) &&
            payloadJson[ "iss" ].IsString() ) )
        {
            wsUtilLogError( "iss missing or invalid in payload json[" << decodedPayloadJsonStr << "]" );

            return nullptr;
        }

        iss.assign( payloadJson[ "iss" ].GetString(), payloadJson[ "iss" ].GetStringLength() );

        auto jwtIssuerCache(
            LHMiscUtilNS::Singleton< LHWSUtilNS::IJwtIssuerCache >::GetInstance() );
        if ( !jwtIssuerCache )
        {
            wsUtilLogError( "failed to get jwtIssuerCache" );

            return nullptr;
        }

        auto jwtIssuer = jwtIssuerCache->GetIssuer( iss );
        if ( !( jwtIssuer ) )
        {
            wsUtilLogError( "failed to get issuer[" << iss << "]" );

            return nullptr;
        }

        if ( jwtIssuer->GetOpenIdConfiguration().empty() )
        {
            wsUtilLogError( "missing openid config for issuer[" << iss << "]" );

            return nullptr;
        }

        if ( jwtIssuer->GetClientAuthzBearerToken().empty() )
        {
            wsUtilLogError( "missing bearer token for issuer[" << iss << "]" );

            return nullptr;
        }

        rapidjson::Document openIdConfigurationJson;
        parsedOkay = openIdConfigurationJson.Parse( jwtIssuer->GetOpenIdConfiguration().c_str() );
        if ( !( parsedOkay ) )
        {
            wsUtilLogError( "failed to parse openid config json["
                << jwtIssuer->GetOpenIdConfiguration() << "]" );

            return nullptr;
        }

        if ( !( openIdConfigurationJson.IsObject() &&
            openIdConfigurationJson.HasMember( "introspection_endpoint" ) &&
            openIdConfigurationJson[ "introspection_endpoint" ].IsString() ) )
        {
            wsUtilLogError( "introspection_endpoint missing or invalid in openid config json["
                << jwtIssuer->GetOpenIdConfiguration() << "]" );

            return nullptr;
        }

        introspectionEndpoint.assign( openIdConfigurationJson[ "introspection_endpoint" ].GetString(),
            openIdConfigurationJson[ "introspection_endpoint" ].GetStringLength() );

        wsUtilLogDebug( "using Bearer token[" << jwtIssuer->GetClientAuthzBearerToken() << "]" );
        headers.emplace( "Authorization", "Basic " + jwtIssuer->GetClientAuthzBearerToken() );
        headers.emplace( "Content-Type", "application/x-www-form-urlencoded" );
        headers.emplace( "Accept", "application/json" );

        rc = simpleHttpClient->Post( introspectionEndpoint, postData, headers, responseBody );
        if ( rc != 0 )
        {
            wsUtilLogError( "failed to post to introspection_endpoint["
                << introspectionEndpoint << "], rc=" << rc );

            return nullptr;
        }

        rapidjson::Document responseJson;
        parsedOkay = responseJson.Parse( responseBody.c_str() );
        if ( !( parsedOkay ) )
        {
            wsUtilLogError( "failed to parse introspection_endpoint response[" << responseBody << "]" );

            return nullptr;
        }

        if ( !( responseJson.HasMember( "active" ) && responseJson[ "active" ].IsBool() ) )
        {
            wsUtilLogError( "missing 'active' in introspection_endpoint response[" << responseBody << "]" );

            return nullptr;
        }

        if ( !( responseJson[ "active" ].GetBool() ) )
        {
            wsUtilLogDebug( "token no longer active[" << b64UrlEncodedJwt << "]" );

            return nullptr;
        }


        return std::unique_ptr< LHWSUtilNS::IValidJwt >( new ValidJwtJson( payloadJson ) );
    }

    JwtValidatorFactory::JwtValidatorFactory()
        : LHWSUtilNS::IJwtValidatorFactory()
    {
    }

    JwtValidatorFactory::~JwtValidatorFactory()
    {
    }

    std::unique_ptr< LHWSUtilNS::IJwtValidator > JwtValidatorFactory::CreateJwtValidator() const
    {
        return std::unique_ptr< LHWSUtilNS::IJwtValidator >( new JwtValidator() );
    }
}
