#pragma once

#include <string>
#include <vector>

#define MAX_SIZE 128

class uint128
{
public:
    uint128()
    {
        for (uint16_t i = 0; i < MAX_SIZE; ++i)
        {
            digits.push_back(0);
        }
    }

    // TODO remove?
    uint128(std::uint64_t value,
              std::uint16_t num_bits,
              std::uint16_t msb_offset)
    {
        for (uint16_t i = 0; i < MAX_SIZE; ++i)
        {
            digits.push_back(0);
        }

        SetValue(value, num_bits, msb_offset);
    }

    // TODO template this?
    void SetValue(std::uint64_t value,
                  const std::uint16_t num_bits,
                  const std::uint16_t msb_offset)
    {
        const std::uint64_t bit_mask = 0x8000000000000000;

        if (num_bits > 64) throw "Max num bits is 64";
        if (msb_offset > 127) throw "msb_offset is out of range. Max 127";

        std::uint32_t shift_out = 64 - num_bits;
        value = value << shift_out;


        std::int32_t idx = msb_offset;
        std::uint16_t iter = 0;
        while (iter < num_bits && idx < MAX_SIZE)
        {
            digits[idx++] = ((value & bit_mask) == bit_mask);
            value = value << 1;
            ++iter;
        }
    }

    std::uint64_t GetValue(const std::uint16_t num_bits,
                           const std::uint16_t msb_offset) const
    {
        if (num_bits > 64) throw "Max num bits is 64";
        if (msb_offset > 127) throw "msb_offset is out of range. Max 127";

        std::uint64_t val = 0;
        std::uint64_t idx = msb_offset;
        std::uint64_t iter = 0;
        while (iter < num_bits && idx < MAX_SIZE)
        {
            val = val << 1;
            val += digits[idx++];
            ++iter;
        }

        return val;
    }

    std::string ToString(char delimiter='\0') const
    {
        std::string str;

        for (std::uint32_t i = 0; i < digits.size(); ++i)
        {
            if (delimiter != '\0' && i != 0 && i % 4 == 0)
            {
                str.push_back(' ');
            }

            str.push_back((digits[i]) + '0');
        }

        return str;
    }

    void Clear()
    {
        for (std::uint32_t i = 0; i < digits.size(); ++i)
        {
            digits[i] = 0;
        }
    }

private:
    std::vector<std::uint8_t> digits;
};