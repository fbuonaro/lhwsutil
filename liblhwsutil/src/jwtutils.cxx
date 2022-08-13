#include <lhwsutil/logging.h>
#include <lhwsutil_impl/jwtutils.h>
#include <lhwsutil_impl/rsa.h>

#include <lhsslutil/base64.h>

namespace LHWSUtilImplNS
{
    int DecomposeJwtStr( const std::string& jwtStr,
        std::string& b64UrlEncodedHeaderOut,
        std::string& b64UrlEncodedPayloadOut,
        std::string& b64UrlEncodedSignatureOut )
    {
        int ret = 0;

        if ( jwtStr.empty() )
        {
            return 1;
        }

        size_t headerDelimiterPos = jwtStr.find( '.' );
        if ( headerDelimiterPos == std::string::npos || headerDelimiterPos <= 0 )
        {
            return 2;
        }


        size_t payloadDelimiterPos = jwtStr.find( '.', headerDelimiterPos + 1 );
        if ( payloadDelimiterPos == std::string::npos )
        {
            return 3;
        }

        if ( payloadDelimiterPos == jwtStr.size() - 1 )
        {
            // empty sign
            return 4;
        }

        b64UrlEncodedHeaderOut = jwtStr.substr( 0, headerDelimiterPos );
        b64UrlEncodedPayloadOut = jwtStr.substr( headerDelimiterPos + 1,
            payloadDelimiterPos - ( headerDelimiterPos + 1 ) );
        b64UrlEncodedSignatureOut = jwtStr.substr( payloadDelimiterPos + 1 );

        return 0;
    }

    int DecodeDecomposedJwtStrs( const std::string& b64UrlEncodedHeader,
        const std::string& b64UrlEncodedPayload,
        std::string& decodedHeaderOut,
        std::string& decodedPayloadOut )
    {
        int rc = 0;
        std::vector< unsigned char > decodedHeaderData;
        std::vector< unsigned char > decodedPayloadData;

        if ( b64UrlEncodedHeader.empty() || b64UrlEncodedPayload.empty() )
        {
            return 1;
        }

        rc = LHSSLUtilNS::DecodeB64UrlStr( b64UrlEncodedHeader, decodedHeaderData );
        if ( rc != 0 )
        {
            return 2;
        }

        rc = LHSSLUtilNS::DecodeB64UrlStr( b64UrlEncodedPayload, decodedPayloadData );
        if ( rc != 0 )
        {
            return 3;
        }

        // TODO - ensure both are valid utf8 [json]

        decodedHeaderOut.assign( reinterpret_cast<char*>( decodedHeaderData.data() ), decodedHeaderData.size() );
        decodedPayloadOut.assign( reinterpret_cast<char*>( decodedPayloadData.data() ), decodedPayloadData.size() );

        return 0;
    }

    int DecomposeAndDecodeJwtStr( const std::string& jwtStr,
        std::string& decodedHeaderOut,
        std::string& decodedPayloadOut,
        std::string& b64UrlEncodedSignatureOut )
    {
        std::string b64UrlEncodedHeader;
        std::string b64UrlEncodedPayload;
        std::string b64UrlEncodedSignature;
        int rc = 0;

        rc = DecomposeJwtStr( jwtStr,
            b64UrlEncodedHeader,
            b64UrlEncodedPayload,
            b64UrlEncodedSignature );
        if ( rc != 0 )
        {
            return 1;
        }

        return DecodeDecomposedJwtStrs( b64UrlEncodedHeader,
            b64UrlEncodedPayload,
            decodedHeaderOut,
            decodedPayloadOut );
    };

    int WriteOutRSAPubKeyComponentsAsPEM( const std::vector< unsigned char >& nBytes,
        const std::vector< unsigned char >& eBytes,
        std::string& pemStrOut )
    {
        wsUtilLogSetScope( "WriteOutRSAPubKeyComponentsAsPEM" );

        RSAPublicKey rsaPublicKey( nBytes, eBytes );
        int ret = rsaPublicKey.GetPEMFormatInto( pemStrOut );
        if ( ret != 0 )
        {
            wsUtilLogError( "failed to write out pubkey as pem" );
            ret = 1;
        }

        return ret;
    }

    int FillRSxKeyFromJwkJson( const rapidjson::Value& key, std::string& keyStrOut )
    {
        wsUtilLogSetScope( "FillRSxKeyFromJwkJson" );

        std::string rsaPublicKeyPEMStr;
        std::vector< unsigned char > nBytes;
        std::vector< unsigned char > eBytes;
        int rc = 0;

        if ( !( key.HasMember( "n" ) && key.HasMember( "e" ) ) )
        {
            wsUtilLogError( "key missing 'n' or 'e'" );
            return 1;
        }

        std::string nStr( key[ "n" ].GetString(), key[ "n" ].GetStringLength() );
        rc = LHSSLUtilNS::DecodeB64UrlStr( nStr, nBytes );
        if ( rc != 0 || nBytes.empty() )
        {
            wsUtilLogError( "failed to decode n=" << nStr );
            return 2;
        }

        std::string eStr( key[ "e" ].GetString(), key[ "e" ].GetStringLength() );
        rc = LHSSLUtilNS::DecodeB64UrlStr( eStr, eBytes );
        if ( rc != 0 || eBytes.empty() )
        {
            wsUtilLogError( "failed to decode e=" << eStr );
            return 3;
        }

        rc = WriteOutRSAPubKeyComponentsAsPEM( nBytes, eBytes, rsaPublicKeyPEMStr );
        if ( rc != 0 || rsaPublicKeyPEMStr.empty() )
        {
            wsUtilLogError( "failed to write out pem, rc=" << rc << ", pem=[" << rsaPublicKeyPEMStr << "]" );
            return 4;
        }

        wsUtilLogTrace( "n=[" << nStr << "], e=[" << eStr << "], pem=[" << rsaPublicKeyPEMStr << "]" );

        keyStrOut = std::move( rsaPublicKeyPEMStr );

        return 0;
    }

    int FillKeyStrFromJwkJson( const std::string& alg, const rapidjson::Value& key, std::string& keyStrOut )
    {
        wsUtilLogSetScope( "FillKeyStrFromJwkJson" );

        if ( alg == "RS256" || alg == "RS384" || alg == "RS512" )
        {
            return FillRSxKeyFromJwkJson( key, keyStrOut );
        }
        else
        {
            wsUtilLogFatal( "unknown alg=[" << alg << "]" );
            return 1;
        }
    }
}
