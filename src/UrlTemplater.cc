#include "UrlTemplater.hh"

#include <regex>
#include <fstream>

UrlTemplater::UrlTemplater()
{

}

UrlTemplater::UrlTemplater(const std::string template_file)
    : filename(template_file)
{
    LoadTemplates(filename);
}

bool UrlTemplater::Add(const std::string new_template)
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
        throw UrlTemplaterException(msg);
    }

    // Get the bits
    std::uint32_t pen_bit = std::stoul(matches[1].str());
    if (pen_bit > 64)
    {
        std::string msg = "Error. PEN cannot have more than 64 bits";
        msg += " Example: " + example;
        throw UrlTemplaterException(msg);
    }

    // Check if a value was entered
    if (matches.length() < 3)
    {
        std::string msg = "Error. No match found for the value of the PEN";
        msg += " Example: " + example;
        throw UrlTemplaterException(msg);
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
        throw msg;
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
                throw UrlTemplaterException(
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

bool UrlTemplater::Remove(const std::uint64_t key)
{
    if (templates.find(key) == templates.end())
    {
        return false;
    }

    templates.erase(key);

    return true;
}

void UrlTemplater::SaveTemplates(const std::string filename) const
{
    json j = ToJson();
    std::ofstream file(filename);
    file << std::setw(4) << j << std::endl;
    file.close();
}

json UrlTemplater::ToJson() const
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

bool UrlTemplater::LoadTemplates(const std::string filename)
{
    this->filename = filename;
    std::ifstream file(filename);

    // Failed to open
    if (!file.good())
        return false;

    // Get the whole file
    json data = json::parse(file);
    file.close();
    return FromJson(data);
}

bool UrlTemplater::FromJson(const json& data)
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

void UrlTemplater::Clear()
{
    templates.clear();
}

const UrlTemplater::template_list& UrlTemplater::GetTemplates() const
{
    return templates;
}

const UrlTemplater::url_template&
    UrlTemplater::GetTemplate(std::uint64_t pen) const
{
    return templates.at(pen);
}