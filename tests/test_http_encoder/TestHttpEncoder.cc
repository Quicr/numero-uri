#include <gtest/gtest.h>
#include "UrlTemplater.hh"
#include "HttpEncoder.hh"
#include "uint128.hh"

namespace
{
    class TestHttpEncoder : public ::testing::Test
    {
    protected:
        TestHttpEncoder()
        {
            templater.Add("https://!{www.}!webex.com/<int24=11259375>/meeting<int16>/user<int16>");
            templater.Add("https://webex.com/<int24=1>/<int16>/meeting<int16>/user<int16>");
            templater.Add("https://!{www.}!webex.com/<int24=16777215>/party<int5>/building<int3>/floor<int39>/room<int25>/meeting<int32>");
        }

        ~TestHttpEncoder() = default;

        HttpEncoder encoder;
        UrlTemplater templater;
    };

    TEST_F(TestHttpEncoder, Encode)
    {
        std::string url = "https://webex.com/11259375/meeting1234/user3213";

        uint128 encoded = encoder.EncodeUrl(url, templater.GetTemplates());
        std::string res = encoded.ToDecimalString();
        std::string actual = "1237981375469430707200000000000000000000";
        ASSERT_EQ(res, actual);
    }

    TEST_F(TestHttpEncoder, EncodeUse128Bits)
    {
        std::string url = "https://www.webex.com/16777215/party31/building7/floor549755813887/room33554431/meeting4294967295";

        uint128 encoded = encoder.EncodeUrl(url, templater.GetTemplates());
        std::string res = encoded.ToDecimalString();
        std::string actual = "1844674407370955161518446744073709551615";
        ASSERT_EQ(res, actual);
    }

    TEST_F(TestHttpEncoder, EncodingOutOfRangeError)
    {
        EXPECT_THROW({
            std::string url = "https://webex.com/11259375/meeting65536/user3213";
            uint128 encoded = encoder.EncodeUrl(url, templater.GetTemplates());
        }, HttpEncodeOutOfRangeException);
    }

    TEST_F(TestHttpEncoder, EncodingNoMatchError)
    {
        EXPECT_THROW({
            std::string url = "https://webex.com/123/12312/3232/meeting2132/user3213";
            uint128 encoded = encoder.EncodeUrl(url, templater.GetTemplates());
        }, HttpEncodeNoMatchException);
    }

    TEST_F(TestHttpEncoder, Decode)
    {
        std::string actual = "https://webex.com/1/123/meeting555/user777";
        std::string encoded = "0000000110157536742700648518346341351424";
        std::string decoded = encoder.DecodeUrl(encoded, templater.GetTemplates());
        ASSERT_EQ(decoded, actual);
    }

    TEST_F(TestHttpEncoder, DecodingErrors)
    {
        EXPECT_THROW({
            std::string url = "https://webex.com/11259375/meeting1234/user3213";

            uint128 encoded = encoder.EncodeUrl(url, templater.GetTemplates());
            std::string res = encoded.ToDecimalString();

            std::string decoded = encoder.DecodeUrl(res, templater.GetTemplates(), 65);
        }, uint128Exception);

        EXPECT_THROW({
            std::string url = "https://webex.com/11259375/meeting1234/user3213";

            uint128 encoded = encoder.EncodeUrl(url, templater.GetTemplates());
            std::string res = encoded.ToDecimalString();

            std::string decoded = encoder.DecodeUrl(res, templater.GetTemplates(), 33);
        }, uint128Exception);

        EXPECT_THROW({
            templater.Add("https://webex.com/<int24=2>/meeting<int16>/user<int16>");
            std::string url = "https://webex.com/2/meeting1234/user3213";

            uint128 encoded = encoder.EncodeUrl(url, templater.GetTemplates());
            std::string res = encoded.ToDecimalString();

            templater.Remove(2);
            std::string decoded = encoder.DecodeUrl(res, templater.GetTemplates());
        }, HttpDecodeNoMatchException);
    }
}