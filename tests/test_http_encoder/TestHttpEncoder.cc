#include <gtest/gtest.h>

#include "HttpEncoder.hh"
#include "uint128.hh"

namespace
{
    TEST(TestHttpEncoder, Encode)
    {
        std::string url = "https://webex.com/11259375/meeting1234/user3213";

        uint128 encoded = HttpEncoder::EncodeUrl(url);
        std::string res = encoded.ToDecimalString();
        std::string actual = "1237981375469430707200000000000000000000";
        ASSERT_EQ(res, actual);
    }

    TEST(TestHttpEncoder, EncodingOutOfRangeError)
    {
        EXPECT_THROW({
            std::string url = "https://webex.com/11259375/meeting65536/user3213";
            HttpEncoder::EncodeUrl(url);
        }, HttpEncodeOutOfRangeException);
    }

    TEST(TestHttpEncoder, EncodingNoMatchError)
    {
        EXPECT_THROW({
            std::string url = "https://webex.com/123/12312/3232/meeting2132/user3213";
            HttpEncoder::EncodeUrl(url);
        }, HttpEncodeNoMatchException);
    }

    TEST(TestHttpEncoder, Decode)
    {
        std::string actual = "https://webex.com/1/123/meeting555/user777";
        std::string encoded = "0000000110157536742700648518346341351424";
        std::string decoded = HttpEncoder::DecodeUrl(encoded);
        ASSERT_EQ(decoded, actual);
    }

    TEST(TestHttpEncoder, DecodingErrors)
    {
        EXPECT_THROW({
            std::string url = "https://webex.com/11259375/meeting1234/user3213";

            uint128 encoded = HttpEncoder::EncodeUrl(url);
            std::string res = encoded.ToDecimalString();

            std::string decoded = HttpEncoder::DecodeUrl(res, 65);
        }, uint128Exception);

        EXPECT_THROW({
            std::string url = "https://webex.com/11259375/meeting1234/user3213";

            uint128 encoded = HttpEncoder::EncodeUrl(url);
            std::string res = encoded.ToDecimalString();

            std::string decoded = HttpEncoder::DecodeUrl(res, 33);
        }, uint128Exception);

        EXPECT_THROW({
            std::string url = "https://webex.com/2/meeting1234/user3213";

            uint128 encoded = HttpEncoder::EncodeUrl(url);
            std::string res = encoded.ToDecimalString();

            std::string decoded = HttpEncoder::DecodeUrl(res);
        }, HttpDecodeNoMatchException);
    }
}