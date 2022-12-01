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
    for (auto pen_temp : templates)
    {
        // Find match
        for (size_t i = 0; i < pen_temp.second.size(); i++)
        {
            // Set the selected templated to whatever we are looking at
            selected_template = pen_temp.second[i];

            // Check if this template matches our url
            if (!std::regex_match(url, std::regex(selected_template.url)))
                continue;

            // Set the PEN to the matched template PEN
            pen = pen_temp.first;
            sub_pen = selected_template.sub_pen;
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
    auto pen_templates = templates.at(pen);
    UrlEncoder::url_template temp;

    // Get the sub PEN value from the big_uint offset by the PEN bits
    std::uint64_t sub_pen = code.GetValue(Sub_Pen_Bits, Pen_Bits);

    // Search for this sub PEN
    bool found_sub_pen = false;
    for (auto pen_temp : pen_templates)
    {
        // If there is a sub PEN with -1 then sub PENs are not being used
        if (pen_temp.sub_pen == -1)
        {
            temp = pen_temp;
            found_sub_pen = true;
            break;
        }

        if (pen_temp.sub_pen != sub_pen) continue;

        temp = pen_temp;
        pen_sub_bits = Sub_Pen_Bits;
        found_sub_pen = true;
    }

    // No sub PEN was found for this PEN so throw an error.
    if (!found_sub_pen)
        throw UrlDecodeNoMatchException("Error. No templates matches the "
            "found PEN " + std::to_string(pen) + " and sub PEN " +
            std::to_string(sub_pen));


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

void UrlEncoder::AddTemplate(const std::string& new_template)
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
    url_template temp;

    // Find the first angle bracket
    std::size_t start = new_template.find('<') + 1;
    std::size_t end = 0;

    // Start the regex match string
    temp.url = '^';

    // Put everything up to the first option into the regex
    temp.url += new_template.substr(0, start-1);

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

    // Do some error checking
    if (templates.find(pen_value->value()) != templates.end())
    {
        // PEN was found
        // Check if a sub PEN was provided
        start = new_template.find('<', end) + 1;
        end = new_template.find('>', start);
        std::string sub_pen_str = new_template.substr(start, end-start);
        if (!std::regex_match(sub_pen_str, matches, sub_pen_regex))
            throw UrlEncoderException("Error. PEN key " +
                pen_value->ToDecimalString() + " exists but no sub-pen was "
                "provided.");

        // Attempt to put the value into the int with a max size of 8 bits
        // Throws an error if it goes over the limit
        big_uint sub_pen(matches[1].str(),
                         big_uint::Representation::dec,
                         8);

        // Check if this PEN template has a sub PEN of the same key
        auto temps = templates[pen_value->value()];
        for (auto t : temps)
        {
            if (t.sub_pen == -1)
                throw UrlEncoderException("Error. Sub PENs are not used for PEN"
                    " " + pen_value->ToDecimalString());
            if (t.sub_pen == sub_pen.value())
                throw UrlEncoderException("Error. Sub PEN key already exists "
                    + std::to_string(t.sub_pen));
        }

        temp.sub_pen = sub_pen.value();
    }
    else
    {
        temp.sub_pen = -1;
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


    templates[pen_value->value()].push_back(temp);
}

void UrlEncoder::AddTemplate(const std::vector<std::string>& new_templates)
{
    for (auto temp : new_templates)
        AddTemplate(temp);
}

void UrlEncoder::AddTemplate(const std::string* new_templates, size_t count)
{
    for (size_t idx = 0; idx < count; idx++)
        AddTemplate(new_templates[0]);
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

    auto temps = templates[pen];

    std::uint8_t idx = 0;
    for (auto sub_temps : temps)
    {
        if (sub_temps.sub_pen == sub_pen)
        {
            temps.erase(temps.begin() + idx);
            break;
        }
        idx++;
    }
}

json UrlEncoder::TemplatesToJson() const
{
    // Create the template string
    json j;
    json j_temp;
    json j_bits;
    json j_sub_temp;
    for (auto pen_temp : templates)
    {
        j_temp.clear();
        j_temp["pen"] = pen_temp.first;

        for (auto temp : pen_temp.second)
        {
            j_sub_temp.clear();

            j_sub_temp["url"] = temp.url;

            j_sub_temp["sub_pen"] = temp.sub_pen;

            // Fill in the bits array
            j_bits.clear();
            for (auto bit : temp.bits)
                j_bits.push_back(bit);
            j_sub_temp["bits"] = j_bits;

            j_temp["templates"].push_back(j_sub_temp);
        }

        j.push_back(j_temp);
    }

    return j;
}

void UrlEncoder::TemplatesFromJson(const json& data)
{
    v_template temps;
    for (unsigned int i = 0; i < data.size(); i++)
    {
        for (unsigned int j = 0; j < data[i]["templates"].size(); j++)
        {
            url_template url_temp;
            // Get the url
            url_temp.url = data[i]["templates"][j]["url"];

            // Get the sub pen
            url_temp.sub_pen = data[i]["templates"][j]["sub_pen"];

            // Get the bits
            url_temp.bits.clear();
            for (auto& element : data[i]["templates"][j]["bits"])
                url_temp.bits.push_back(static_cast<std::uint32_t>(element));

            temps.push_back(url_temp);
        }

        // Push the values onto the templates list
        templates[static_cast<std::uint64_t>(data[i]["pen"])] = temps;
        temps.clear();
    }
}

void UrlEncoder::Clear()
{
    templates.clear();
}

const UrlEncoder::template_list& UrlEncoder::GetTemplates() const
{
    return templates;
}

const UrlEncoder::v_template&
    UrlEncoder::GetTemplate(std::uint64_t pen) const
{
    return templates.at(pen);
}

