#include "HttpEncoder.hh"

#include <iostream>

std::string HttpEncoder::EncodeUrl(const std::string &url)
{
    url_details details = ParseUrl(url);

    size_t num_remaining_zeroes = HttpEncoder::NumPrependZeroes(static_cast<uint64_t>(0), 56);
    size_t num_reg_id_zeroes = HttpEncoder::NumPrependZeroes(details.reg_id, sizeof(details.reg_id) * 8);
    size_t num_pen_zeroes = HttpEncoder::NumPrependZeroes(details.pen, 24);
    size_t num_meeting_zeroes = HttpEncoder::NumPrependZeroes(details.meeting, sizeof(details.meeting) * 8);
    size_t num_user_zeroes = HttpEncoder::NumPrependZeroes(details.user, sizeof(details.user) * 8);

    /* 128 bit integer broken down by the following
     * 56 bits are zeroes = 17 significant digits
     * 16 bits register id = 5 significant digits
     * 24 bits pen = 8 significant digits
     * 16 bits meeting
     * 16 bits user
    **/
    std::string res;

    // Of the 128 bits 56 of them are zeroes?
    ZeroPadString(res, num_remaining_zeroes);
    std::cout << "res with extra zeroes " << res.length() << " " << res << std::endl;

    // Since we are calculating the number of prepend zeroes to a value
    // then we need to append a zero to the string to make it all 17 sig figs
    res += std::to_string(0);
    std::cout << "res with extra values " << res.length() << " " << res << std::endl;

    // Pad the num of zeroes the the regex id
    ZeroPadString(res, num_reg_id_zeroes);
    std::cout << "res with regex zeroes " << res.length() << " " << res << std::endl;

    // Place the regex id onto the string
    res += std::to_string(details.reg_id);
    std::cout << "res with regex values " << res.length() << " " << res << std::endl;

    // Pad the num of pen zeroes
    ZeroPadString(res, num_pen_zeroes);
    std::cout << "res with pen zeroes " << res.length() << " " << res << std::endl;

    // Place the pen onto the string
    res += std::to_string(details.pen);
    std::cout << "res with pen value " << res.length() << " " << res << std::endl;

    // Pad the num of meeting zeroes
    ZeroPadString(res, num_meeting_zeroes);
    std::cout << "res with meeting zeroes " << res.length() << " " << res << std::endl;

    // Place the meeting number onto the string
    res += std::to_string(details.meeting);
    std::cout << "res with meeting value " << res.length() << " " << res << std::endl;

    // Pad the num of user zeroes
    ZeroPadString(res, num_user_zeroes);
    std::cout << "res with user zeroes " << res.length() << " " << res << std::endl;

    // Place the user number onto the string
    res += std::to_string(details.user);
    std::cout << "res with user value " << res.length() << " " << res << std::endl;

    return res;
}

std::string HttpEncoder::DecodeUrl(const std::string &url)
{
    // Consts
    const std::uint32_t Extra_Sig_Figs = 17;
    const std::uint32_t Bit16_Sig_Figs = 5;
    const std::uint32_t Bit22_Sig_Figs = 8;

    // Get the template and ignore the first 56 bits and get regex id
    url_details details;

    std::uint32_t sub_start = Extra_Sig_Figs;

    details.reg_id = std::stoi(url.substr(sub_start, Bit16_Sig_Figs));
    sub_start += Bit16_Sig_Figs;
    details.pen = std::stoi(url.substr(sub_start, Bit22_Sig_Figs));
    sub_start += Bit22_Sig_Figs;
    details.meeting = std::stoi(url.substr(sub_start, Bit16_Sig_Figs));
    sub_start += Bit16_Sig_Figs;
    details.user = std::stoi(url.substr(sub_start, Bit16_Sig_Figs));
    std::cout << details.reg_id << std::endl;
    std::cout << details.pen << std::endl;
    std::cout << details.meeting << std::endl;
    std::cout << details.user << std::endl;

    // Scrape out the regex stuff from the template by reading groups
    // replace groups with the appropriate values

    std::string reg = HttpEncoder::regexes.at(details.reg_id);
    std::string decoded;
    std::cout << reg << std::endl;

    std::uint32_t idx = 0;
    std::vector<size_t> group_indices;
    size_t find;
    while (idx < reg.length())
    {
        if (reg[idx] == '[')
        {
            find = reg.find(']', idx);
            if (find != std::string::npos)
            {
                idx = find + 1;
                continue;
            }
        }

        if (reg[idx] == '(')
        {
            find = reg.find(')', idx);
            if (find != std::string::npos)
            {
                group_indices.push_back(decoded.length());
                idx = find + 1;
                continue;
            }
        }

        if (reg[idx] == '?')
        {
            idx++;
            continue;
        }

        decoded += reg[idx++];
    }

    for (int i = 0; i < group_indices.size(); i++)
    {
        std::cout << group_indices[i] << std::endl;
    }
    std::cout << "str len " << decoded.size() << std::endl;

    std::string str_pen = std::to_string(details.pen);
    std::string str_meeting = std::to_string(details.meeting);
    std::string str_user = std::to_string(details.user);

    decoded.insert(group_indices[0], str_pen);
    decoded.insert(group_indices[1] + str_pen.size(), str_meeting);
    decoded.insert(group_indices[2] + str_pen.size() + str_meeting.size(), str_user);

    return decoded;
}

HttpEncoder::url_details HttpEncoder::ParseUrl(const std::string &url)
{
    constexpr size_t Pen_Group = 1;
    constexpr size_t Meeting_Group = 2;
    constexpr size_t User_Group = 3;

    // The details we'll need for the url
    url_details details;

    // To extract the 3 groups from each url format
    std::smatch matches;

    // For error checking and too large of numbers
    uint64_t tmp_pen;
    uint32_t tmp_meeting;
    uint32_t tmp_user;

    bool match = false;

    // Find some sort of template and return it
    for (auto reg : regexes)
    {
        // If this is not a match continue to the next regex
        if (!std::regex_match(url, matches, std::regex(reg.second))) continue;

        tmp_pen = atoi(matches[Pen_Group].str().c_str());
        tmp_meeting = atoi(matches[Meeting_Group].str().c_str());
        tmp_user = atoi(matches[User_Group].str().c_str());

        // Ensure that the value is not greater than an 24 bit int
        if (tmp_pen > 0xFFFFFF)
            throw "PEN ID is outside of the acceptable range";

        if (tmp_meeting > UINT16_MAX)
            throw "Meeting ID is outside of the acceptable range";

        if (tmp_user > UINT16_MAX)
            throw "User ID is outside of the acceptable range";

        // Now that we have checked the values can fill in the details
        details.reg_id = reg.first;
        details.pen = tmp_pen;
        details.meeting = tmp_meeting;
        details.user = tmp_user;

        match = true;

        break;
    }

    if (!match)
        throw "No template found for the given input";

    return details;
}

void HttpEncoder::ZeroPadString(std::string &str, size_t zeroes)
{
    while (zeroes-- > 0)
    {
        str.append("0");
    }
}
