#include "UrlEncoder.hh"

#include <regex>

#include "NumericHelper.hh"

#include <iostream>

uint128 UrlEncoder::EncodeUrl(const std::string &url)
{
    // TODO there is a bug here..
    // Originally we just cycled through the templates
    // but now we need to read the PEN value and check if it is
    // in the templates.

    std::vector<std::uint64_t> numerical_groups;

    // To extract the 3 groups from each url format
    std::smatch matches;

    UrlEncoder::url_template selected_template;

    // Get the pen from the URL
    if (!std::regex_search(url, matches, std::regex("(\\d+)")))
    {
        throw UrlEncoderException("Error. No PEN found");
    }

    // Get the PEN from the matches which should be the first number
    std::uint64_t pen = std::stoull(matches[0].str());

    for (auto temp : templates)
    {
        std::cout << temp.second.url << std::endl;
    }

    // Check if the template is in the list of templates
    if (templates.find(pen) == templates.end())
        throw UrlEncoderNoMatchException("Error. No template for PEN " + matches[0].str());

    // Get the template based on the PEN
    selected_template = templates.at(pen);

    // If there is not a match then there is no template for this PEN
    if (!std::regex_match(url, matches, std::regex(selected_template.url)))
        throw UrlEncoderNoMatchException("Error. No match for url: " + url);

    // Need the same number of numbers as the template expects
    if (matches.size() - 1 != selected_template.bits.size())
        throw UrlEncoderNoMatchException("Error. Match is missing values for the given template");

    // Skip the first group since its the whole match
    for (std::uint32_t i = 1; i < matches.size(); i++)
    {
        std::uint64_t val = std::stoull(matches[i].str());
        std::uint32_t bits = selected_template.bits[i-1];
        std::uint64_t max_val = NumericHelper::MaxValue(bits);

        if (val > max_val)
        {
            throw UrlEncoderOutOfRangeException(
                "Error. Out of range. Group " + std::to_string(i)
                + " value is " + std::to_string(val)
                + " but the max value is " + std::to_string(max_val),
                i, val);
        }

        // Keep track of each numerical group
        numerical_groups.push_back(val);
    }

    // TODO move this into the above loop...
    uint128 encoded;
    std::uint32_t offset_bits = 0;
    std::uint32_t bits = 0;
    for (std::uint32_t i = 0; i < numerical_groups.size(); ++i)
    {
        // Get the amount of bits in the parallel array in numerical groups
        bits = selected_template.bits[i];

        // Set the value in the uint128 variable
        encoded.SetValue(numerical_groups[i], bits, offset_bits);

        // Slide the bit window over
        offset_bits += bits;
    }

    return encoded;
}

std::string UrlEncoder::DecodeUrl(const std::string code_str,
                                  const uint128::Representation rep)
{
    const std::uint32_t Pen_Bits = 24;

    // Convert the string to the uint128
    uint128 code(code_str, rep);

    std::string decoded;

    // Assumed that the first 24 bits is always the PEN
    std::uint64_t pen = code.GetValue(Pen_Bits, 0);

    // Check if pen exists in list of templates
    if (templates.find(pen) == templates.end())
        throw UrlDecodeNoMatchException(
            "Error. No templates matches the found PEN " + std::to_string(pen));

    // Get the template for that PEN
    UrlEncoder::url_template temp = templates.at(pen);

    // Get the regex
    std::string reg = temp.url;

    size_t idx = 0;
    std::vector<size_t> group_indices;
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
    size_t str_offset = 0;
    size_t insert_idx;
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

bool UrlEncoder::Add(const std::string new_template)
{
    // The first value must be filled in with their PEN
    const std::string example =
        "http://!{www.}!webex.com/<int24=777>/meeting<int16>/user<int16>";

    // If there is a !{...}! it is an optional group
    const std::regex optional_regex("!\\{(.+)\\}!");

    // Parse the string
    // Build a regex out of it.. good luck brett
    const std::regex bit_group_regex("^u?int([1-9][0-9]?)(=\\d+)?$");
    std::smatch matches;

    // Template variable
    url_template temp;

    // Find the first angle bracket
    std::size_t start = new_template.find('<') + 1;
    std::size_t end = 0;

    // Start the regex match string
    temp.url = '^';

    // Put everything up to the first option into the regex
    temp.url += new_template.substr(0, start-1);

    // Add the regex for a digit
    temp.url += "(\\d+)";

    // Get the pen from the string
    end = new_template.find('>', start);
    std::string group_str = new_template.substr(start, end-start);
    // Get the match the pen to the regex
    if (!std::regex_match(group_str, matches, bit_group_regex))
    {
        std::string msg = "Error. Missing bit definition for PEN.";
        msg += " Example: " + example;
        throw UrlEncoderException(msg);
    }

    // Get the bits
    std::uint32_t pen_bit = std::stoul(matches[1].str());
    if (pen_bit > 64)
    {
        std::string msg = "Error. PEN cannot have more than 64 bits";
        msg += " Example: " + example;
        throw UrlEncoderException(msg);
    }

    // Check if a value was entered
    if (matches.length() < 3)
    {
        std::string msg = "Error. No match found for the value of the PEN";
        msg += " Example: " + example;
        throw UrlEncoderException(msg);
    }

    std::uint64_t pen_value;
    try
    {
        // Get the substr after the = character
        pen_value = std::stoull(matches[2].str().substr(1));
    }
    catch(std::out_of_range &ex)
    {
        std::string msg = "Error. PEN value is too large for a 64 bit number";
        msg += " PEN value found = " + matches[2].str();
        msg += " ";
        msg += ex.what();
        throw UrlEncoderException(msg);
    }

    if (templates.find(pen_value) != templates.end())
    {
        std::string msg = "Error. PEN key " + std::to_string(pen_value) +
                          " is not unique and already exists"
                          " in the templates list.";
        throw UrlEncoderException(msg);
    }

    temp.bits.push_back(pen_bit);

    // Start from the end of the PEN group
    start = end + 1;
    char ch;
    while (start < new_template.length())
    {
        // Get each other group from the string and build out the regex..
        ch = new_template[start++];

        if (ch == '<')
        {
            // Find the end of the group
            end = new_template.find('>', start);

            // Pull out the group
            group_str = new_template.substr(start, end-start);

            // Run it through regex
            if (!std::regex_match(group_str, matches, bit_group_regex))
            {
                // Failed so throw an error
                throw UrlEncoderException(
                    "Error. Could not find the end of group starting at position " +
                    std::to_string(start));
            }

            // Get the bits from the match
            temp.bits.push_back(std::stoul(matches[1].str()));
            start = end + 1;

            // Push regex onto the string
            temp.url += ("(\\d+)");
            continue;
        }

        temp.url += ch;
    }

    // Close the entire string
    temp.url += '$';

    // Extract and replace optional chunks
    std::regex_search(temp.url, matches, optional_regex);
    std::string optional_str;
    size_t search_idx = 1;
    size_t optional_idx;
    size_t optional_sz;
    size_t period_idx;
    while (search_idx < matches.size())
    {
        optional_str = matches[search_idx++].str();
        optional_idx = temp.url.find("!{" + optional_str + "}!");
        optional_sz = optional_str.size() + 4;

        // This could be a problem if there are a lot of wild cards..
        // Replace the period if found
        period_idx = optional_str.find('.');
        if (period_idx != std::string::npos)
            optional_str.replace(optional_str.find('.'), 1, "\\.");

        // Replace the optional chunk !{...}!
        if (optional_idx != std::string::npos)
            temp.url.replace(optional_idx, optional_sz, "(?:" + optional_str + ")?");
    }

    templates[pen_value] = temp;

    return true;
}

bool UrlEncoder::Remove(const std::uint64_t key)
{
    if (templates.find(key) == templates.end())
    {
        return false;
    }

    templates.erase(key);

    return true;
}

json UrlEncoder::ToJson() const
{
    // Create the template string
    json j;
    json j_temp;
    json j_bits;
    for (auto temp : templates)
    {
        j_temp.clear();
        j_temp["pen"] = temp.first;
        j_temp["url"] = temp.second.url;

        // Fill in the bits array
        j_bits.clear();
        for (auto bit : temp.second.bits)
            j_bits.push_back(bit);
        j_temp["bits"] = j_bits;

        j.push_back(j_temp);
    }

    return j;
}

bool UrlEncoder::FromJson(const json& data)
{
    for (unsigned int i = 0; i < data.size(); i++)
    {
        url_template temp;

        // Get the url
        temp.url = data[i]["url"];

        // Get the bits
        for (auto& element : data[i]["bits"])
            temp.bits.push_back(static_cast<std::uint32_t>(element));

        // Push the values onto the templates list
        templates[static_cast<std::uint64_t>(data[i]["pen"])] = temp;
    }

    return true;
}

void UrlEncoder::Clear()
{
    templates.clear();
}

const UrlEncoder::template_list& UrlEncoder::GetTemplates() const
{
    return templates;
}

const UrlEncoder::url_template&
    UrlEncoder::GetTemplate(std::uint64_t pen) const
{
    return templates.at(pen);
}

