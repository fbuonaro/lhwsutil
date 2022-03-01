#include <openssl/evp.h> // TODO - upgrade to 1.1, support multiple versions

#include <lhwsutil/logging.h>
#include <lhwsutil_impl/jwtutils.h>
#include <lhwsutil_impl/rsa.h>

namespace LHWSUtilImplNS
{
    int ConvertB64UrlStrToB64( const std::string& b64UrlStr, std::string& b64StrOut, int& paddingLengthOut )
    {
        std::string b64Str;
        int paddingNeeded = b64UrlStr.size() % 4;

        switch( paddingNeeded )
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

        // switch( paddingNeeded )
        // {
        //     case( 0 ):
        //     {
        //         b64Str = b64UrlStr;
        //         break;
        //     }
        //     case( 1 ):
        //     {
        //         b64Str = b64UrlStr + "=";
        //         break;
        //     }
        //     case( 2 ):
        //     {
        //         b64Str = b64UrlStr + "==";
        //         break;
        //     }
        //     default:
        //     {
        //         return 1;
        //     }
        // }

        for( auto it = b64Str.begin(); it != b64Str.end(); ++it )
        {
            switch( *it )
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

        int numBytesDecoded = 0;
        int paddingLength = 0;
        int ret = 0;
        size_t decodedDataLen = 0;
        std::string b64EncodedStr;
        std::vector< unsigned char > decodedData;

       
        ret = ConvertB64UrlStrToB64( b64UrlEncodedStr, b64EncodedStr, paddingLength );
        if( ret != 0 )
        {
            wsUtilLogError( "failed to convert from B64Url to B64[" << b64UrlEncodedStr << "]" );
            return 1;
        }

        decodedDataLen = 3 * ( b64EncodedStr.size() / 4 ) - paddingLength;
        decodedData.resize( decodedDataLen );

        EVP_ENCODE_CTX decodeContext{ 0 };
        EVP_DecodeInit( &decodeContext );
        ret = EVP_DecodeUpdate( &decodeContext,
                                decodedData.data(),
                                &numBytesDecoded, 
                                reinterpret_cast< const unsigned char* >( b64EncodedStr.c_str() ),
                                b64EncodedStr.size() );
        if( ret != 0 && ret != 1 )
        {
            wsUtilLogError( "failed to decode update B64[" << b64EncodedStr << "], rc=" << ret );
            return 2;
        }

        if( numBytesDecoded != decodedDataLen )
        {
            wsUtilLogError( "unexpected number of bytes decoded for B64["
                       << b64EncodedStr << "], expected="
                       << decodedDataLen << " decoded="
                       << numBytesDecoded );
            return 3;
        }

        ret = EVP_DecodeFinal( &decodeContext, decodedData.data(), &numBytesDecoded );
        if( ret != 1 )
        {
            wsUtilLogError( "failed to decode finalize, rc=" << ret );
            return 4;
        }

        decodedStrOut = std::move( decodedData );
        return 0;
    }

    int EncodeStrToB64( const std::string& str, std::string& b64EncodedStr )
    {
        wsUtilLogSetScope( "EncodeStrToB64" );

        if( str.empty() )
        {
            return 0;
        }

        int numBytesEncoded = 0;
        size_t expectedEncodedDataLen = 4 * ( 1 + ( ( str.size() - 1 ) / 3 ) );
        size_t encodedDataLen = 0;
        std::vector< unsigned char > encodedData;

        encodedData.resize( expectedEncodedDataLen + 1 );

        EVP_ENCODE_CTX encodeContext{ 0 };
        EVP_EncodeInit( &encodeContext );
        EVP_EncodeUpdate( &encodeContext,
                          encodedData.data(),
                          &numBytesEncoded, 
                          reinterpret_cast< const unsigned char* >( str.c_str() ),
                          str.size() );
        encodedDataLen += numBytesEncoded - 1;

        EVP_EncodeFinal( &encodeContext, encodedData.data() + encodedDataLen, &numBytesEncoded );
        encodedDataLen += numBytesEncoded - 1;

        if( expectedEncodedDataLen != encodedDataLen )
        {
            wsUtilLogError( "unexpected number of bytes encoded for B64["
                            << str << "], expected="
                            << expectedEncodedDataLen << " encoded="
                            << encodedDataLen );
            return 3;
        }

        b64EncodedStr.assign( reinterpret_cast< char* >( encodedData.data() ), expectedEncodedDataLen );

        wsUtilLogDebug( "encoded str=[" << str << "] to b64=[" << b64EncodedStr << "]" );

        return 0;

    }

    int DecomposeJwtStr( const std::string& jwtStr,
                         std::string& b64UrlEncodedHeaderOut,
                         std::string& b64UrlEncodedPayloadOut,
                         std::string& b64UrlEncodedSignatureOut )
    {
        int ret = 0;

        if( jwtStr.empty() )
        {
            return 1;
        }
        
        size_t headerDelimiterPos = jwtStr.find( '.' );
        if( headerDelimiterPos == std::string::npos || headerDelimiterPos <= 0 )
        {
            return 2;
        }
        

        size_t payloadDelimiterPos = jwtStr.find( '.', headerDelimiterPos + 1 );
        if( payloadDelimiterPos == std::string::npos )
        {
            return 3;
        }

        if( payloadDelimiterPos == jwtStr.size() - 1 )
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

        if( b64UrlEncodedHeader.empty() || b64UrlEncodedPayload.empty() )
        {
            return 1;
        }

        rc = DecodeB64UrlStr( b64UrlEncodedHeader, decodedHeaderData );
        if( rc != 0 )
        {
            return 2;
        }

        rc = DecodeB64UrlStr( b64UrlEncodedPayload, decodedPayloadData );
        if( rc != 0 )
        {
            return 3;
        }

        // TODO - ensure both are valid utf8 [json]

        decodedHeaderOut.assign( reinterpret_cast< char* >( decodedHeaderData.data() ), decodedHeaderData.size() );
        decodedPayloadOut.assign( reinterpret_cast< char* >( decodedPayloadData.data() ), decodedPayloadData.size() );

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
        if( rc != 0 )
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
        int ret =  rsaPublicKey.GetPEMFormatInto( pemStrOut );
        if( ret != 0 )
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
        
        if( !( key.HasMember( "n" ) && key.HasMember( "e" ) ) )
        {
            wsUtilLogError( "key missing 'n' or 'e'" );
            return 1;
        }

        std::string nStr( key[ "n" ].GetString(), key[ "n" ].GetStringLength() );
        rc = DecodeB64UrlStr( nStr, nBytes );
        if( rc != 0 || nBytes.empty() )
        {
            return 2;
        }

        std::string eStr( key[ "e" ].GetString(), key[ "e" ].GetStringLength() );
        rc = DecodeB64UrlStr( eStr, eBytes );
        if( rc != 0 || eBytes.empty() )
        {
            return 3;
        }

        rc = WriteOutRSAPubKeyComponentsAsPEM( nBytes, eBytes, rsaPublicKeyPEMStr );
        if( rc != 0 || rsaPublicKeyPEMStr.empty() )
        {
            wsUtilLogError( "failed to write out pem, rc=" << rc << ", pem=[" << rsaPublicKeyPEMStr << "]" );
            return 4;
        }

        keyStrOut = std::move( rsaPublicKeyPEMStr );

        return 0;
    }

    int FillKeyStrFromJwkJson( const std::string& alg, const rapidjson::Value& key, std::string& keyStrOut )
    {
        wsUtilLogSetScope( "FillKeyStrFromJwkJson" );

        if( alg == "RS256" || alg == "RS384" || alg == "RS512" )
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
