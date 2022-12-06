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
            encoder.AddTemplate(std::string("https://!{www.}!webex.com<pen="
                "11259375>/meeting<int16>/user<int16>"));
            encoder.AddTemplate(std::string("https://webex.com<pen=1>/"
                "<int16>/meeting<int16>/user<int16>"));
            encoder.AddTemplate(std::string("https://!{www.}!webex.com"
                "<pen=16777215>/party<int5>/building<int3>/floor<int39>"
                "/room<int25>/meeting<int32>"));
        }

        ~TestUrlEncoder() = default;

        UrlEncoder encoder;
    };

    TEST_F(TestUrlEncoder, Encode)
    {
        std::string url = "https://www.webex.com/meeting1234/user3213";

        big_uint encoded = encoder.EncodeUrl(url);
        std::string res = encoded.ToDecimalString();
        std::string actual = "0d228367256013035201762680357424937828352";
        ASSERT_EQ(res, actual);
    }

    TEST_F(TestUrlEncoder, EncodeUse128Bits)
    {
        std::string url = "https://www.webex.com/party31/building7/"
            "floor549755813887/room33554431/meeting4294967295";

        big_uint encoded = encoder.EncodeUrl(url);
        std::string res = encoded.ToDecimalString();
        std::string actual = "0d340282366920938463463374607431768211455";
        ASSERT_EQ(res, actual);
    }

    TEST_F(TestUrlEncoder, EncodingOutOfRangeError)
    {
        EXPECT_THROW({
            std::string url =
                "https://webex.com/meeting65536/user3213";
            big_uint encoded = encoder.EncodeUrl(url);
        }, UrlEncoderOutOfRangeException);
    }

    TEST_F(TestUrlEncoder, EncodingNoMatchError)
    {
        EXPECT_THROW({
            std::string url = "https://webex.com/3232/test_meeting2132/"
                "user3213";
            big_uint encoded = encoder.EncodeUrl(url);
        }, UrlEncoderNoMatchException);
    }

    TEST_F(TestUrlEncoder, Decode)
    {
        std::string actual = "https://www.webex.com/meeting555/user777";
        std::string encoded = "0d228367255802883376409234785305070927872";
        std::string decoded = encoder.DecodeUrl(encoded);
        ASSERT_EQ(decoded, actual);
    }

    TEST_F(TestUrlEncoder, DecodingErrors)
    {
        EXPECT_THROW({
            encoder.AddTemplate(std::string("https://webex.com<pen=2>/"
                "meeting<int16>/user<int16>"));
            std::string url = "https://webex.com/meeting1234/user3213";

            big_uint encoded = encoder.EncodeUrl(url);
            std::string res = encoded.ToDecimalString();

            encoder.RemoveTemplate(2);
            std::string decoded = encoder.DecodeUrl(res);
        }, UrlDecodeNoMatchException);
    }

    TEST_F(TestUrlEncoder, AddTemplate)
    {
        encoder.AddTemplate(std::string("https://!{www.}!webex.com/"
            "<pen=11259376>/meeting<int16>/user<int16>"));

        // Since we already have 3 templates from the constructor we are only
        // adding 1 extra.
        ASSERT_EQ(4, encoder.GetTemplates().size());
    }

    TEST_F(TestUrlEncoder, AddBigTemplate)
    {
        encoder.AddTemplate(std::string("https://!{www.}!webex.com/"
            "<pen=16777215>/party<int5>/building<int3>/floor<int39>"
            "/room<int25>/meeting<int32>"));

        // There should be only 1 template with this PEN so just grab it
        auto output_template = encoder.GetTemplate(16777215).at(-1);
        std::string actual_url = "^https://(?:www\\.)?webex.com"
            "/party(\\d+)/building(\\d+)/floor(\\d+)/room(\\d+)/meeting(\\d+)$";
        std::vector<uint32_t> actual_bits = {5,3,39,25,32};

        ASSERT_EQ(actual_url, output_template.url);
        ASSERT_EQ(actual_bits, output_template.bits);
    }

    TEST_F(TestUrlEncoder, RemoveTemplate)
    {
        encoder.AddTemplate(std::string("https://!{www.}!webex.com/"
            "<pen=11259376>/meeting<int16>/user<int16>"));
        encoder.RemoveTemplate(11259376);
        ASSERT_EQ(3, encoder.GetTemplates().size());
    }

    TEST_F(TestUrlEncoder, GetTemplate)
    {
        encoder.AddTemplate(std::string("https://!{www.}!webex.com<pen=23>"
            "/meeting<int16>/user<int16>"));

        auto output_template = encoder.GetTemplate(23).at(-1);

        ASSERT_EQ("^https://(?:www\\.)?webex.com/meeting(\\d+)/user(\\d+)$",
            output_template.url);
        ASSERT_EQ(std::vector<std::uint32_t>({16, 16}),
            output_template.bits);
    }

    TEST_F(TestUrlEncoder, GetTemplates)
    {
        encoder.AddTemplate(std::string("https://!{www.}!webex.com"
            "<pen=11259374>/meeting<int16>/user<int16>"));

        auto output = encoder.GetTemplate(11259374).at(-1);

        ASSERT_EQ("^https://(?:www\\.)?webex.com/meeting(\\d+)"
            "/user(\\d+)$", output.url);
        ASSERT_EQ(std::vector<std::uint32_t>({16, 16}), output.bits);
    }

    TEST_F(TestUrlEncoder, TemplatesToJson)
    {
        UrlEncoder tmp_encoder;
        tmp_encoder.AddTemplate(std::string("https://!{www.}!webex.com"
            "<pen=11259374>/meeting<int16>/user<int16>"));
        json temp;
        temp["pen"] = 11259374;

        json sub_temp;
        sub_temp["url"] = "^https://(?:www\\.)?webex.com/meeting(\\d+)"
            "/user(\\d+)$";
        sub_temp["bits"] = {16, 16};
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
        sub_temp["url"] = "^https://(?:www\\.)?webex.com/meeting(\\d+)"
            "/user(\\d+)$";
        sub_temp["bits"] = {16, 16};
        sub_temp["sub_pen"] = -1;

        temp["templates"].push_back(sub_temp);

        json j;
        j.push_back(temp);
        encoder.TemplatesFromJson(j);

        auto output = encoder.GetTemplate(11259374).at(-1);

        ASSERT_EQ("^https://(?:www\\.)?webex.com/meeting(\\d+)"
            "/user(\\d+)$",
            output.url);
        ASSERT_EQ(std::vector<std::uint32_t>({16, 16}), output.bits);
    }

    TEST_F(TestUrlEncoder, Clear)
    {
        encoder.Clear();

        ASSERT_EQ(encoder.GetTemplates().size(), 0);
    }
}