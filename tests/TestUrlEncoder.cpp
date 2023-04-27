#include <gtest/gtest.h>

#include <string>
#include <vector>

#include <UrlEncoder.h>
#include <nlohmann/json.hpp>

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
                                        "<int16>/party<int16>/user<int16>"));
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

    quicr::Name encoded = encoder.EncodeUrl(url);
    std::string actual = "0xabcdef04d20c8d000000000000000000";
    ASSERT_EQ(encoded, actual);
}

TEST_F(TestUrlEncoder, EncodeUse128Bits)
{
    std::string url = "https://www.webex.com/party31/building7/"
                      "floor549755813887/room33554431/meeting4294967295";

    quicr::Name encoded = encoder.EncodeUrl(url);
    std::string actual = "0xffffffffffffffffffffffffffffffff";
    ASSERT_EQ(encoded, actual);
}

TEST_F(TestUrlEncoder, EncodingOutOfRangeError)
{
    EXPECT_THROW(
        {
            std::string url = "https://webex.com/meeting65536/user3213";
            quicr::Name encoded = encoder.EncodeUrl(url);
        },
        UrlEncoderOutOfRangeException);
}

TEST_F(TestUrlEncoder, EncodingNoMatchError)
{
    EXPECT_THROW(
        {
            std::string url = "https://webex.com/3232/test_meeting2132/"
                              "user3213";
            quicr::Name encoded = encoder.EncodeUrl(url);
        },
        UrlEncoderNoMatchException);
}

TEST_F(TestUrlEncoder, Decode)
{
    std::string actual = "https://webex.com/meeting555/user777";
    std::string encoded = "0xabcdef022b0309000000000000000000";
    std::string decoded = encoder.DecodeUrl(encoded);
    ASSERT_EQ(decoded, actual);
}

TEST_F(TestUrlEncoder, Decode128bit)
{
    std::string actual = "https://webex.com/party31/building7/floor549755813887/"
                         "room33554431/meeting4294967295";
    std::string encoded = "0xffffffffffffffffffffffffffffffff";
    std::string decoded = encoder.DecodeUrl(encoded);
    ASSERT_EQ(decoded, actual);
}

TEST_F(TestUrlEncoder, DecodingErrors)
{
    EXPECT_THROW(
        {
            encoder.AddTemplate(std::string("https://webex.com<pen=2>/"
                                            "meeting<int16>/user<int16>"));
            std::string url = "https://webex.com/meeting1234/user3213";

            quicr::Name encoded = encoder.EncodeUrl(url);

            encoder.RemoveTemplate(2);
            std::string decoded = encoder.DecodeUrl(encoded);
        },
        UrlDecodeNoMatchException);
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
    std::string actual_url =
        "^https://(?:www\\.)?webex.com"
        "/party((?:0x|0d)?(?:[0-9ABCDEFabcdef]+|\\d+))/building((?:0x|0d)?(?:[0-9ABCDEFabcdef]+|\\d+))/"
        "floor((?:0x|0d)?(?:[0-9ABCDEFabcdef]+|\\d+))/room((?:0x|0d)?(?:[0-9ABCDEFabcdef]+|\\d+))/"
        "meeting((?:0x|0d)?(?:[0-9ABCDEFabcdef]+|\\d+))$";
    std::vector<uint32_t> actual_bits = {5, 3, 39, 25, 32};

    ASSERT_EQ(actual_url, output_template.url);
    ASSERT_EQ(actual_bits, output_template.bits);
}

TEST_F(TestUrlEncoder, AddVectorOfTemplates)
{
    UrlEncoder temp_encoder;
    std::vector<std::string> template_v;
    template_v.push_back("https://webex.com<pen=123>/meeting<int50>");
    template_v.push_back("https://webex.com<pen=124>/meeting<int50>");
    template_v.push_back("https://webex.com<pen=125>/meeting<int50>");
    template_v.push_back("https://webex.com<pen=126>/meeting<int50>");
    template_v.push_back("https://webex.com<pen=127>/meeting<int50>");

    temp_encoder.AddTemplate(template_v);

    ASSERT_EQ(temp_encoder.GetTemplates().size(), 5);
}

TEST_F(TestUrlEncoder, AddArrayOfTemplates)
{
    UrlEncoder temp_encoder;
    const std::uint64_t count = 5;
    std::string template_a[count];
    template_a[0] = "https://webex.com<pen=123>/meeting<int50>";
    template_a[1] = "https://webex.com<pen=124>/meeting<int50>";
    template_a[2] = "https://webex.com<pen=125>/meeting<int50>";
    template_a[3] = "https://webex.com<pen=126>/meeting<int50>";
    template_a[4] = "https://webex.com<pen=127>/meeting<int50>";

    temp_encoder.AddTemplate(template_a, count);

    ASSERT_EQ(temp_encoder.GetTemplates().size(), 5);
}

TEST_F(TestUrlEncoder, AddJsonOfTemplates)
{
    json temp;
    temp["pen"] = 777;

    json sub_temp;
    sub_temp["url"] = "^https://(?:www\\.)?webex.com/meeting((?:0x|0d)?(?:[0-9ABCDEFabcdef]+|\\d+))"
                      "/user((?:0x|0d)?(?:[0-9ABCDEFabcdef]+|\\d+))/fun((?:0x|0d)?(?:[0-9ABCDEFabcdef]+|\\d+))$";
    sub_temp["bits"] = {16, 16, 32};
    sub_temp["sub_pen"] = -1;

    temp["templates"].push_back(sub_temp);

    json j;
    j.push_back(temp);
    encoder.TemplatesFromJson(j);

    auto output = encoder.GetTemplate(777).at(-1);

    ASSERT_EQ("^https://(?:www\\.)?webex.com/meeting((?:0x|0d)?(?:[0-9ABCDEFabcdef]+|\\d+))"
              "/user((?:0x|0d)?(?:[0-9ABCDEFabcdef]+|\\d+))/fun((?:0x|0d)?(?:[0-9ABCDEFabcdef]+|\\d+))$",
              output.url);
    ASSERT_EQ(std::vector<std::uint32_t>({16, 16, 32}), output.bits);
}

TEST_F(TestUrlEncoder, RemoveTemplate)
{
    UrlEncoder temp_encoder;
    temp_encoder.AddTemplate(std::string("https://!{www.}!webex.com/"
                                         "<pen=11259376>/meeting<int16>/user<int16>"));
    temp_encoder.RemoveTemplate(11259376);
    ASSERT_EQ(0, temp_encoder.GetTemplates().size());
    temp_encoder.AddTemplate(std::string("https://!{www.}!webex.com/"
                                         "<pen=777><sub_pen=10>/meeting<int16>/user<int16>"));
    temp_encoder.RemoveSubTemplate(777, 10);
    ASSERT_EQ(0, temp_encoder.GetTemplates().size());
}

TEST_F(TestUrlEncoder, TemplatesToJson)
{
    UrlEncoder temp_encoder;
    temp_encoder.AddTemplate(std::string("https://!{www.}!webex.com"
                                         "<pen=11259374>/meeting<int16>/user<int16>"));
    json temp;
    temp["pen"] = 11259374;

    json sub_temp;
    sub_temp["url"] = "^https://(?:www\\.)?webex.com/meeting((?:0x|0d)?(?:[0-9ABCDEFabcdef]+|\\d+))"
                      "/user((?:0x|0d)?(?:[0-9ABCDEFabcdef]+|\\d+))$";
    sub_temp["bits"] = {16, 16};
    sub_temp["sub_pen"] = -1;

    temp["templates"].push_back(sub_temp);

    json real;
    real.push_back(temp);

    ASSERT_EQ(temp_encoder.TemplatesToJson(), real);
}

TEST_F(TestUrlEncoder, TemplatesFromJson)
{
    json temp;
    temp["pen"] = 11259374;

    json sub_temp;
    sub_temp["url"] = "^https://(?:www\\.)?webex.com/meeting((?:0x|0d)?(?:[0-9ABCDEFabcdef]+|\\d+))"
                      "/user((?:0x|0d)?(?:[0-9ABCDEFabcdef]+|\\d+))$";
    sub_temp["bits"] = {16, 16};
    sub_temp["sub_pen"] = -1;

    temp["templates"].push_back(sub_temp);

    json j;
    j.push_back(temp);
    encoder.TemplatesFromJson(j);

    auto output = encoder.GetTemplate(11259374).at(-1);

    ASSERT_EQ("^https://(?:www\\.)?webex.com/meeting((?:0x|0d)?(?:[0-9ABCDEFabcdef]+|\\d+))"
              "/user((?:0x|0d)?(?:[0-9ABCDEFabcdef]+|\\d+))$",
              output.url);
    ASSERT_EQ(std::vector<std::uint32_t>({16, 16}), output.bits);
}

TEST_F(TestUrlEncoder, GetTemplate)
{
    encoder.AddTemplate(std::string("https://!{www.}!webex.com<pen=23>"
                                    "/meeting<int16>/user<int16>"));

    auto output_template = encoder.GetTemplate(23).at(-1);

    ASSERT_EQ("^https://(?:www\\.)?webex.com/meeting((?:0x|0d)?(?:[0-9ABCDEFabcdef]+|\\d+))/"
              "user((?:0x|0d)?(?:[0-9ABCDEFabcdef]+|\\d+))$",
              output_template.url);
    ASSERT_EQ(std::vector<std::uint32_t>({16, 16}), output_template.bits);
}

TEST_F(TestUrlEncoder, GetTemplates)
{
    encoder.AddTemplate(std::string("https://!{www.}!webex.com"
                                    "<pen=11259374>/meeting<int16>/user<int16>"));

    auto output = encoder.GetTemplate(11259374).at(-1);

    ASSERT_EQ("^https://(?:www\\.)?webex.com/meeting((?:0x|0d)?(?:[0-9ABCDEFabcdef]+|\\d+))"
              "/user((?:0x|0d)?(?:[0-9ABCDEFabcdef]+|\\d+))$",
              output.url);
    ASSERT_EQ(std::vector<std::uint32_t>({16, 16}), output.bits);
}

TEST_F(TestUrlEncoder, TemplateCount)
{
    ASSERT_EQ(encoder.TemplateCount(false), 3);
    ASSERT_EQ(encoder.TemplateCount(true), 3);

    encoder.AddTemplate(std::string("https://!{www.}!webex.com"
                                    "<pen=11259374>Cookie/meeting<int16>/user<int16>"));

    ASSERT_EQ(encoder.TemplateCount(true), 4);
}

TEST_F(TestUrlEncoder, Clear)
{
    encoder.Clear();

    ASSERT_EQ(encoder.GetTemplates().size(), 0);
}
} // namespace