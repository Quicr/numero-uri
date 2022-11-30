#include <gtest/gtest.h>
#include "NumericHelper.hh"

namespace
{
    TEST(TestNumericHelper, DoubleStringValue)
    {
        // TODO
        ASSERT_EQ(true, false);
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