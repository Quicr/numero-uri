#include <string>
#include <vector>
#include <map>
#include <regex>

#include <iostream>


class HttpEncoder
{
public:
    static std::string EncodeUrl(const std::string &url);
    static std::string DecodeUrl(const std::string &url);
private:
    typedef struct {
        std::uint16_t reg_id;
        std::uint32_t pen;
        std::uint16_t meeting;
        std::uint16_t user;
    } url_details;

    static url_details ParseUrl(const std::string &url);
    static void ZeroPadString(std::string &str, size_t zeroes);

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

    static const inline std::map<const std::uint16_t, const std::string> regexes = {
        { 0, std::string("https://[www.]?webex.com/(\\d+)/meeting(\\d+)/user(\\d+)") }
    };
};