#include <lhwsutil/ijwtvalidator.h>
#include <lhwsutil/logging.h>

#include <sstream>

namespace LHWSUtilNS
{
    IValidJwt::IValidJwt()
    {
    }

    IValidJwt::~IValidJwt()
    {
    }

    IJwtValidator::IJwtValidator()
    {
    }

    IJwtValidator::~IJwtValidator()
    {
    }

    IJwtValidatorFactory::IJwtValidatorFactory()
    {
    }

    IJwtValidatorFactory::~IJwtValidatorFactory()
    {
    }

    int GetIdentifiers( const IValidJwt& jwt, std::string& username, std::string& email, bool& emailVerified )
    {
        wsUtilLogSetScope( "GetIdentifiers" );
        int rc = 0;

        std::string _username;
        std::string _email;
        bool _emailVerified;
        
        rc = jwt.GetGrantStrValue( "preferred_username", _username );
        if( rc != 0 )
        {
            wsUtilLogError( "jwt is missing preferred_username" );
            return 1;
        }   

        rc = jwt.GetGrantStrValue( "email", _email );
        if( rc != 0 )
        {
            wsUtilLogError( "jwt is missing email" );
            return 2;
        }   

        rc = jwt.GetGrantBoolValue( "email_verified", _emailVerified );
        if( rc != 0 )
        {
            wsUtilLogError( "jwt is missing email_verified" );
            return 3;
        }   

        email = std::move( _email );
        username = std::move( _username );
        emailVerified = _emailVerified;

        return 0;
    }

    int GetScopes( const IValidJwt& jwt, std::unordered_set< std::string >& scopes )
    {
        int rc = 0;
        std::string scopesStr;

        rc = jwt.GetGrantStrValue( "scope", scopesStr );
        if( rc != 0 )
        {
            wsUtilLogError( "jwt is missing scope" );
            return 1;
        }

        size_t pos = scopesStr.find( ' ' ); // end of token
        if( pos != std::string::npos )
        {
            if( pos > 0 )
            {
                scopes.emplace( scopesStr.substr( 0, pos ) );
            }

            size_t lastPos = pos;
            pos = scopesStr.find( ' ' ); // end of token
            while( pos != std::string::npos )
            {
                if( pos > ( lastPos + 1 ) )
                {
                    scopes.emplace( scopesStr.substr( lastPos + 1, pos - ( lastPos + 1 ) ) );
                }

                lastPos = pos;
                if( lastPos >= scopesStr.size() - 1 )
                    break;
                pos = scopesStr.find( ' ', lastPos + 1 );
            }

            if( lastPos < scopesStr.size() )
            {
                scopes.emplace( scopesStr.substr( lastPos + 1 ) );
            }
        }

        
        return 0;
    }

}
