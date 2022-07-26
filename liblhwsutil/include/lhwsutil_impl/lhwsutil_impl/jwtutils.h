#ifndef __LHWSUTIL_IMPL_JWTUTILS_H__
#define __LHWSUTIL_IMPL_JWTUTILS_H__

#include <rapidjson/document.h>

#include <string>
#include <vector>

namespace LHWSUtilImplNS
{
    // does not add padding
    int ConvertB64UrlStrToB64( const std::string& b64UrlStr, std::string& b64StrOut, int& paddingLengthOut );

    int DecodeB64UrlStr(
        const std::string& b64UrlEncodedStr,
        std::vector< unsigned char >& decodedStrOut );

    int DecodeB64Str(
        const std::string& b64EncodedStr,
        std::vector< unsigned char >& decodedStrOut,
        bool includeNewLines );

    int DecomposeJwtStr( const std::string& jwtStr,
        std::string& b64UrlEncodedHeaderOut,
        std::string& b64UrlEncodedPayloadOut,
        std::string& b64UrlEncodedSignatureOut );

    int DecodeDecomposedJwtStrs( const std::string& b64UrlEncodedHeader,
        const std::string& b64UrlEncodedPayload,
        std::string& decodedHeaderOut,
        std::string& decodedPayloadOut );

    int DecomposeAndDecodeJwtStr( const std::string& jwtStr,
        std::string& decodedHeaderOut,
        std::string& decodedPayloadOut,
        std::string& b64UrlEncodedSignatureOut );

    // nBytes and eBytes are big endian encoded integers
    int WriteOutRSAPubKeyComponentsAsPEM( const std::vector< unsigned char >& nBytes,
        const std::vector< unsigned char >& eBytes,
        std::string& pemStrOut );

    int FillRSxKeyFromJwkJson( const rapidjson::Value& key, std::string& keyStrOut );

    int FillKeyStrFromJwkJson( const std::string& alg, const rapidjson::Value& key, std::string& keyStrOut );

    int EncodeStrToB64( const std::string& str, std::string& b64EncodedStr, bool includeNewLines = false );
}

#endif
