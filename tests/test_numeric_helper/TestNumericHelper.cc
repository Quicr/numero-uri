#include <gtest/gtest.h>
#include "NumericHelper.hh"

namespace
{
    TEST(TestNumericHelper, MaxValue)
    {
        // Test for the max value based on bits
        std::uint64_t output = NumericHelper::MaxValue(4);
        ASSERT_EQ(15, output);

        output = NumericHelper::MaxValue(5);
        ASSERT_EQ(31, output);

        output = NumericHelper::MaxValue(16);
        ASSERT_EQ(UINT16_MAX, output);

        output = NumericHelper::MaxValue(32);
        ASSERT_EQ(UINT32_MAX, output);

        output = NumericHelper::MaxValue(64);
        ASSERT_EQ(UINT64_MAX, output);
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