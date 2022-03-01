#include <openssl/bn.h> // TODO - upgrade to 1.1, support multiple versions
#include <openssl/pem.h> // TODO - upgrade to 1.1, support multiple versions

#include <lhwsutil/logging.h>
#include <lhwsutil_impl/rsa.h>

#include <stdexcept>

namespace LHWSUtilImplNS
{
    RSAPublicKey::RSAPublicKey( const std::vector< unsigned char >& nBytes,
                                const std::vector< unsigned char >& eBytes )
    :   rsa( nullptr )
    {
        if( nBytes.empty() || eBytes.empty() )
        {
            throw std::runtime_error( "n or e is empty" );
        }

        rsa = RSA_new();
        if( !( rsa ) )
        {
            throw std::runtime_error( "" );
        }

        rsa->n = BN_bin2bn( nBytes.data(), nBytes.size(), nullptr );
        rsa->e = BN_bin2bn( eBytes.data(), eBytes.size(), nullptr );
    }

    int RSAPublicKey::GetPEMFormatInto( std::string& pemOut ) const
    {
        BIO* bioMem = NULL;
        int rc = 0;
        
        try
        {
            wsUtilLogSetScope( "GetPEMFormatInto" );

            char* bioMemData = nullptr;
            long bioMemDataLength = 0;

            bioMem = BIO_new( BIO_s_mem() );
            if( !( bioMem ) )
            {
                wsUtilLogFatal( "failed to create new bio" );
                return 1;
            }

            rc = BIO_set_close( bioMem, BIO_CLOSE );

            rc = PEM_write_bio_RSA_PUBKEY( bioMem, rsa );
            if( rc != 1 )
            {
                wsUtilLogError( "failed to write rsa pubkey to bio" );
                return 2;
            }

            bioMemDataLength = BIO_get_mem_data( bioMem, &bioMemData );
            if( bioMemDataLength <= 0 || !( bioMemData ) )
            {
                wsUtilLogError( "failed to get bio mem data" );
                return 3;
            }

            pemOut.assign( bioMemData, bioMemDataLength );

            BIO_vfree( bioMem );

            return 0;
        }
        catch( ... )
        {
            if( bioMem )
            {
                BIO_vfree( bioMem );
            }

            return 4;
        }
    }

    RSAPublicKey::~RSAPublicKey()
    {
        if( rsa )
        {
            RSA_free( rsa );
            rsa = nullptr;
        }
    }
}
