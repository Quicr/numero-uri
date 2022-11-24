#ifndef NUMERICHELPER_H
#define NUMERICHELPER_H

#include <string>
namespace NumericHelper
{
    static std::uint64_t MaxValue(std::uint32_t bits)
    {
        std::uint64_t max_val = 1;
        while (bits-- > 1)
        {
            max_val = max_val << 1;
            max_val += 1;
        }

        return max_val;
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