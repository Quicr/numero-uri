#include <gtest/gtest.h>
#include "NumericHelper.hh"

namespace
{
    TEST(TestNumericHelper, DoubleStringValue)
    {
        std::string num = "5957";
        NumericHelper::DoubleStringNumber(num);

        ASSERT_EQ("11914", num);

        num = "10000";
        NumericHelper::DoubleStringNumber(num);

        ASSERT_EQ("20000", num);

        num = "65535";
        NumericHelper::DoubleStringNumber(num);

        ASSERT_EQ("131070", num);

        num = "15";
        NumericHelper::DoubleStringNumber(num);

        ASSERT_EQ("30", num);
    }

    TEST(TestNumericHelper, SigFigs)
    {
        std::uint64_t output;

        output = NumericHelper::SigFigs(10);
        ASSERT_EQ(2, output);

        output = NumericHelper::SigFigs(1000);
        ASSERT_EQ(4, output);

        output = NumericHelper::SigFigs(123143);
        ASSERT_EQ(6, output);

        output = NumericHelper::SigFigs(65535);
        ASSERT_EQ(5, output);

        output = NumericHelper::SigFigs(4294967295);
        ASSERT_EQ(10, output);

        output = NumericHelper::SigFigs(UINT64_MAX);
        ASSERT_EQ(20, output);
    }
}