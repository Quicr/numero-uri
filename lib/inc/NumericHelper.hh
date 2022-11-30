#ifndef NUMERICHELPER_H
#define NUMERICHELPER_H

#include <string>
#include <exception>
namespace NumericHelper
{
    static void DoubleStringNumber(std::string &str_val)
    {
        std::uint8_t overflow = 0;
        char digit;
        for (std::int64_t i = str_val.length() - 1; i >= 0; --i)
        {
            digit = str_val[i];
            if (digit < '0' || digit > '9')
                throw std::out_of_range("Error. String contains non-digits");

            digit = (digit - '0') * 2;
            str_val[i] = ((digit % 10) + overflow) + '0';
            overflow = digit / 10;
        }

        if (overflow)
            str_val.insert(0, "1");
    }

    static std::uint64_t SigFigs(std::uint64_t val)
    {
        std::uint64_t sig_figs = 0;
        while (val > 0)
        {
            val /= 10;
            sig_figs++;
        }

        return sig_figs;
    }
}

#endif