#ifndef __LHWSUTIL_IMPL_RSA_H__
#define __LHWSUTIL_IMPL_RSA_H__

#include <openssl/rsa.h> // TODO - upgrade to 1.1, support multiple versions

#include <string>
#include <vector>

namespace LHWSUtilImplNS
{
    class RSAPublicKey
    {
        public:
            // big endian encoded integers
            RSAPublicKey( const std::vector< unsigned char >& nBytes,
                          const std::vector< unsigned char >& eBytes );
            ~RSAPublicKey();

            RSAPublicKey( const RSAPublicKey& other ) = delete;
            RSAPublicKey& operator=( const RSAPublicKey& other ) = delete;
            RSAPublicKey( RSAPublicKey&& other ) = delete;
            RSAPublicKey() = delete;

            int GetPEMFormatInto( std::string& pemOut ) const;

        private:
            RSA* rsa; 
    };
}

#endif
