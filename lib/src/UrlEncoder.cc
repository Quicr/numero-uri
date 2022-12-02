#include "UrlEncoder.hh"
#include <regex>
#include "NumericHelper.hh"
#include <iostream>

big_uint UrlEncoder::EncodeUrl(const std::string &url)
{
    // To extract the 3 groups from each url format
    std::smatch matches;

    std::uint64_t pen;
    std::int16_t sub_pen;
    UrlEncoder::url_template selected_template;
    bool found_template = false;
    for (auto temp_map : templates)
    {
        // Find match
        for (auto url_temp : temp_map.second)
        {
            // Set the selected templated to whatever we are looking at
            selected_template = url_temp.second;

            // Check if this template matches our url
            if (!std::regex_match(url, std::regex(selected_template.url)))
                continue;

            // Set the PEN to the matched template PEN
            pen = temp_map.first;
            sub_pen = url_temp.first;
            found_template = true;
            break;
        }

        // Found the template break out of the loop
        if (found_template) break;
    }

    if (!found_template)
        throw UrlEncoderException("Error. No match found for given url: "
            + url);

    // If there is not a match then there is no template for this PEN
    if (!std::regex_match(url, matches, std::regex(selected_template.url)))
        throw UrlEncoderNoMatchException("Error. No match for url: " + url);

    // Need the same number of numbers as the template expects
    if (matches.size() - 1 != selected_template.bits.size())
        throw UrlEncoderNoMatchException("Error. Match is missing values for "
            "the given template");


    big_uint val = pen;
    std::uint32_t bits = Pen_Bits;
    std::uint32_t offset_bits = 0;
    big_uint max_val;

    big_uint encoded;

    // Set the PEN value and bits
    encoded.SetValue(val, bits, offset_bits);
    offset_bits += bits;

    // Set the sub PEN value and bits if it is positive
    if (sub_pen >= 0)
    {
        val = sub_pen;
        bits = Sub_Pen_Bits;
        encoded.SetValue(val, bits, offset_bits);
        offset_bits += bits;
    }

    // Skip the first group since its the whole match
    for (std::uint32_t i = 1; i < matches.size(); i++)
    {
        val.FromString(matches[i].str(), big_uint::Representation::dec);
        bits = selected_template.bits[i-1];

        // Ensure the value isn't greater than the support bits
        max_val = big_uint::BitValue(bits);

        if (val > max_val)
        {
            throw UrlEncoderOutOfRangeException(
                "Error. Out of range. Group " + std::to_string(i)
                + " value is " + val.ToDecimalString()
                + " but the max value is " + max_val.ToDecimalString(),
                i, val.ToDecimalString());
        }

        // Set the value in the big_uint variable
        encoded.SetValue(val, bits, offset_bits);

        // Slide the bit window over
        offset_bits += bits;
    }

    return encoded;
}

std::string UrlEncoder::DecodeUrl(const std::string &code_str,
                                  const big_uint::Representation rep)
{
    // Convert the string to the big_uint
    big_uint code(code_str, rep);
    return DecodeUrl(code);
}

std::string UrlEncoder::DecodeUrl(const big_uint &code)
{
    // If the sub pen is not used then we use 0 bits on it
    std::uint32_t pen_sub_bits = 0;

    std::string decoded;

    // Assumed that the first 24 bits is always the PEN
    std::uint64_t pen = code.GetValue(Pen_Bits, 0);

    // Check if pen exists in list of templates
    if (templates.find(pen) == templates.end())
        throw UrlDecodeNoMatchException(
            "Error. No templates matches the found PEN " + std::to_string(pen));

    // Get the template for that PEN
    auto temp_map = templates.at(pen);
    UrlEncoder::url_template temp;

    // Get the sub PEN value from the big_uint offset by the PEN bits
    std::uint64_t sub_pen = code.GetValue(Sub_Pen_Bits, Pen_Bits);

    // Search for this sub PEN
    bool found_sub_pen = false;

    // TODO use find instead, search for a -1
    // search for the sub pen
    if (temp_map.find(-1) != temp_map.end())
    {
        temp = temp_map.at(-1);
    }
    else if (temp_map.find(sub_pen) != temp_map.end())
    {
        temp = temp_map.at(sub_pen);
        pen_sub_bits = Sub_Pen_Bits;
    }
    else
    {
        // No sub PEN was found for this PEN so throw an error.
        throw UrlDecodeNoMatchException("Error. No templates matches the "
            "found PEN " + std::to_string(pen) + " and sub PEN " +
            std::to_string(sub_pen));
    }

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

        // Find groups and make note of them
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
    std::uint32_t bit_offset = Pen_Bits + pen_sub_bits;
    for (std::uint32_t i = 0; i < group_indices.size(); i++)
    {
        // Get the number of bits for this number
        // skip the pen bits
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

void UrlEncoder::AddTemplate(const std::string& new_template,
                             const bool overwrite)
{
    // The first value must be filled in with their PEN
    const std::string example =
        "https://!{www.}!webex.com<int24=777>/meeting<int16>/user<int16>";

    // If there is a !{...}! it is an optional group
    const std::regex optional_regex("!\\{(.+)\\}!");

    const std::regex pen_regex("pen=(\\d+)");

    const std::regex sub_pen_regex("sub_pen=(\\d+)");

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
    temp.second.url += new_template.substr(0, start-1);

    // Get the pen from the string
    end = new_template.find('>', start);
    std::string pen_str = new_template.substr(start, end-start);
    // Get the match the pen to the regex
    if (!std::regex_match(pen_str, matches, pen_regex))
        throw UrlEncoderException("Error. Missing definition for PEN. "
            " Example: " + example);

    // Check if a value was entered
    if (matches.length() < 2)
        throw UrlEncoderException("Error. No match found for the value of the "
            " PEN. Example: :" + example);

    big_uint* pen_value;
    try
    {
        // Get the pen group value
        pen_value = new big_uint(matches[1].str(),
                                 big_uint::Representation::dec,
                                 24);
    }
    catch(std::out_of_range &ex)
    {
        throw UrlEncoderException("Error. PEN value is too large for a 64 bit "
            " PEN value found = " + matches[1].str() + " " + ex.what());
    }

    if (overwrite)
        templates.erase(pen_value->value());

    // FIX, this doesn't allow someone to add a sub-pen and pen at the same time.
    // Do some error checking
    if (templates.find(pen_value->value()) != templates.end())
    {
        // PEN was found
        // Check if a sub PEN was provided
        start = new_template.find('<', end) + 1;
        end = new_template.find('>', start);
        auto temp_map = templates[pen_value->value()];
        std::string sub_pen_str = new_template.substr(start, end-start);
        if (!std::regex_match(sub_pen_str, matches, sub_pen_regex) &&
            temp_map.find(-1) == temp_map.end())
        {
            // If there are sub PENs for this PEN and one is not provided
            throw UrlEncoderException("Error. PEN key " +
                pen_value->ToDecimalString() + " exists but no sub-pen was "
                "provided.");
        }
        else if (temp_map.find(-1) != temp_map.end())
        {
            // If there are not sub PENs for this PEN
            throw UrlEncoderException("Error. Sub PENs are not used for PEN"
                " " + pen_value->ToDecimalString());
        }

        // Attempt to put the value into the int with a max size of 8 bits
        // Throws an error if it goes over the limit
        big_uint sub_pen(matches[1].str(),
                         big_uint::Representation::dec,
                         8);

        // Check if this PEN template has a sub PEN of the same key
        if (temp_map.find(sub_pen.value()) != temp_map.end())
        {
            throw UrlEncoderException("Error. Sub PEN key already exists "
                + sub_pen.ToDecimalString());
        }

        temp.first = sub_pen.value();
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
            temp.second.bits.push_back(std::stoul(matches[1].str()));
            start = end + 1;

            // Push regex onto the string
            temp.second.url += ("(\\d+)");
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


    templates[pen_value->value()].emplace(temp);
}

void UrlEncoder::AddTemplate(const std::vector<std::string>& new_templates,
                             const bool overwrite)
{
    for (auto temp : new_templates)
        AddTemplate(temp, overwrite);
}

void UrlEncoder::AddTemplate(const std::string* new_templates,
                             const size_t count,
                             const bool overwrite)
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

bool UrlEncoder::RemoveSubTemplate(const std::uint32_t pen,
                                   const std::uint8_t sub_pen)
{
    if (templates.find(pen) == templates.end())
        return false;

    auto temp_map = templates[pen];

    if (temp_map.find(sub_pen) == temp_map.end())
        return false;

    temp_map.erase(sub_pen);

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

const UrlEncoder::template_map&
    UrlEncoder::GetTemplate(std::uint64_t pen) const
{
    return templates.at(pen);
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
