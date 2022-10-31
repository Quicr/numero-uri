#pragma once

#include <string>
#include <vector>
#include <map>
#include <regex>

#include "uint128.hh"

class HttpEncoder
{
public:
    static uint128 EncodeUrl(const std::string &url);
    static std::string DecodeUrl(const std::string code_str,
                                 const std::uint32_t bit_format=64);
private:
    static void ZeroPadString(std::string &str, size_t zeroes);

    // TODO move into a bit helper
    static std::uint64_t GetMaxBitValue(uint64_t bits)
    {
        std::uint64_t max_value = 1;

        while (bits-- > 1)
        {
            max_value = max_value << 1;
            max_value += 1;
        }

        return max_value;
    }

    /* Template function that only takes integer datatypes to find the
     * number of zeroes in front depending on the msb to check from
     * -- value - The number you are looking for zeroes in front of
     * -- msg   - The bits that will be compared to. ex. 1000 0000
     */
    template <typename T,
              typename = typename std::enable_if_t<std::is_integral<T>::value>>
    static size_t GetBinaryZeroPadding(T value, const uint64_t msb)
    {
        if (value < 1)
            value += 1;
        size_t num_zeroes = 0;
        while ((value & msb) == 0)
        {
            num_zeroes += 1;
            value = value << 1;
        }

        return num_zeroes;
    }
    template <typename T,
              typename = typename std::enable_if_t<std::is_integral<T>::value>>
    static std::uint64_t NumPrependZeroes(T value, const std::uint64_t num_bits)
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

    typedef struct {
        std::string url;
        std::vector<std::uint32_t> bits;
    } url_template;

    // TODO load these in from some sort of file?
    static inline std::map<std::uint32_t, const url_template> templates = {
        { 11259375,
            {
                "https://[www.]?webex.com/(\\d+)/meeting(\\d+)/user(\\d+)",
                { 24, 16, 16 }
            }
        },
        { 1,
            {
                "https://[www.]?webex.com/(\\d+)/(\\d+)/meeting(\\d+)/user(\\d+)",
                { 24, 16, 16, 16 }
            }
        }
    };
};