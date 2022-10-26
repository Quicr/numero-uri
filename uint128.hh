#pragma once

#include <string>
#include <vector>

#define MAX_IDX 128

class uint128_t
{
public:
    uint128_t()
    {
        for (uint16_t i = 0; i < 128; ++i)
        {
            digits.push_back(0);
        }
    }

    void SetValue(std::uint64_t value,
                  std::uint16_t num_bits,
                  std::uint16_t offset)
    {
        // TODO there is something wrong and the digit is not being recorded
        // into the vector. Just use cpp.sh to figure it out.
        if (num_bits > 64) throw "Max num bits is 64";

        std::uint64_t final_bit = 1;
        std::uint16_t sz = num_bits;
        while (sz-- > 1)
            final_bit = final_bit << 1;

        std::uint16_t shift_out = 64 - num_bits;
        value = value << shift_out;

        std::uint16_t idx = MAX_IDX - offset;
        std::uint16_t count = 0;
        while (count < num_bits && idx < MAX_IDX)
        {
            digits[idx--] = ((value & final_bit) == 1);
            value = value << 1;
            count++;
        }
    }

private:
    std::vector<std::uint8_t> digits;
};