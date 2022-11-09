#include <gtest/gtest.h>
#include "UrlTemplater.hh"
#include "nlohmann/json.hpp"
using json = nlohmann::json;

namespace
{
    class TestUrlTemplater : public ::testing::Test
    {
    protected:
        TestUrlTemplater() = default;
        ~TestUrlTemplater() = default;

        UrlTemplater templater;
    };

    TEST_F(TestUrlTemplater, AddTemplate)
    {
        templater.Add("https://!{www.}!webex.com/<int24=11259375>/meeting<int16>/user<int16>");

        ASSERT_EQ(1, templater.GetTemplates().size());
    }

    TEST_F(TestUrlTemplater, RemoveTemplate)
    {
        templater.Add("https://!{www.}!webex.com/<int24=11259375>/meeting<int16>/user<int16>");
        templater.Remove(11259375);
        ASSERT_EQ(0, templater.GetTemplates().size());
    }

    TEST_F(TestUrlTemplater, GetTemplates)
    {
        templater.Add("https://!{www.}!webex.com/<int24=11259375>/meeting<int16>/user<int16>");

        auto output = *templater.GetTemplates().begin();

        ASSERT_EQ(11259375, output.first);
        ASSERT_EQ("^https://(?:www\\.)?webex.com/(\\d+)/meeting(\\d+)/user(\\d+)$",
            output.second.url);
        ASSERT_EQ(std::vector<std::uint32_t>({24, 16, 16}), output.second.bits);
    }

    TEST_F(TestUrlTemplater, ToJson)
    {
        templater.Add("https://!{www.}!webex.com/<int24=11259375>/meeting<int16>/user<int16>");
        json temp;
        temp["pen"] = 11259375;
        temp["url"] = "^https://(?:www\\.)?webex.com/(\\d+)/meeting(\\d+)/user(\\d+)$";
        temp["bits"] = {24, 16, 16};

        json real;
        real.push_back(temp);

        ASSERT_EQ(templater.ToJson(), real);
    }

    TEST_F(TestUrlTemplater, FromJson)
    {
        json temp;
        temp["pen"] = 11259375;
        temp["url"] = "^https://(?:www\\.)?webex.com/(\\d+)/meeting(\\d+)/user(\\d+)$";
        temp["bits"] = {24, 16, 16};

        json j;
        j.push_back(temp);
        templater.FromJson(j);

        auto output = *templater.GetTemplates().begin();

        ASSERT_EQ(11259375, output.first);
        ASSERT_EQ("^https://(?:www\\.)?webex.com/(\\d+)/meeting(\\d+)/user(\\d+)$",
            output.second.url);
        ASSERT_EQ(std::vector<std::uint32_t>({24, 16, 16}), output.second.bits);
    }

    TEST_F(TestUrlTemplater, Clear)
    {
        templater.Clear();

        ASSERT_EQ(templater.GetTemplates().size(), 0);
    }
}