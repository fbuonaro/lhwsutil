#include <lhwsutil/ijwtissuercache.h>

namespace LHWSUtilNS
{
    IJwtIssuer::IJwtIssuer()
    {
    }

    IJwtIssuer::~IJwtIssuer()
    {
    }

    JwtIssuerCacheParams::JwtIssuerCacheParams()
    :   iss()
    ,   clientAuthzBearerToken()
    ,   algToKeyPem()
    ,   pulldownOpenIdConfiguration( false )
    {
    }

    IJwtIssuerCache::IJwtIssuerCache()
    {
    }

    IJwtIssuerCache::~IJwtIssuerCache()
    {
    }
}
