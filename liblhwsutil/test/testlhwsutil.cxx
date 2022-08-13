#include <gtest/gtest.h>

#include <lhwsutil_impl/jwtvalidator.h>
#include <lhwsutil_impl/simplehttpclientcurl.h>
#include <lhwsutil_impl/jwtutils.h>

namespace TestLHThingAPINS
{
    TEST( TestLHWSUtil, Test1 )
    {
        LHWSUtilImplNS::JwtValidatorFactory jwtValidatorFactory;
        auto jwtValidator = jwtValidatorFactory.CreateJwtValidator();
        auto validJwt = jwtValidator->ValidateIntoJwt( "abc" );
        ASSERT_TRUE( true );
    }
}
