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

    namespace
    {
        int decodeB64( const std::string& in, std::string& out, bool includeNewLines = false )
        {
            int rc = 0;
            std::vector< unsigned char > vec;

            rc = LHWSUtilImplNS::DecodeB64Str( in, vec, includeNewLines );
            if ( ( rc == 0 ) && vec.size() )
            {
                out.assign( reinterpret_cast<char*>( vec.data() ), vec.size() );
            }

            return rc;
        }
    }

    TEST( TestLHWSUtil, TestEncodeStrToB641 )
    {
        int rc = 0;
        std::string inputToEncode;
        std::string encodedOutput;

        inputToEncode = "";
        rc = LHWSUtilImplNS::EncodeStrToB64( inputToEncode, encodedOutput );
        ASSERT_EQ( 0, rc ) << "rc=" << rc << " for inputToEncode=[" << inputToEncode << "]";

        std::string decodedInput;
        rc = decodeB64( encodedOutput, decodedInput );
        ASSERT_EQ( 0, rc ) << "rc=" << rc << " for encodedOutput=[" << encodedOutput << "]";
        ASSERT_EQ( inputToEncode, decodedInput );
    }

    TEST( TestLHWSUtil, TestEncodeStrToB642 )
    {
        int rc = 0;
        std::string inputToEncode;
        std::string encodedOutput;

        inputToEncode = "a";
        rc = LHWSUtilImplNS::EncodeStrToB64( inputToEncode, encodedOutput );
        ASSERT_EQ( 0, rc ) << "rc=" << rc << " for inputToEncode=[" << inputToEncode << "]";

        std::string decodedInput;
        rc = decodeB64( encodedOutput, decodedInput );
        ASSERT_EQ( 0, rc ) << "rc=" << rc << " for encodedOutput=[" << encodedOutput << "]";
        ASSERT_EQ( inputToEncode, decodedInput );
    }

    TEST( TestLHWSUtil, TestEncodeStrToB643 )
    {
        int rc = 0;
        std::string inputToEncode;
        std::string encodedOutput;

        inputToEncode = "abc";
        rc = LHWSUtilImplNS::EncodeStrToB64( inputToEncode, encodedOutput );
        ASSERT_EQ( 0, rc ) << "rc=" << rc << " for inputToEncode=[" << inputToEncode << "]";

        std::string decodedInput;
        rc = decodeB64( encodedOutput, decodedInput );
        ASSERT_EQ( 0, rc ) << "rc=" << rc << " for encodedOutput=[" << encodedOutput << "]";
        ASSERT_EQ( inputToEncode, decodedInput );
    }

    TEST( TestLHWSUtil, TestEncodeStrToB644 )
    {
        int rc = 0;
        std::string inputToEncode;
        std::string encodedOutput;

        inputToEncode = "abcd";
        rc = LHWSUtilImplNS::EncodeStrToB64( inputToEncode, encodedOutput );
        ASSERT_EQ( 0, rc ) << "rc=" << rc << " for inputToEncode=[" << inputToEncode << "]";

        std::string decodedInput;
        rc = decodeB64( encodedOutput, decodedInput );
        ASSERT_EQ( 0, rc ) << "rc=" << rc << " for encodedOutput=[" << encodedOutput << "]";
        ASSERT_EQ( inputToEncode, decodedInput );
    }

    TEST( TestLHWSUtil, TestEncodeStrToB645 )
    {
        int rc = 0;
        std::string inputToEncode;
        std::string encodedOutput;

        inputToEncode = "abcabcabcabcabcabc";
        rc = LHWSUtilImplNS::EncodeStrToB64( inputToEncode, encodedOutput, true );
        ASSERT_EQ( 0, rc ) << "rc=" << rc << " for inputToEncode=[" << inputToEncode << "]";

        std::string decodedInput;
        rc = decodeB64( encodedOutput, decodedInput, true );
        ASSERT_EQ( 0, rc ) << "rc=" << rc << " for encodedOutput=[" << encodedOutput << "]";
        ASSERT_EQ( inputToEncode, decodedInput );
    }

    TEST( TestLHWSUtil, TestEncodeStrToB646 )
    {
        int rc = 0;
        std::string inputToEncode;
        std::string encodedOutput;

        inputToEncode = "abcabcabcabcabcabcA";
        rc = LHWSUtilImplNS::EncodeStrToB64( inputToEncode, encodedOutput, true );
        ASSERT_EQ( 0, rc ) << "rc=" << rc << " for inputToEncode=[" << inputToEncode << "]";

        std::string decodedInput;
        rc = decodeB64( encodedOutput, decodedInput, true );
        ASSERT_EQ( 0, rc ) << "rc=" << rc << " for encodedOutput=[" << encodedOutput << "]";
        ASSERT_EQ( inputToEncode, decodedInput );
    }

    TEST( TestLHWSUtil, TestEncodeStrToB647 )
    {
        int rc = 0;
        std::string inputToEncode;
        std::string encodedOutput;

        inputToEncode = "abcabcabcabcabcabcAB";
        rc = LHWSUtilImplNS::EncodeStrToB64( inputToEncode, encodedOutput, true );
        ASSERT_EQ( 0, rc ) << "rc=" << rc << " for inputToEncode=[" << inputToEncode << "]";

        std::string decodedInput;
        rc = decodeB64( encodedOutput, decodedInput, true );
        ASSERT_EQ( 0, rc ) << "rc=" << rc << " for encodedOutput=[" << encodedOutput << "]";
        ASSERT_EQ( inputToEncode, decodedInput );
    }
}
