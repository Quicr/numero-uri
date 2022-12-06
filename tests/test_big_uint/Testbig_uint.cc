#include <gtest/gtest.h>
#include <string>

#include "big_uint.hh"

namespace
{
    TEST(Testbig_uint, SetGetNumericValue)
    {
        big_uint value;

        uint64_t in = 0;
        uint32_t bits = 4;

        value.SetValue(in, bits, value.last() - static_cast<uint16_t>(bits));

        uint64_t output = value.GetValue(bits, value.last() - bits);

        ASSERT_EQ(output, in);
    }

    TEST(Testbig_uint, SetGetStringValue)
    {
        // TODO fix.
        big_uint value;
        std::string str_in = "0d132133453000";

        value.FromDecimalString(str_in);

        std::uint64_t output = value.GetValue(64, 0);

        ASSERT_EQ(output, std::stoull(str_in));
    }

    TEST(Testbig_uint, ToBinaryString)
    {
        big_uint value;

        uint64_t in = 486527;
        uint32_t bits = 64;

        value.SetValue(in, bits, 64);

        std::string bin_out = value.ToBinaryString();
        // lol
        std::string actual = "0b00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001110110110001111111";

        ASSERT_EQ(bin_out, actual);
    }

    TEST(Testbig_uint, ToDecimalString)
    {
        big_uint value;

        uint64_t in = 123120534320;
        uint32_t bits = 64;

        value.SetValue(in, bits, 64);

        std::string str_out = value.ToDecimalString();
        std::string actual = "0d000000000000000000000000000123120534320";

        ASSERT_EQ(str_out, actual);
    }

    TEST(Testbig_uint, Clear)
    {
        big_uint value;

        uint64_t first = 123120534320;
        uint64_t second = 1;

        value.SetValue(first, 64, 0);
        value.SetValue(second, 64, 64);

        std::string actual = "0d000000002271172986819413459449539461121";
        ASSERT_EQ(value.ToDecimalString(), actual);

        value.Clear();

        actual = "0d000000000000000000000000000000000000000";
        ASSERT_EQ(value.ToDecimalString(), actual);
    }
}