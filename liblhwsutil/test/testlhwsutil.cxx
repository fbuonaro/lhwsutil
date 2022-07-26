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

    TEST( TestLHWSUtil, TestEncodeStrToB641 )
    {
        int rc = 0;
        std::string inputToEncode;
        std::string encodedOutput;

        inputToEncode = "";
        rc = LHWSUtilImplNS::EncodeStrToB64( inputToEncode, encodedOutput );
        ASSERT_EQ( 0, rc ) << "rc=" << rc << " for inputToEncode=[" << inputToEncode << "]";
        std::cout << "encoding=[" << encodedOutput << "] for input=[" << inputToEncode << "]" << std::endl;
    }

    TEST( TestLHWSUtil, TestEncodeStrToB642 )
    {
        int rc = 0;
        std::string inputToEncode;
        std::string encodedOutput;

        inputToEncode = "a";
        rc = LHWSUtilImplNS::EncodeStrToB64( inputToEncode, encodedOutput );
        ASSERT_EQ( 0, rc ) << "rc=" << rc << " for inputToEncode=[" << inputToEncode << "]";
        std::cout << "encoding=[" << encodedOutput << "] for input=[" << inputToEncode << "]" << std::endl;
    }

    TEST( TestLHWSUtil, TestEncodeStrToB643 )
    {
        int rc = 0;
        std::string inputToEncode;
        std::string encodedOutput;

        inputToEncode = "abc";
        rc = LHWSUtilImplNS::EncodeStrToB64( inputToEncode, encodedOutput );
        ASSERT_EQ( 0, rc ) << "rc=" << rc << " for inputToEncode=[" << inputToEncode << "]";
        std::cout << "encoding=[" << encodedOutput << "] for input=[" << inputToEncode << "]" << std::endl;
    }

    TEST( TestLHWSUtil, TestEncodeStrToB644 )
    {
        int rc = 0;
        std::string inputToEncode;
        std::string encodedOutput;

        inputToEncode = "abcd";
        rc = LHWSUtilImplNS::EncodeStrToB64( inputToEncode, encodedOutput );
        ASSERT_EQ( 0, rc ) << "rc=" << rc << " for inputToEncode=[" << inputToEncode << "]";
        std::cout << "encoding=[" << encodedOutput << "] for input=[" << inputToEncode << "]" << std::endl;
    }

    TEST( TestLHWSUtil, TestEncodeStrToB645 )
    {
        int rc = 0;
        std::string inputToEncode;
        std::string encodedOutput;

        inputToEncode = "abcabcabcabcabcabc";
        rc = LHWSUtilImplNS::EncodeStrToB64( inputToEncode, encodedOutput );
        ASSERT_EQ( 0, rc ) << "rc=" << rc << " for inputToEncode=[" << inputToEncode << "]";
        std::cout << "encoding=[" << encodedOutput << "] for input=[" << inputToEncode << "]" << std::endl;
    }

    TEST( TestLHWSUtil, TestEncodeStrToB646 )
    {
        int rc = 0;
        std::string inputToEncode;
        std::string encodedOutput;

        inputToEncode = "abcabcabcabcabcabcA";
        rc = LHWSUtilImplNS::EncodeStrToB64( inputToEncode, encodedOutput );
        ASSERT_EQ( 0, rc ) << "rc=" << rc << " for inputToEncode=[" << inputToEncode << "]";
        std::cout << "encoding=[" << encodedOutput << "] for input=[" << inputToEncode << "]" << std::endl;
    }

    TEST( TestLHWSUtil, TestEncodeStrToB647 )
    {
        int rc = 0;
        std::string inputToEncode;
        std::string encodedOutput;

        inputToEncode = "abcabcabcabcabcabcAB";
        rc = LHWSUtilImplNS::EncodeStrToB64( inputToEncode, encodedOutput );
        ASSERT_EQ( 0, rc ) << "rc=" << rc << " for inputToEncode=[" << inputToEncode << "]";
        std::cout << "encoding=[" << encodedOutput << "] for input=[" << inputToEncode << "]" << std::endl;
    }
}
