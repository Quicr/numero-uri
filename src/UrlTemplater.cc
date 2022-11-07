#include "UrlTemplater.hh"

#include <regex>
#include <iostream>
#include <fstream>

#include "nlohmann/json.hpp"
// Just because writing nlohmann is hard.
using json = nlohmann::json;

UrlTemplater::UrlTemplater()
{

}

UrlTemplater::UrlTemplater(std::string template_file)
    : filename(template_file)
{
    LoadTemplates(filename);
}

bool UrlTemplater::Add(std::string new_template)
{
    // The first value must be filled in with their PEN
    // Example http://www?.webex.com/<int24=77>/meeting<int16>/user<int16>
    const std::string example =
        "http://www.?webex.com/<int24=777>/meeting<int16>/user<int16>";

    // TODO if there is a question mark put it into a non-capturing group
    const std::string optional_www_regex = "(?:www\\.)?";

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

    // TODO this should be a little more robust?
    std::size_t www_start = temp.url.find("www.?");
    if (www_start != std::string::npos)
    {
        temp.url.replace(www_start, 5, optional_www_regex);
    }

    // TODO maybe need to check for a / between the (\\d+)?
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
    std::uint32_t pen_bit = std::stoull(matches[1].str());
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
            temp.bits.push_back(std::stoull(matches[1].str()));
            start = end + 1;

            // Push regex onto the string
            temp.url += ("(\\d+)");
            continue;
        }

        temp.url += ch;
    }

    // Close the entire string
    temp.url += '$';

    templates[pen_value] = temp;

    return true;
}

bool UrlTemplater::Remove(std::uint64_t key)
{
    if (templates.find(key) == templates.end())
    {
        return false;
    }

    templates.erase(key);

    return true;
}

bool UrlTemplater::SaveTemplates(std::string filename)
{
    std::ofstream file(filename);
    json j;


    return true;
}

bool UrlTemplater::LoadTemplates(std::string filename)
{
    this->filename = filename;
    std::ifstream file(filename);

    // Failed to open
    if (!file.good())
        return false;

    // Get the whole file
    json data = json::parse(file);
    // Grab only the templates
    json in_templates = data["templates"];

    for (unsigned int i = 0; i < in_templates.size(); i++)
    {
        url_template temp;

        // Get the url
        temp.url = in_templates[i]["url"];

        // Get the bits
        for (auto& element : in_templates[i]["bits"])
            temp.bits.push_back(static_cast<std::uint32_t>(element));

        // Push the values onto the templates list
        templates[static_cast<std::uint64_t>(in_templates[i]["pen"])] = temp;
    }

    return true;
}

const UrlTemplater::template_list& UrlTemplater::GetTemplates() const
{
    return templates;
}