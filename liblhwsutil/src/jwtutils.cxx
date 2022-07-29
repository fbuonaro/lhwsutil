#include <openssl/bio.h>
#include <openssl/evp.h> // exports BIO_f_base64 for reasons unknown

#include <lhwsutil/logging.h>
#include <lhwsutil_impl/jwtutils.h>
#include <lhwsutil_impl/rsa.h>

namespace LHWSUtilImplNS
{
    namespace
    {
        // I just cannot anymore with b64
        // back to the bio method because it is supported I guess idk

        int opensslB64Encode(
            const void* msg,
            size_t msgLen,
            std::string& b64EncodedStr,
            bool includeNewLines )
        {
            BIO* bio = nullptr;
            BIO* bioB64 = nullptr;
            BIO* bioMem = nullptr;
            char* lpEncoded = nullptr;
            int ret = 0;
            long encodedLength = 0;

            if ( !( msg ) )
            {
                return 11;
            }

            if ( msgLen == 0 )
            {
                b64EncodedStr = "";
                return 0;
            }

            bioB64 = BIO_new( BIO_f_base64() );
            if ( !( bioB64 ) )
            {
                return 1;
            }

            if ( !( includeNewLines ) )
            {
                BIO_set_flags( bioB64, BIO_FLAGS_BASE64_NO_NL );
            }

            bioMem = BIO_new( BIO_s_mem() );
            if ( !( bioMem ) )
            {
                return 2;
            }

            ret = BIO_set_close( bioMem, BIO_CLOSE );
            if ( ret != 1 )
            {
                return 3;
            }

            // data is first encoded, then written to mem, or some nonsense
            bio = BIO_push( bioB64, bioMem );
            if ( !( bio ) )
            {
                return 4;
            }

            ret = BIO_write( bioB64, msg, msgLen );
            if ( ret != msgLen )
            {
                return 5;
            }

            ret = BIO_flush( bioB64 );
            if ( ret <= 0 )
            {
                return 6;
            }

            encodedLength = BIO_get_mem_data( bioMem, &lpEncoded );
            if ( encodedLength <= 0 )
            {
                return 7;
            }

            b64EncodedStr.assign( lpEncoded, encodedLength );

            BIO_free_all( bio );

            return 0;
        }

        int opensslB64Decode(
            const std::string& b64EncodedStr,
            std::vector< unsigned char >& decodedMsgOut,
            bool includeNewLines )
        {
            wsUtilLogSetScope( "opensslB64Decode" );

            BIO* bio = NULL;
            BIO* bioB64 = NULL;
            BIO* bioMem = NULL;
            int ret = 0;
            std::vector< unsigned char > decodedMsg;

            if ( b64EncodedStr.empty() )
            {
                decodedMsgOut.resize( 0 );
                return 0;
            }

            // might exceed final size, don't care anymore
            decodedMsg.resize( 3 * ( b64EncodedStr.size() / 4 ) );

            bioB64 = BIO_new( BIO_f_base64() );
            if ( !( bioB64 ) )
            {
                return 1;
            }

            //TODO - I might be able to link decoded buffer directly to the bioMem so that I do not need to copy anything
            bioMem = BIO_new_mem_buf( const_cast<char*>( b64EncodedStr.data() ), b64EncodedStr.size() );
            if ( !( bioMem ) )
            {
                return 2;
            }

            bio = BIO_push( bioB64, bioMem );
            if ( !( bio ) )
            {
                return 3;
            }

            //not sure if I should call these functions on the bio variable or the individual modules
            //Here, I am calling it on the individual modules
            if ( !( includeNewLines ) )
            {
                BIO_set_flags( bioB64, BIO_FLAGS_BASE64_NO_NL );
            }

            ret = BIO_set_close( bioMem, BIO_NOCLOSE );
            if ( ret != 1 )
            {
                return 4;
            }

            ret = BIO_read( bio, decodedMsg.data(), decodedMsg.size() );
            if ( ret < 0 )
            {
                return 5;
            }

            decodedMsg.resize( ret );

            decodedMsgOut = std::move( decodedMsg );

            BIO_free_all( bio );

            return 0;
        }

    }

    int ConvertB64UrlStrToB64( const std::string& b64UrlStr, std::string& b64StrOut, int& paddingLengthOut )
    {
        std::string b64Str;
        int paddingNeeded = b64UrlStr.size() % 4;

        switch ( paddingNeeded )
        {
        case( 0 ):
        {
            b64Str = b64UrlStr;
            break;
        }
        case( 3 ):
        {
            b64Str = b64UrlStr + "=";
            paddingNeeded = 1;
            break;
        }
        case( 2 ):
        {
            b64Str = b64UrlStr + "==";
            break;
        }
        default:
        {
            return 1;
        }
        }

        for ( auto it = b64Str.begin(); it != b64Str.end(); ++it )
        {
            switch ( *it )
            {
            case( '_' ):
            {
                *it = '/';
                break;
            }
            case( '-' ):
            {
                *it = '+';
                break;
            }
            case( '\n' ):
            case( '/' ):
            case( '+' ):
            {
                // string might not be b64 url encoded
                return 3;
            }
            default:
            {
                break;
            }
            }
        }

        paddingLengthOut = paddingNeeded;
        b64StrOut = std::move( b64Str );

        return 0;
    }

    int DecodeB64UrlStr( const std::string& b64UrlEncodedStr,
        std::vector< unsigned char >& decodedStrOut )
    {
        wsUtilLogSetScope( "DecodeB64UrlStr" );

        int paddingLength = 0;
        int ret = 0;
        std::string b64EncodedStr;

        ret = ConvertB64UrlStrToB64( b64UrlEncodedStr, b64EncodedStr, paddingLength );
        if ( ret != 0 )
        {
            wsUtilLogError( "failed to convert from B64Url to B64[" << b64UrlEncodedStr << "]" );
            return 1;
        }

        ret = opensslB64Decode( b64EncodedStr, decodedStrOut, false );
        if ( ret != 0 )
        {
            wsUtilLogError( "failed to decode B64[" << b64EncodedStr << "], ret=" << ret );
            return 2;
        }

        return 0;
    }

    int DecodeB64Str(
        const std::string& b64EncodedStr,
        std::vector< unsigned char >& decodedStrOut,
        bool includeNewLines )
    {
        wsUtilLogSetScope( "DecodeB64Str" );

        int ret = opensslB64Decode( b64EncodedStr, decodedStrOut, includeNewLines );
        if ( ret != 0 )
        {
            wsUtilLogError( "failed to decode B64[" << b64EncodedStr << "], ret=" << ret );
            return 2;
        }

        return 0;
    }

    int EncodeStrToB64( const std::string& str, std::string& b64EncodedStr, bool includeNewLines )
    {
        wsUtilLogSetScope( "EncodeStrToB64" );

        int ret = opensslB64Encode( str.data(), str.size(), b64EncodedStr, includeNewLines );
        if ( ret != 0 )
        {
            wsUtilLogError( "failed to B64 encode str[" << str << "], ret=" << ret );
            return 1;
        }

        return 0;
    }

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

        rc = DecodeB64UrlStr( b64UrlEncodedHeader, decodedHeaderData );
        if ( rc != 0 )
        {
            return 2;
        }

        rc = DecodeB64UrlStr( b64UrlEncodedPayload, decodedPayloadData );
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
        rc = DecodeB64UrlStr( nStr, nBytes );
        if ( rc != 0 || nBytes.empty() )
        {
            wsUtilLogError( "failed to decode n=" << nStr );
            return 2;
        }

        std::string eStr( key[ "e" ].GetString(), key[ "e" ].GetStringLength() );
        rc = DecodeB64UrlStr( eStr, eBytes );
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
