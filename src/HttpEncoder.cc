#include "HttpEncoder.hh"

#include <fstream>

#include "NumericHelper.hh"

uint128 HttpEncoder::EncodeUrl(const std::string &url,
                               const UrlTemplater::template_list& template_list)
{
    // TODO there is a bug here..
    // Originally we just cycled through the templates
    // but now we need to read the PEN value and check if it is
    // in the templates.

    std::vector<std::uint64_t> numerical_groups;

    // To extract the 3 groups from each url format
    std::smatch matches;

    std::pair<std::uint32_t, UrlTemplater::url_template> selected_template;
    bool matched = false;

    // Find some sort of template and return it
    for (auto cur_template : template_list)
    {
        selected_template = cur_template;

        // If this is not a match continue to the next regex
        if (!std::regex_match(url, matches, std::regex(cur_template.second.url)))
            continue;

        matched = true;
        // Skip the first group since its the whole match
        for (std::uint32_t i = 1; i < matches.size(); i++)
        {
            std::uint64_t val = std::stoi(matches[i].str());
            std::uint64_t bits = cur_template.second.bits[i-1];
            std::uint64_t max_val = NumericHelper::MaxValue(bits);

            if (val > max_val)
            {
                throw HttpEncodeOutOfRangeException(
                    "Error. Out of range. Group " + std::to_string(i)
                    + " value is " + std::to_string(val)
                    + " but the max value is " + std::to_string(max_val),
                    i, val);
            }

            // Keep track of each numerical group
            numerical_groups.push_back(val);
        }

        // Break out after found a regex match
        break;
    }

    if (!matched)
        throw HttpEncodeNoMatchException("No match for url: " + url);

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
    const UrlTemplater::template_list& template_list,
    const std::uint32_t bit_format)
{
    const std::uint32_t Pen_Bits = 24;

    // Convert the string to the uint128
    uint128 code(code_str, bit_format);

    std::string decoded;

    // Assumed that the first 24 bits is always the PEN
    std::uint64_t pen = code.GetValue(Pen_Bits, 0);

    // Check if pen exists in list of templates
    if (template_list.find(pen) == template_list.end())
        throw HttpDecodeNoMatchException(
            "Error. No templates matches the found PEN " + std::to_string(pen));

    // Get the template for that PEN
    UrlTemplater::url_template temp = template_list.at(pen);

    // Get the regex
    std::string reg = temp.url;

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
            // Check the next couple characters for non matching-groups
            if (idx + 3 < reg.size() && reg[idx+1] == '?' && reg[idx+2] == ':')
            {
                idx += 3;
                continue;
            }

            find = reg.find(')', idx);
            if (find != std::string::npos)
            {
                group_indices.push_back(decoded.length());
                idx = find + 1;
                continue;
            }
        }

        // Skip over ?, ^, $, \, )
        if (ch == '?' || ch == '^' || ch == '$' || ch == '\\' || ch == ')')
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
