#include "HttpEncoder.hh"

#include <iostream>

uint128 HttpEncoder::EncodeUrl(const std::string &url)
{

    std::vector<std::uint64_t> numerical_groups;

    // To extract the 3 groups from each url format
    std::smatch matches;

    std::pair<std::uint32_t, url_template> selected_template;

    // Find some sort of template and return it
    for (auto cur_template : templates)
    {
        selected_template = cur_template;

        // If this is not a match continue to the next regex
        if (!std::regex_match(url, matches, std::regex(cur_template.second.url)))
            continue;

        // Skip the first group since its the whole match
        for (std::uint32_t i = 1; i < matches.size(); i++)
        {
            std::uint64_t val = std::stoi(matches[i].str());
            std::uint64_t bits = cur_template.second.bits[i-1];
            std::uint64_t max_val = GetMaxBitValue(bits);

            if (val > max_val)
            {
                throw "Error. Out of range. Group " + std::to_string(i)
                    + " value is " + std::to_string(val)
                    + " but the max value is " + std::to_string(max_val);
            }

            // Keep track of each numerical group
            numerical_groups.push_back(val);
        }

        // Break out after found a regex match
        break;
    }

    uint128 encoded;
    std::uint32_t offset_bits = 0;
    std::uint32_t bits = 0;
    for (std::uint32_t i = 0; i < numerical_groups.size(); ++i)
    {
        // Get the amount of bits in the parallel array in numerical groups
        bits = selected_template.second.bits[i];

        // Set the value in the uint128 variable
        encoded.SetValue(numerical_groups[i], bits, offset_bits);

        // Slide the bit window over
        offset_bits += bits;
    }

    return encoded;
}

std::string HttpEncoder::DecodeUrl(const std::string code_str,
                                   const std::uint32_t bit_format)
{
    if (bit_format > 64)
        throw "bit_format cannot be greater than 64";
    if (128 % bit_format != 0)
        throw "bit_format must be a factor of 128 that is less than 64";

    const std::uint32_t Pen_Bits = 24;

    // Convert the string to the uint128
    uint128 code(code_str, bit_format);

    std::string decoded;

    // Assumed that the first 24 bits is always the PEN
    std::uint64_t pen = code.GetValue(Pen_Bits, 0);

    // Get the template for that PEN
    auto temp = templates[pen];

    // Get the regex
    auto reg = temp.url;

    std::uint32_t idx = 0;
    std::vector<std::uint32_t> group_indices;
    char ch;
    size_t find = 0;
    while (idx < reg.size())
    {
        ch = reg[idx];

        // Find and ignore optional brackets
        if (ch == '[')
        {
            find = reg.find(']', idx);
            if (find != std::string::npos)
            {
                idx = find + 1;
                continue;
            }
        }

        // Find and groups and make note of them
        if (ch == '(')
        {
            find = reg.find(')', idx);
            if (find != std::string::npos)
            {
                group_indices.push_back(decoded.length());
                idx = find + 1;
                continue;
            }
        }

        // Skip over question marks
        if (ch == '?')
        {
            idx++;
            continue;
        }

        decoded += ch;
        ++idx;
    }

    std::uint32_t num_bits;
    std::uint32_t str_offset = 0;
    std::uint32_t insert_idx;
    std::string insert_str;
    std::uint32_t bit_offset = 0;
    for (std::uint32_t i = 0; i < group_indices.size(); i++)
    {
        // Get the number of bits for this number
        num_bits = temp.bits[i];

        // Calc how much we've inserted into the string
        str_offset += insert_str.length();

        // Get the idx with an offset
        insert_idx = group_indices[i] + str_offset;

        // Get the bits converted into decimal
        insert_str = std::to_string(code.GetValue(num_bits, bit_offset));

        // Add the number of bits that was read
        bit_offset += num_bits;

        // Insert the numeric values into the decoded string
        decoded.insert(insert_idx, insert_str);
    }

    return decoded;
}
