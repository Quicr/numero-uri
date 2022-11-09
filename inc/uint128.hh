#pragma once

#include <string>
#include <vector>

#include "NumericHelper.hh"

#define MAX_SIZE 128
#define BIT_64 64

class uint128Exception : public std::runtime_error
{
public:
    explicit uint128Exception(const std::string &what_arg) :
        std::runtime_error(what_arg)
    {
    }

    explicit uint128Exception(const char *what_arg) :
        std::runtime_error(what_arg)
    {
    }
};

class uint128
{
public:
    uint128()
    {
        InitDigits();
    }

    uint128(std::string str_in, std::uint32_t bit_format=BIT_64)
    {
        InitDigits();

        FromString(str_in, bit_format);
    }

    void SetValue(std::uint64_t value,
                  const std::uint32_t num_bits,
                  const std::uint16_t msb_offset)
    {
        if (num_bits > BIT_64)
            throw uint128Exception("Max num bits is 64");
        if (msb_offset > MAX_SIZE - 1)
            throw uint128Exception("msb_offset is out of range. Max 127");

        const std::uint64_t bit_mask = 0x8000000000000000;

        std::uint32_t shift_out = BIT_64 - num_bits;
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

    std::uint64_t GetValue(const std::uint32_t num_bits,
                           const std::uint16_t msb_offset) const
    {
        if (num_bits > BIT_64)
            throw uint128Exception("Max num bits is 64");
        if (msb_offset > MAX_SIZE - 1)
            throw uint128Exception("msb_offset is out of range. Max 127");

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

    // Expects raw string with no delimiters
    void FromString(const std::string str_in,
                    const std::uint32_t bit_format=BIT_64)
    {
        if (bit_format > BIT_64)
            throw uint128Exception("bit_format cannot be greater than 64");
        if (MAX_SIZE % bit_format != 0)
            throw uint128Exception(
                "bit_format must be a factor of 128 that is less than 64");

        // Clear the bits in case this is happening not as an init
        Clear();

        // Get the sig figs for the bit format
        std::uint64_t sig_figs = NumericHelper::SigFigs(
            NumericHelper::MaxValue(bit_format));

        std::uint32_t groupings = (MAX_SIZE/bit_format);
        std::uint32_t group = 0;

        std::uint64_t val;
        std::uint64_t str_offset = 0;
        std::uint32_t idx;
        std::uint32_t lower_bound = 0;

        bool end_of_string = false;
        while (group < groupings && !end_of_string)
        {
            if (str_offset + sig_figs > str_in.length() && str_in.length() <= 64)
            {
                val = std::stoull(str_in.substr(0));
                end_of_string = true;
            }
            else
            {
                // Get the value from the string of sig_fig digits
                val = std::stoull(str_in.substr(str_offset, sig_figs));
                str_offset += sig_figs;
            }

            // Push the value into the vector
            idx = ((bit_format) + (bit_format * group));
            while (idx-- != lower_bound)
            {
                digits[idx] = ((val & 0x1) == 0x1);
                val = val >> 1;
            }
            lower_bound += bit_format;

            group++;
        }
    }

    std::string ToBinaryString(const char delimiter='\0') const
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

    std::string ToDecimalString(const char delimiter='\0',
                                const std::uint32_t bit_format=BIT_64) const
    {
        if (bit_format > BIT_64)
            throw uint128Exception("bit_format cannot be greater than 64");
        if (MAX_SIZE % bit_format != 0)
            throw uint128Exception(
                "bit_format must be a factor of 128 that is less than 64");

        std::string str;

        std::uint32_t num_binary_groups = MAX_SIZE / bit_format;
        std::uint32_t group = 0;

        std::uint64_t value;
        std::uint32_t iter;
        std::uint32_t idx = 0;
        std::uint64_t add_zeroes = 0;
        while (group++ < num_binary_groups)
        {
            value = 0;
            iter = 0;
            while (iter < bit_format && idx < MAX_SIZE)
            {
                value = value << 1;
                value += digits[idx++];
                ++iter;
            }

            add_zeroes = NumPrependZeroes(value, bit_format);

            while (add_zeroes-- != 0)
            {
                str += '0';
            }

            str += std::to_string(value);
        }
        return str;
    }

    std::uint16_t last()
    {
        return MAX_SIZE - 1;
    }

    void Clear()
    {
        for (std::uint32_t i = 0; i < digits.size(); ++i)
        {
            digits[i] = 0;
        }
    }

private:
    std::uint64_t NumPrependZeroes(std::uint64_t value,
                                   const std::uint64_t num_bits) const
    {
        // If the value is a zero make it 1 for the
        // number of prepend zeroes that would precede the value
        // Even though its zero
        if (value == 0) value = 1;

        // Get the max number for this type
        std::uint64_t max_num = 1;
        std::uint64_t size = num_bits;
        while (size-- > 1)
        {
            max_num = max_num << 1;
            max_num += 1;
        }

        // Now that we know the max value we can see how many sigfigs it has
        std::uint64_t sig_figs = 0;
        while (max_num > 0)
        {
            max_num /= 10;
            sig_figs++;
        }

        // Now that we know the sig figs we can count down how
        // many zeroes we need
        while (value > 0 && sig_figs > 0)
        {
            value /= 10;
            sig_figs--;
        }

        return sig_figs;
    }

    void InitDigits()
    {
        for (std::uint32_t i = 0; i < MAX_SIZE; i++)
        {
            digits.push_back(0);
        }
    }


    std::vector<std::uint8_t> digits;
};