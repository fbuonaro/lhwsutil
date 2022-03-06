#ifndef __LHWSUTIL_IJWTVALIDATOR_H__
#define __LHWSUTIL_IJWTVALIDATOR_H__

#include <memory>
#include <string>
#include <unordered_set>

namespace LHWSUtilNS
{
    class IValidJwt
    {
        public:
            IValidJwt();
            virtual ~IValidJwt();

            virtual int GetGrantBoolValue( const std::string& grant,
                                           bool& valueOut ) const = 0;
            virtual int GetGrantIntValue( const std::string& grant,
                                          long& valueOut ) const = 0;
            virtual int GetGrantJsonValue( const std::string& grant,
                                           std::string& valueOut ) const = 0;
            virtual int GetGrantStrValue( const std::string& grant,
                                          std::string& valueOut ) const = 0;
            virtual void ToString( std::string& out, bool prettyPrint ) const = 0;
    };

    class IJwtValidator
    {
        public:
            IJwtValidator();
            virtual ~IJwtValidator();

            // return 0 if valid
            // return !=0 and set errorStr if invalid
            virtual std::unique_ptr< IValidJwt > ValidateIntoJwt( const std::string& b64UrlEncodedJwt ) const = 0;

            virtual std::unique_ptr< LHWSUtilNS::IValidJwt > IntrospectJwt( const std::string& b64UrlEncodedJwt ) const = 0;
    };

    class IJwtValidatorFactory
    {
        public:
            IJwtValidatorFactory();
            virtual ~IJwtValidatorFactory();

            virtual std::unique_ptr< IJwtValidator > CreateJwtValidator() const = 0;
    };

    std::shared_ptr< IJwtValidatorFactory > GetStandardJwtValidatorFactory();

    struct UserIdentifiers
    {
        UserIdentifiers();

        std::string username;
        std::string email;
        bool emailVerified;
        std::string sub;
    };

    int GetIdentifiers( const IValidJwt& jwt, UserIdentifiers& userIdentifiers );

    int GetScopes( const IValidJwt& jwt, std::unordered_set< std::string >& scopes );
}

#include <lhmiscutil/singleton.h>

namespace LHMiscUtilNS
{
    EnableClassAsSingleton( LHWSUtilNS::IJwtValidatorFactory, SingletonCanBeSet::WhenEmpty );
}

#endif
