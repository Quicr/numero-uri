#include <gtest/gtest.h>
#include "UrlEncoder.hh"
#include "big_uint.hh"
#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include <iostream>

namespace
{
    class TestUrlEncoder : public ::testing::Test
    {
    protected:
        TestUrlEncoder()
        {
            // Note, this does get tested later
            encoder.AddTemplate(std::string("https://!{www.}!webex.com/<int24="
                "11259375>/meeting<int16>/user<int16>"));
            encoder.AddTemplate(std::string("https://webex.com/<int24=1>/"
                "<int16>/meeting<int16>/user<int16>"));
            encoder.AddTemplate(std::string("https://!{www.}!webex.com/"
                "<int24=16777215>/party<int5>/building<int3>/floor<int39>"
                "/room<int25>/meeting<int32>"));
        }

        ~TestUrlEncoder() = default;

        UrlEncoder encoder;
    };

    TEST_F(TestUrlEncoder, Encode)
    {
        std::string url = "https://webex.com/11259375/meeting1234/user3213";

        big_uint encoded = encoder.EncodeUrl(url);
        std::string res = encoded.ToDecimalString();
        std::string actual = "1237981375469430707200000000000000000000";
        ASSERT_EQ(res, actual);
    }

    TEST_F(TestUrlEncoder, EncodeUse128Bits)
    {
        std::string url = "https://www.webex.com/16777215/party31/building7/"
            "floor549755813887/room33554431/meeting4294967295";

        big_uint encoded = encoder.EncodeUrl(url);
        std::string res = encoded.ToDecimalString();
        std::string actual = "1844674407370955161518446744073709551615";
        ASSERT_EQ(res, actual);
    }

    TEST_F(TestUrlEncoder, EncodingOutOfRangeError)
    {
        EXPECT_THROW({
            std::string url =
                "https://webex.com/11259375/meeting65536/user3213";
            big_uint encoded = encoder.EncodeUrl(url);
        }, UrlEncoderOutOfRangeException);
    }

    TEST_F(TestUrlEncoder, EncodingNoMatchError)
    {
        EXPECT_THROW({
            std::string url = "https://webex.com/123/12312/3232/meeting2132/"
                "user3213";
            big_uint encoded = encoder.EncodeUrl(url);
        }, UrlEncoderNoMatchException);
    }

    TEST_F(TestUrlEncoder, Decode)
    {
        std::string actual = "https://webex.com/1/123/meeting555/user777";
        std::string encoded = "0000000110157536742700648518346341351424";
        std::string decoded = encoder.DecodeUrl(encoded);
        ASSERT_EQ(decoded, actual);
    }

    TEST_F(TestUrlEncoder, DecodingErrors)
    {
        EXPECT_THROW({
            encoder.AddTemplate(std::string("https://webex.com/<int24=2>/"
                "meeting<int16>/user<int16>"));
            std::string url = "https://webex.com/2/meeting1234/user3213";

            big_uint encoded = encoder.EncodeUrl(url);
            std::string res = encoded.ToDecimalString();

            encoder.RemoveTemplate(2);
            std::string decoded = encoder.DecodeUrl(res);
        }, UrlDecodeNoMatchException);
    }

    TEST_F(TestUrlEncoder, AddTemplate)
    {
        encoder.AddTemplate(std::string("https://!{www.}!webex.com/"
            "<int24=11259376>/meeting<int16>/user<int16>"));

        std::cout << encoder.GetTemplates().size() << std::endl;
        std::cout << encoder.GetTemplates().size() << std::endl;
        std::cout << encoder.GetTemplates().size() << std::endl;
        std::cout << encoder.GetTemplates().size() << std::endl;
        std::cout << encoder.GetTemplates().size() << std::endl;
        std::cout << encoder.GetTemplates().size() << std::endl;
        std::cout << encoder.GetTemplates().size() << std::endl;
        std::cout << encoder.GetTemplates().size() << std::endl;
        std::cout << encoder.GetTemplates().size() << std::endl;
        std::cout << encoder.GetTemplates().size() << std::endl;

        // Since we already have 3 templates from the constructor we are only
        // adding 1 extra.
        ASSERT_EQ(4, encoder.GetTemplates().size());
    }

    TEST_F(TestUrlEncoder, AddBigTemplate)
    {
        encoder.AddTemplate(std::string("https://!{www.}!webex.com/"
            "<int24=16777216>/party<int5>/building<int3>/floor<int39>"
            "/room<int25>/meeting<int32>"));

        // There should be only 1 template with this PEN so just grab it
        auto output_template = encoder.GetTemplate(16777216).at(-1);
        std::string actual_url = "^https://(?:www\\.)?webex.com/(\\d+)"
            "/party(\\d+)/building(\\d+)/floor(\\d+)/room(\\d+)/meeting(\\d+)$";
        std::vector<uint32_t> actual_bits = {5,3,39,25,32};

        ASSERT_EQ(actual_url, output_template.url);
        ASSERT_EQ(actual_bits, output_template.bits);
    }

    TEST_F(TestUrlEncoder, RemoveTemplate)
    {
        encoder.AddTemplate(std::string("https://!{www.}!webex.com/"
            "<int24=11259376>/meeting<int16>/user<int16>"));
        encoder.RemoveTemplate(11259376);
        ASSERT_EQ(3, encoder.GetTemplates().size());
    }

    TEST_F(TestUrlEncoder, GetTemplate)
    {
        encoder.AddTemplate(std::string("https://!{www.}!webex.com/<int24=23>"
            "/meeting<int16>/user<int16>"));

        auto output_template = encoder.GetTemplate(23).at(-1);

        ASSERT_EQ("^https://(?:www\\.)?webex.com/(\\d+)/meeting(\\d+)/"
            "user(\\d+)$", output_template.url);
        ASSERT_EQ(std::vector<std::uint32_t>({24, 16, 16}),
            output_template.bits);
    }

    TEST_F(TestUrlEncoder, GetTemplates)
    {
        encoder.AddTemplate(std::string("https://!{www.}!webex.com/"
            "<int24=11259374>/meeting<int16>/user<int16>"));

        auto output = encoder.GetTemplate(11259374).at(-1);

        ASSERT_EQ("^https://(?:www\\.)?webex.com/(\\d+)/meeting(\\d+)"
            "/user(\\d+)$", output.url);
        ASSERT_EQ(std::vector<std::uint32_t>({24, 16, 16}), output.bits);
    }

    TEST_F(TestUrlEncoder, TemplatesToJson)
    {
        UrlEncoder tmp_encoder;
        tmp_encoder.AddTemplate(std::string("https://!{www.}!webex.com/"
            "<int24=11259374>/meeting<int16>/user<int16>"));
        json temp;
        temp["pen"] = 11259374;

        json sub_temp;
        sub_temp["url"] = "^https://(?:www\\.)?webex.com/(\\d+)/meeting(\\d+)"
            "/user(\\d+)$";
        sub_temp["bits"] = {24, 16, 16};
        sub_temp["sub_pen"] = -1;

        temp["templates"].push_back(sub_temp);

        json real;
        real.push_back(temp);

        ASSERT_EQ(tmp_encoder.TemplatesToJson(), real);
    }

    TEST_F(TestUrlEncoder, TemplatesFromJson)
    {
        json temp;
        temp["pen"] = 11259374;

        json sub_temp;
        sub_temp["url"] = "^https://(?:www\\.)?webex.com/(\\d+)/meeting(\\d+)"
            "/user(\\d+)$";
        sub_temp["bits"] = {24, 16, 16};
        sub_temp["sub_pen"] = -1;

        temp["templates"].push_back(sub_temp);

        json j;
        j.push_back(temp);
        encoder.TemplatesFromJson(j);

        auto output = encoder.GetTemplate(11259374).at(-1);

        ASSERT_EQ("^https://(?:www\\.)?webex.com/(\\d+)/meeting(\\d+)"
            "/user(\\d+)$",
            output.url);
        ASSERT_EQ(std::vector<std::uint32_t>({24, 16, 16}), output.bits);
    }

    TEST_F(TestUrlEncoder, Clear)
    {
        encoder.Clear();

        ASSERT_EQ(encoder.GetTemplates().size(), 0);
    }
}