#include <UrlEncoder.h>
#include <quicr/hex_endec.h>

#include <iostream>
#include <regex>

UrlEncoder::UrlEncoder() : templates()
{
}

UrlEncoder::UrlEncoder(const std::string& init_template) : templates()
{
    AddTemplate(init_template);
}

UrlEncoder::UrlEncoder(const std::vector<std::string>& init_templates) : templates()
{
    AddTemplate(init_templates);
}

UrlEncoder::UrlEncoder(const std::string* init_templates, const size_t count) : templates()
{
    AddTemplate(init_templates, count);
}

UrlEncoder::UrlEncoder(const json& init_templates) : templates()
{
    AddTemplate(init_templates);
}

quicr::Name UrlEncoder::EncodeUrl(const std::string& url) const
{
    std::uint64_t found_pen;
    std::int16_t sub_pen;
    UrlEncoder::url_template selected_template;
    bool found_template = false;
    for (const auto& [pen, sub_templates] : templates)
    {
        // Find match
        const auto& found = std::find_if(sub_templates.begin(), sub_templates.end(), [&](const auto& temp) {
            return std::regex_match(url, std::regex(temp.second.url));
        });

        if (found == sub_templates.end())
            continue;

        found_pen = pen;
        std::tie(sub_pen, selected_template) = *found;
        found_template = true;
        break;
    }

    if (!found_template)
        throw UrlEncoderNoMatchException("Error. No match found for given url: " + url);

    // To extract the 3 groups from each url format
    std::smatch matches;

    // If there is not a match then there is no template for this PEN
    if (!std::regex_match(url, matches, std::regex(selected_template.url)))
        throw UrlEncoderNoMatchException("Error. No match for url: " + url);

    // Need the same number of numbers as the template expects
    if (matches.size() - 1 != selected_template.bits.size())
        throw UrlEncoderNoMatchException("Error. Match is missing values for "
                                         "the given template");

    std::vector<uint64_t> values;
    std::vector<uint8_t> distribution;
    values.push_back(found_pen);
    distribution.push_back(Pen_Bits);

    // Set the sub PEN value and bits if it is positive
    if (sub_pen >= 0)
    {
        values.push_back(sub_pen);
        distribution.push_back(Sub_Pen_Bits);
    }

    // Skip the first group since its the whole match
    std::string_view match;
    for (std::uint32_t i = 1; i < matches.size(); i++)
    {
        match = matches[i].str();
        std::uint16_t base = match.starts_with("0x") ? 16 : match.starts_with("0b") ? 2 : 10;
        std::uint64_t val = std::stoull(match.data(), nullptr, base);
        std::uint32_t bits = selected_template.bits[i - 1];
        if ((val & ~(~0x0ull << bits)) != val)
        {
            throw UrlEncoderOutOfRangeException("Error. Out of range. Group " + std::to_string(i) + " value is " +
                                                std::to_string(val) +
                                                " which exceeds the maximum amount of bits: " + std::to_string(bits));
        }

        values.push_back(val);
        distribution.push_back(bits);
    }

    return quicr::HexEndec<128>::Encode(distribution, values);
}

std::string UrlEncoder::DecodeUrl(const quicr::Name& code)
{
    return DecodeUrl(code.to_hex());
}

std::string UrlEncoder::DecodeUrl(const std::string& code)
{
    // If the sub pen is not used then we use 0 bits on it
    std::uint32_t pen_sub_bits = 0;

    // Assumed that the first 24 and 8 bits are PEN and Sub PEN respectively.
    // Other bits can be ignored for now.
    const auto& [pen, sub_pen] = quicr::HexEndec<128, Pen_Bits, Sub_Pen_Bits>::Decode(code);
    std::vector<uint8_t> bit_distribution = {Pen_Bits};

    // Get the template for that PEN
    UrlEncoder::template_map temp_map;
    try
    {
        temp_map = templates.at(pen);
    }
    catch (const std::out_of_range&)
    {
        throw UrlDecodeNoMatchException("Error. No templates matches the found PEN " + std::to_string(pen));
    }

    UrlEncoder::url_template temp;

    // Search for this sub PEN
    bool found_sub_pen = false;

    // search for the sub pen
    if (temp_map.find(-1) != temp_map.end())
    {
        temp = temp_map.at(-1);
    }
    else if (temp_map.find(sub_pen) != temp_map.end())
    {
        temp = temp_map.at(sub_pen);
        pen_sub_bits = Sub_Pen_Bits;
        bit_distribution.push_back(pen_sub_bits);
    }
    else
    {
        // No sub PEN was found for this PEN so throw an error.
        throw UrlDecodeNoMatchException("Error. No templates matches the "
                                        "found PEN " +
                                        std::to_string(pen) + " and sub PEN " + std::to_string(sub_pen));
    }

    // Get the regex
    std::string reg = temp.url;
    std::string decoded;

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

        // Find groups and make note of them
        if (ch == '(')
        {

            find = reg.find(')', idx);
            // Check the next couple characters for non matching-groups
            if (idx + 3 < reg.size() && reg[idx + 1] == '?' && reg[idx + 2] == ':')
            {
                // Find the closing bracket for this optional group
                idx = find + 1;
                continue;
            }

            if (find != std::string::npos)
            {
                group_indices.push_back(decoded.length());
                idx = find + 1;
                continue;
            }
        }

        // Skip over ?, ^, $, \, ), +, |
        if (ch == '?' || ch == '^' || ch == '$' || ch == '\\' || ch == ')' || ch == '+' || ch == '|')
        {
            ++idx;
            continue;
        }

        decoded += ch;
        ++idx;
    }

    const size_t num_pens = bit_distribution.size();
    bit_distribution.insert(bit_distribution.end(), temp.bits.begin(), temp.bits.end());
    auto decoded_nums = quicr::HexEndec<128>::Decode(bit_distribution, code);
    decoded_nums.erase(decoded_nums.begin(), decoded_nums.begin() + num_pens);

    size_t str_offset = 0;
    size_t insert_idx;
    std::string insert_str;
    for (std::uint32_t i = 0; i < group_indices.size(); i++)
    {
        // Calc how much we've inserted into the string
        str_offset += insert_str.length();

        // Get the idx with an offset
        insert_idx = group_indices[i] + str_offset;

        // Get the bits converted into decimal
        insert_str = std::to_string(decoded_nums[i]);

        // Insert the numeric values into the decoded string
        decoded.insert(insert_idx, insert_str);
    }

    return decoded;
}

void UrlEncoder::AddTemplate(const std::string& new_template, const bool overwrite)
{
    // The first value must be filled in with their PEN
    const std::string example = "https://!{www.}!webex.com<pen=777>/meeting<int16>/user<int16>";

    // If there is a !{...}! it is an optional group
    const std::regex optional_regex("!\\{(.+)\\}!");

    const std::regex pen_regex("pen=((?:0x|0d)?(?:[0-9ABCDEFabcdef]+|\\d+))");

    const std::regex sub_pen_regex("sub_pen=((?:0x|0d)?(?:[0-9ABCDEFabcdef]+|\\d+))");

    // Parse the string
    // Build a regex out of it.. good luck brett
    const std::regex bit_group_regex("^u?int([1-9][0-9]?)$");
    std::smatch matches;

    // Template variable
    std::pair<std::int16_t, url_template> temp;

    // Find the first angle bracket
    std::size_t start = new_template.find('<') + 1;
    std::size_t end = 0;

    // Start the regex match string
    temp.second.url = '^';

    // Put everything up to the first option into the regex
    temp.second.url += new_template.substr(0, start - 1);

    // Get the pen from the string
    end = new_template.find('>', start);
    std::string pen_str = new_template.substr(start, end - start);
    // Get the match the pen to the regex
    if (!std::regex_match(pen_str, matches, pen_regex))
        throw UrlEncoderException("Error. Missing definition for PEN. "
                                  " Example: " +
                                  example);

    // Check if a value was entered
    if (matches.length() < 2)
        throw UrlEncoderException("Error. No match found for the value of the "
                                  " PEN. Example: :" +
                                  example);

    uint32_t pen_value;
    try
    {
        // Get the pen group value
        std::string_view match;
        match = matches[1].str();
        std::uint16_t base = match.starts_with("0x") ? 16 : match.starts_with("0b") ? 2 : 10;
        pen_value = std::stoul(match.data(), nullptr, base);
    }
    catch (std::out_of_range& ex)
    {
        throw UrlEncoderException("Error. PEN value is too large for a 64 bit "
                                  " PEN value found = " +
                                  matches[1].str() + " " + ex.what());
    }

    if (overwrite)
        templates.erase(pen_value);

    // Check if a sub PEN was provided
    // Check for sub pen
    std::size_t sub_pen_start = new_template.find('<', end) + 1;
    std::size_t sub_pen_end = new_template.find('>', sub_pen_start);
    std::string sub_pen_str = new_template.substr(sub_pen_start, sub_pen_end - sub_pen_start);

    // Attempt to put the value into the int with a max size of 8 bits
    // Throws an error if it goes over the limit
    if (std::regex_match(sub_pen_str, matches, sub_pen_regex))
    {
        uint8_t sub_pen = std::stoul(matches[1].str());

        // Do some error checking
        if (templates.find(pen_value) != templates.end())
        {
            template_map temp_map = templates[pen_value];
            if (temp_map.find(-1) != temp_map.end())
            {
                // If there are not sub PENs for this PEN
                throw UrlEncoderException("Error. Sub PENs are not used for PEN"
                                          " " +
                                          std::to_string(pen_value));
            }

            // Check if this PEN template has a sub PEN of the same key
            if (temp_map.find(sub_pen) != temp_map.end())
            {
                throw UrlEncoderException("Error. Sub PEN key already exists " + std::to_string(sub_pen) +
                                          " for PEN key " + std::to_string(pen_value));
            }
        }

        temp.first = sub_pen;

        // Move the end variable based on the sub PEN's end
        end = sub_pen_end;
    }
    else
    {
        temp.first = -1;
    }

    // Start from the end of the PEN group
    start = end + 1;
    std::string group_str;
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
            group_str = new_template.substr(start, end - start);

            // Run it through regex
            if (!std::regex_match(group_str, matches, bit_group_regex))
            {
                // Failed so throw an error
                throw UrlEncoderException("Error. Could not find the end of group starting at position " +
                                          std::to_string(start));
            }

            // Get the bits from the match
            temp.second.bits.push_back(std::stoul(matches[1].str()));
            start = end + 1;

            // Push regex onto the string
            temp.second.url += ("((?:0x|0d)?(?:[0-9ABCDEFabcdef]+|\\d+))");
            continue;
        }

        temp.second.url += ch;
    }

    // Close the entire string
    temp.second.url += '$';

    // Extract and replace optional chunks
    std::regex_search(temp.second.url, matches, optional_regex);
    std::string optional_str;
    size_t search_idx = 1;
    size_t optional_idx;
    size_t optional_sz;
    size_t period_idx;
    while (search_idx < matches.size())
    {
        optional_str = matches[search_idx++].str();
        optional_idx = temp.second.url.find("!{" + optional_str + "}!");
        optional_sz = optional_str.size() + 4;

        // This could be a problem if there are a lot of wild cards..
        // Replace the period if found
        period_idx = optional_str.find('.');
        if (period_idx != std::string::npos)
            optional_str.replace(optional_str.find('.'), 1, "\\.");

        // Replace the optional chunk !{...}!
        if (optional_idx != std::string::npos)
            temp.second.url.replace(optional_idx, optional_sz, "(?:" + optional_str + ")?");
    }

    templates[pen_value].emplace(temp);
}

void UrlEncoder::AddTemplate(const std::vector<std::string>& new_templates, const bool overwrite)
{
    for (auto temp : new_templates)
        AddTemplate(temp, overwrite);
}

void UrlEncoder::AddTemplate(const std::string* new_templates, const size_t count, const bool overwrite)
{
    for (size_t idx = 0; idx < count; idx++)
        AddTemplate(new_templates[idx], overwrite);
}

void UrlEncoder::AddTemplate(const json& new_templates, const bool overwrite)
{
    // Parse json
    UrlEncoder::pen_template_map res = ParseJson(new_templates);

    for (auto p : res)
    {
        if (overwrite && templates.find(p.first) != templates.end())
        {
            // Overwrite the key's value
            templates[p.first] = p.second;
        }
        else
        {
            // Skips if the key exists
            templates.insert(p);
        }
    }
}

bool UrlEncoder::RemoveTemplate(const std::uint64_t pen)
{
    if (templates.find(pen) == templates.end())
    {
        return false;
    }

    templates.erase(pen);

    return true;
}

bool UrlEncoder::RemoveSubTemplate(const std::uint32_t pen, const std::uint8_t sub_pen)
{
    // Check if the PEN exists
    if (templates.find(pen) == templates.end())
        return false;

    // Get the template map for this PEN
    auto temp_map = templates[pen];

    // Check if this sub PEN exists
    if (temp_map.find(sub_pen) == temp_map.end())
        return false;

    // Remove the sub PEN
    temp_map.erase(sub_pen);

    // If there are no more sub-PENs remove the PEN from the template
    if (temp_map.size() == 0)
        templates.erase(pen);

    return true;
}

json UrlEncoder::TemplatesToJson() const
{
    // Create the template string
    json j;
    json j_pen_list;
    json j_bits;
    json j_temp_map;
    for (auto temp_map : templates)
    {
        j_pen_list.clear();
        j_pen_list["pen"] = temp_map.first;

        for (auto url_temp : temp_map.second)
        {
            j_temp_map.clear();

            j_temp_map["url"] = url_temp.second.url;

            j_temp_map["sub_pen"] = url_temp.first;

            // Fill in the bits array
            j_bits.clear();
            for (auto bit : url_temp.second.bits)
                j_bits.push_back(bit);
            j_temp_map["bits"] = j_bits;

            j_pen_list["templates"].push_back(j_temp_map);
        }

        j.push_back(j_pen_list);
    }

    return j;
}

void UrlEncoder::TemplatesFromJson(const json& data)
{
    templates.clear();
    AddTemplate(data);
}

void UrlEncoder::Clear()
{
    templates.clear();
}

const UrlEncoder::pen_template_map& UrlEncoder::GetTemplates() const
{
    return templates;
}

const UrlEncoder::template_map& UrlEncoder::GetTemplate(std::uint64_t pen) const
{
    return templates.at(pen);
}

const std::uint64_t UrlEncoder::TemplateCount(const bool count_sub_pen) const
{
    if (!count_sub_pen)
    {
        return templates.size();
    }

    std::uint64_t sz = 0;
    for (auto temp_map : templates)
    {
        sz += temp_map.second.size();
    }

    return sz;
}

/** Begin Private functions**/
UrlEncoder::pen_template_map UrlEncoder::ParseJson(const json& data) const
{
    pen_template_map t_templates;
    template_map temps;
    for (unsigned int i = 0; i < data.size(); i++)
    {
        for (unsigned int j = 0; j < data[i]["templates"].size(); j++)
        {
            url_template url_temp;
            // Get the url
            url_temp.url = data[i]["templates"][j]["url"];

            // Get the sub pen
            // url_temp.first = data[i]["templates"][j]["sub_pen"];

            // Get the bits
            url_temp.bits.clear();
            for (auto& element : data[i]["templates"][j]["bits"])
                url_temp.bits.push_back(static_cast<std::uint32_t>(element));

            temps[data[i]["templates"][j]["sub_pen"]] = url_temp;
        }

        // Push the values onto the templates list
        t_templates[static_cast<std::uint64_t>(data[i]["pen"])] = temps;
        temps.clear();
    }

    return t_templates;
}
