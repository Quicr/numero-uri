#include <gtest/gtest.h>
#include <string>

#include "uint128.hh"

namespace
{
    TEST(Testuint128, SetGetNumericValue)
    {
        uint128 value;

        uint16_t in = 0;
        uint32_t bits = 4;

        value.SetValue(in, bits, value.last() - bits);

        uint16_t output = value.GetValue(bits, value.last() - bits);

        ASSERT_EQ(output, in);
    }

    TEST(Testuint128, SetGetStringValue)
    {
        uint128 value;
        std::string str_in = "132133453000";

        value.FromString(str_in);

        std::uint64_t output = value.GetValue(64, 0);

        ASSERT_EQ(output, std::stoull(str_in));
    }

    TEST(Testuint128, ToBinary)
    {
        uint128 value;

        uint64_t in = 486527;
        uint32_t bits = 64;

        value.SetValue(in, bits, 64);

        std::string bin_out = value.ToBinaryString();
        // lol
        std::string actual = "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001110110110001111111";

        ASSERT_EQ(bin_out, actual);
    }

    TEST(Testuint128, ToString)
    {
        uint128 value;

        uint64_t in = 123120534320;
        uint32_t bits = 64;

        value.SetValue(in, bits, 64);

        std::string str_out = value.ToDecimalString();
        std::string actual = "0000000000000000000000000000123120534320";

        ASSERT_EQ(str_out, actual);
    }

    TEST(Testuint128, Clear)
    {
        uint128 value;

        uint64_t first = 123120534320;
        uint64_t second = 1;

        value.SetValue(first, 64, 0);
        value.SetValue(second, 64, 64);

        std::string actual = "0000000012312053432000000000000000000001";
        ASSERT_EQ(value.ToDecimalString(), actual);

        value.Clear();

        actual = "0000000000000000000000000000000000000000";
        ASSERT_EQ(value.ToDecimalString(), actual);
    }
}