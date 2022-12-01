#pragma once

#include <string>
#include <vector>

#include "NumericHelper.hh"

#define BIT_64 64

class big_uint_exception : public std::runtime_error
{
public:
    explicit big_uint_exception(const std::string &what_arg) :
        std::runtime_error(what_arg)
    {
    }

    explicit big_uint_exception(const char *what_arg) :
        std::runtime_error(what_arg)
    {
    }
};

class big_uint
{
public:
    enum Representation
    {
        sym, // Implies string contains format (0x, 0b, 0d).
        hex,
        bin,
        dec
    };

    big_uint(const std::uint64_t val_in=0,
             const std::uint32_t num_bits=128)
    {
        // Put in lower bits
        InitDigits(num_bits);
        SetValue(val_in, 64, 64);
    }

    big_uint(const std::string str_in,
             const Representation rep,
             const std::uint32_t num_bits=128)
    {
        InitDigits(num_bits);

        FromString(str_in, rep);
    }

    big_uint& operator=(const std::uint64_t val_in)
    {
        InitDigits(128);
        this->SetValue(val_in, 64, 64);
        return *this;
    }

    void SetValue(std::uint64_t value,
                  const std::uint32_t num_bits,
                  const std::uint16_t msb_offset)
    {
        if (num_bits > BIT_64)
            throw big_uint_exception("Error. Max num bits is 64");
        if (msb_offset > digits.size() - 1)
            throw big_uint_exception("Error. msb offset is out of range. Max " +
                std::to_string(digits.size() - 1));

        const std::uint64_t bit_mask = 0x8000000000000000;

        std::uint32_t shift_out = BIT_64 - num_bits;
        value = value << shift_out;

        std::int32_t idx = msb_offset;
        std::uint16_t iter = 0;
        while (iter < num_bits && idx < 128)
        {
            digits[idx++] = ((value & bit_mask) == bit_mask);
            value = value << 1;
            ++iter;
        }
    }

    void SetValue(const big_uint& value,
                  const std::uint16_t num_bits,
                  const std::uint16_t msb_offset)
    {
        if (msb_offset > digits.size() - 1)
            throw big_uint_exception("Error. msb offset is out of range. "
                "Max 127.");

        if (msb_offset + num_bits > digits.size() - 1)
            throw big_uint_exception("Error. Copied data is larger than "
                "Max size of 127.");

        std::uint16_t to_idx = msb_offset;
        std::uint16_t from_idx = value.digits.size() - num_bits;

        while (to_idx < digits.size() - 1 && from_idx < value.digits.size())
        {
            digits[to_idx] = value.at(from_idx);
            to_idx++;
            from_idx++;
        }
    }

    std::uint64_t GetValue(const std::uint32_t num_bits,
                           const std::uint16_t msb_offset) const
    {
        if (num_bits > BIT_64)
            throw big_uint_exception("Max num bits is 64");
        if (msb_offset > 128 - 1)
            throw big_uint_exception("msb_offset is out of range. Max 127");

        std::uint64_t val = 0;
        std::uint64_t idx = msb_offset;
        std::uint64_t iter = 0;
        while (iter < num_bits && idx < 128)
        {
            val = val << 1;
            val += digits[idx++];
            ++iter;
        }

        return val;
    }

    void FromString(const std::string &str_in, const Representation rep)
    {
        switch (rep)
        {
            case sym:
                FromString(str_in);
                break;
            case hex:
                FromHexString(str_in);
                break;
            case bin:
                FromBinaryString(str_in);
                break;
            case dec:
                FromDecimalString(str_in);
                break;
            default:
                throw big_uint_exception("Error. Given representation is not "
                "recognized");
        }
    }

    void FromString(std::string str_in)
    {
        if (str_in.length() < 3)
            throw big_uint_exception("Error. The length of the input string must"
            " contain a number system symbol and values. With a min length of 3"
            ". ex. 0xFA231");

        Clear();

        // Check the first two symbols for 0x, 0b, 0d
        // Strip out the symbols
        char sym[] = { str_in[0], str_in[1], '\0' };

        str_in = str_in.substr(2);

        if (strcmp(sym, "0x") == 0)
            FromHexString(str_in);
        else if (strcmp(sym, "0b") == 0)
            FromBinaryString(str_in);
        else if (strcmp(sym, "0d") == 0)
            FromDecimalString(str_in);
        else
            throw big_uint_exception("Error. No number symbol format given."
            " ex. 0x132 | 0b1101 | 0d1023");
    }

    void FromHexString(std::string hex_in)
    {
        if (hex_in.length() * 4 > digits.size())
            throw big_uint_exception("Error. Hex string contains more significant"
                " digits than the size of the digit array");

        Clear();

        std::int64_t digit_idx = digits.size() - 1;
        char hex_ch;

        // Start from the end
        for (std::int64_t i = hex_in.size() - 1; i >= 0; --i)
        {
            // Look at each letter
            hex_ch = hex_in[i];
            if (hex_ch >= '0' && hex_ch <= '9')
                hex_ch -= '0';
            else if (hex_ch >= 'a' && hex_ch <= 'f')
                hex_ch -= 87;
            else if (hex_ch >= 'A' && hex_ch <= 'F')
                hex_ch -= 55;
            else
                throw big_uint_exception(std::string("Error. Letter ") + hex_ch +
                    std::string(" is not a hexadecimal value."));

            // Convert 0-15 to bin
            uint16_t iter = 0;
            while (hex_ch > 0 && digit_idx >= 0)
            {
                int16_t res = hex_ch % 2;

                digits[digit_idx--] = hex_ch % 2;
                hex_ch /= 2;
                iter++;
            }

            digit_idx = digit_idx - (4 - iter);
        }
    }

    void FromBinaryString(std::string bin_in)
    {
        if (bin_in.length() > digits.size())
            throw big_uint_exception("Error. Binary string contains more "
            "significant digits than the size of the digit array.");

        Clear();

        // Assume big endian
        std::int64_t digits_idx = digits.size() - 1;
        std::int64_t bin_in_idx = bin_in.length() - 1;
        while (digits_idx >= 0 && bin_in_idx >= 0)
        {
            digits[digits_idx--] = bin_in[bin_in_idx--] - '0';
        }
    }

    // Expects raw string with no delimiters
    void FromDecimalString(std::string str_in)
    {
        // Clear the bits in case this is happening not as an init
        Clear();

        const std::uint64_t divisor = 2;
        size_t back = str_in.length() - 1;
        size_t front = 0;
        std::uint64_t dividend;
        std::uint64_t div_res;
        std::string tmp_bin;
        std::string tmp_dec;
        bool first_non_zero = false;

        while (str_in.length() > 0)
        {
            first_non_zero = false;
            back = str_in.length() - 1;
            front = 0;
            dividend = str_in[front] - '0';

            // If the value at the end is even then push a zero
            // if odd push a 1
            tmp_bin.push_back((str_in[back] % divisor));

            // Proceed through the string until the last number
            while (front++ <= back)
            {
                // Divide the number
                div_res = dividend / divisor;

                // If div is non zero set the first non zero flag
                if (div_res)
                    first_non_zero = true;

                // Push in values if there was a non-zero
                if (first_non_zero)
                    tmp_dec.push_back(div_res + '0');

                // Needs to be in because we need at least one last
                // iteration and I want to protect overflow and invalid
                // memory accessing.
                if (front > back) continue;

                // Long division math
                dividend = dividend - (div_res * divisor);
                dividend = (dividend * 10) + (str_in[front] - '0');
            }

            str_in = tmp_dec;
            tmp_dec.clear();
        }

        if (tmp_bin.length() > digits.size())
            throw big_uint_exception("Error. The value of the given string"
                                   " is larger than set size of " +
                                   std::to_string(digits.size()) + " bits");

        // Copy it into the digits.
        std::int64_t digit_idx = digits.size() - 1;
        std::int64_t conv_idx = 0;
        while (digit_idx >= 0 && conv_idx < tmp_bin.size())
        {
            digits[digit_idx] = tmp_bin[conv_idx];
            digit_idx--;
            conv_idx++;
        }
    }

    std::string ToString(const Representation rep,
                         const char delimiter='\0')
    {
        switch (rep)
        {
            case hex:
                return ToHexString(delimiter);
            case bin:
                return ToBinaryString(delimiter);
            case dec:
                return ToDecimalString(delimiter);
            default:
                throw big_uint_exception("Error. Given representation is not "
                "recognized");
        }

        return "";
    }

    std::string ToBinaryString(const char delimiter='\0') const
    {
        std::string str = "0b";

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

    std::string ToHexString(const char delimiter='\0',
                            const char prepend_zeros=true) const
    {
        std::string str = "0x";
        bool first_non_zero = false;
        std::uint32_t i = 0;
        std::uint8_t val = digits[i];
        while (i++ < digits.size())
        {

            // Every 4 iterations push the hex to the string
            // At zero and 4 iter, set the value
            if (i % 4 == 0 && i != 0)
            {
                if (val > 0)
                {
                    first_non_zero = true;
                }

                if ((i != 0 && first_non_zero) || prepend_zeros)
                {
                    // Push the hex character on the string
                    if (val >= 0 && val <= 9)
                        str.push_back(static_cast<char>(val) + '0');
                    else
                        str.push_back(static_cast<char>(val-10) + 'A');

                }

                val = 0;
            }

            // Add a delimiter
            if (delimiter != '\0' && i != 0 && i % 8 == 0)
                str.push_back(delimiter);

            val = val << 1;
            val += digits[i];
        }

        return str;
    }

    std::string ToDecimalString(const char delimiter='\0',
                                const bool prepend_zeroes=true) const
    {
        std::string str;

        std::uint8_t overflow = 0;
        bool first_non_zero = false;
        char digit;
        // Time for a slow conversion
        for (size_t i = 0; i < digits.size(); ++i)
        {
            // This is going to be O(n^2)
            std::uint8_t bin = digits[i];

            if (!first_non_zero && bin) first_non_zero = true;

            if (!first_non_zero) continue;

            for (std::int64_t j = str.length() - 1; j >= 0; --j)
            {
                // double each value in the string
                digit = (str[j] - '0') * 2;
                str[j] = ((digit % 10) + overflow) + '0';
                overflow = digit / 10;
            }

            if (overflow)
            {
                str.insert(0, "1");
                overflow = 0;
            }

            if (bin && str.length() == 0)
            {
                str.push_back('1');
            }
            else if (bin)
            {
                str[str.length() - 1] = str[str.length() - 1] + 1;
            }
        }

        str.insert(0, "0d");
        return str;
    }

    std::uint16_t last()
    {
        return 128 - 1;
    }

    void Clear()
    {
        for (std::uint32_t i = 0; i < digits.size(); ++i)
        {
            digits[i] = 0;
        }
    }

    const std::vector<uint8_t>& Data() const
    {
        return digits;
    }

    const bool operator>(const big_uint& other) const
    {
        const std::vector<uint8_t> other_digits = other.Data();
        for (std::uint32_t i = 0; i < digits.size(); i++)
        {
            if (digits[i] > other_digits[i])
                return true;
            else if (digits[i] < other_digits[i])
                return false;
        }

        return false;
    }

    std::uint8_t& operator[](const std::uint16_t idx)
    {
        if (idx > digits.size())
            throw big_uint_exception("Error. Index out of range");

        return digits[idx];
    }

    const std::uint8_t at(const std::uint16_t idx) const
    {
        if (idx > digits.size())
            throw big_uint_exception("Error. Index out of range");

        return digits[idx];
    }

    std::uint64_t value()
    {
        if (digits.size() > 64)
            return 0;

        return GetValue(digits.size(), 0);
    }

    static big_uint BitValue(std::uint32_t bits)
    {
        big_uint val;

        std::uint64_t i = val.digits.size() - 1;
        while (bits-- > 0)
        {
            val.digits[i--] = 1;
        }

        return val;
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

    void InitDigits(std::uint32_t num_bits)
    {
        if (digits.size() > 0)
            digits.clear();

        for (std::uint32_t i = 0; i < num_bits; i++)
        {
            digits.push_back(0);
        }
    }

    std::vector<std::uint8_t> digits;
};