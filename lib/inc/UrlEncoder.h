/*
 *  UrlEncoder.h
 *
 *  Copyright (C) 2022
 *  Cisco Systems, Inc.
 *  All Rights Reserved.
 *
 *  Description:
 *      Emulates a 128 bit unsigned integer number.
 *
 *  Portability Issues:
 *      None.
 */

#pragma once

#include <quicr/namespace.h>

#include <map>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct UrlEncoderException : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

struct UrlEncoderNoMatchException : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

struct UrlDecodeNoMatchException : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

struct UrlEncoderOutOfRangeException : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

class UrlEncoder
{
  public:
    // Structure to describe a url template
    typedef struct
    {
        std::string url;
        std::vector<std::uint32_t> bits;
    } url_template;

    typedef std::map<std::int16_t, url_template> template_map;

    // Alias for url templates
    typedef std::map<std::uint64_t, template_map> pen_template_map;

    static constexpr std::uint16_t Pen_Bits = 24;
    static constexpr std::uint16_t Sub_Pen_Bits = 8;

    /*
     *  UrlEncoder::UrlEncoder
     *
     *  Description:
     *       Initializes an empty UrlEncoder
     *
     *  Parameters:
     *
     *  Returns:
     *
     *  Comments:
     *
     */
    UrlEncoder();

    /*
     *  UrlEncoder::UrlEncoder
     *
     *  Description:
     *       Initializes an UrlEncoder with the given template
     *
     *  Parameters:
     *      init_template [in]
     *          A string containing a template
     *
     *  Returns:
     *
     *  Comments:
     *
     */
    UrlEncoder(const std::string& init_template);

    /*
     *  UrlEncoder::UrlEncoder
     *
     *  Description:
     *       Initializes an UrlEncoder with the given templates
     *
     *  Parameters:
     *      init_template [in]
     *          A vector of strings containing templates
     *
     *  Returns:
     *
     *  Comments:
     *
     */
    UrlEncoder(const std::vector<std::string>& init_templates);

    /*
     *  UrlEncoder::UrlEncoder
     *
     *  Description:
     *       Initializes an UrlEncoder with the given templates
     *
     *  Parameters:
     *      init_template [in]
     *          A array of strings containing templates
     *      count [in]
     *          The size of the array
     *
     *  Returns:
     *
     *  Comments:
     *
     */
    UrlEncoder(const std::string* init_templates, const size_t count);

    /*
     *  UrlEncoder::UrlEncoder
     *
     *  Description:
     *       Initializes an UrlEncoder with the given templates
     *
     *  Parameters:
     *      init_template [in]
     *          A json object that contains complete templates
     *
     *  Returns:
     *
     *  Comments:
     *
     */
    UrlEncoder(const json& init_templates);

    /*
     *  UrlEncoder::EncodeUrl
     *
     *  Description:
     *      Receives a url and encodes it into a big_uint object. The url that is
     *      given will be matched against a set of urls that are saved in memory.
     *
     *  Parameters:
     *      url [in]
     *          The url to be encoded
     *
     *  Returns:
     *      big_uint - Contains the encoding
     *
     *  Comments:
     *      Note: To add a new template see UrlEncoder::AddTemplate
     */
    quicr::Namespace EncodeUrl(const std::string& url) const;

    /*
     *  UrlEncoder::DecodeUrl
     *
     *  Description:
     *      Receives a encoding in string format converts it into a big_uint and
     *      passes it to a overloaded UrlEncoder::DecoderUrl. The representation
     * of the string matters if there is not a format given in the string. ex.
     * Hexadecimal - 0xF123 | Binary - 0b0101 | Decimal - 0d1232
     *
     *  Parameters:
     *      code [in]
     *          A string or namespace of an encoded url
     *      rep [in]
     *          The number format representation that the string is
     *          - Default = big_uint::Representation::sym - implies the number
     *              format is contained in the string
     *  Returns:
     *      std::string - A decoded url string
     *
     *  Comments:
     *      Note: rep can be sym, hex, dec, bin.
     */
    std::string DecodeUrl(const quicr::Namespace& code);

    /*
     *  UrlEncoder::AddTemplate
     *
     *  Description:
     *      Adds a template from a url to memory that can be used to encode
     *      and decode urls.
     *      - Expected format to be a regular url with the PEN value for your
     *          org to be immediately following the top level domain and numbers
     *          replaced by <intxx> values.
     *          Ex. https://webex.com<int24=777>/meeting<int53>
     *      - Additionally surround a portion of url to make it optional.
     *              Ex. https://!{www.}!webex.com<int24=777>/meeting<int16>
     *
     *  Parameters:
     *      new_template [in]
     *          A string url that will be used to make a template for the
     *          encoding and decoding
     *
     *  Returns:
     *      std::string - A decoded url string
     *
     *  Comments:
     */
    void AddTemplate(const std::string& new_template, const bool overwrite = false);

    /*
     *  UrlEncoder::AddTemplate
     *
     *  Description:
     *      Adds a vectors of templates to memory
     *
     *  Parameters:
     *      new_templates [in]
     *          A vector of strings of templates
     *
     *  Returns:
     *
     *  Comments:
     */
    void AddTemplate(const std::vector<std::string>& new_templates, const bool overwrite = false);

    /*
     *  UrlEncoder::AddTemplate
     *
     *  Description:
     *      Adds a array of templates to memory
     *
     *  Parameters:
     *      new_templates [in]
     *          An array of strings of templates
     *      count [in]
     *          Number of new_templates in the string array
     *      Overwrite [in]
     *          Overwrite flag will cause overwriting of existing PEN
     *          otherwise they will be skipped
     *
     *  Returns:
     *
     *  Comments:
     */
    void AddTemplate(const std::string* new_templates, const size_t count, const bool overwrite = false);

    /*
     *  UrlEncoder::AddTemplate
     *
     *  Description:
     *      Adds templates from a json object
     *
     *  Parameters:
     *      new_templates [in]
     *          A json object with >=1 templates
     *      Overwrite [in]
     *          Overwrite flag will cause overwriting of existing PEN
     *          otherwise they will be skipped
     *
     *  Returns:
     *
     *  Comments:
     */
    void AddTemplate(const json& new_templates, const bool overwrite = false);

    /*
     *  UrlEncoder::RemoveTemplate
     *
     *  Description:
     *      Removes a template from memory
     *
     *  Parameters:
     *      pen [in]
     *          The PEN number key for a template
     *
     *  Returns:
     *      bool - If the PEN number is found in the templates and it a
     *       template is deleted.
     *
     *  Comments:
     */
    bool RemoveTemplate(const std::uint64_t pen);

    /*
     *  UrlEncoder::RemoveSubTemplate
     *
     *  Description:
     *      Removes a sub template from memory. If there are no sub templates
     *       remaining then the PEN template will be removed.
     *
     *  Parameters:
     *       pen [in]
     *          The PEN number key for a template
     *
     *  Returns:
     *      bool - If the Sub-PEN number is found in the templates and a
     *       template is deleted then true is returned
     *
     *  Comments:
     */
    bool RemoveSubTemplate(const std::uint32_t pen, const std::uint8_t sub_pen);

    /*
     *  UrlEncoder::TemplatesToJson
     *
     *  Description:
     *      Converts the templates in memory into a json string.
     *
     *  Parameters:
     *
     *  Returns:
     *
     *  Comments:
     */
    json TemplatesToJson() const;

    /*
     *  UrlEncoder::TemplatesFromJson
     *
     *  Description:
     *      Loads templates from a json object
     *
     *  Parameters:
     *      data [in]
     *          The json object that will be used to load templates into memory
     *
     *  Returns:
     *
     *  Comments:
     *      Note- This will clear current templates, if you want to append use
     *            add_templates(json obj) function.
     */
    void TemplatesFromJson(const json& data);

    /*
     *  UrlEncoder::Clear
     *
     *  Description:
     *      Removes all templates from memory
     *
     *  Parameters:
     *
     *  Returns:
     *
     *  Comments:
     */
    void Clear();

    /*
     *  UrlEncoder::GetTemplates
     *
     *  Description:
     *      Returns all of the templates loaded in memory
     *
     *  Parameters:
     *
     *  Returns:
     *
     *  Comments:
     */
    const pen_template_map& GetTemplates() const;

    /*
     *  UrlEncoder::GetTemplate
     *
     *  Description:
     *      Gets the templates for the id of PEN
     *
     *  Parameters:
     *      pen [in]
     *          The PEN number key for a template
     *
     *  Returns:
     *      UrlEncoder::templates - The url templates vector that was retrieved
     *
     *  Comments:
     */
    const template_map& GetTemplate(std::uint64_t pen) const;

    std::uint64_t TemplateCount(const bool count_sub_pen = true) const;

  private:
    /*
     *  UrlEncoder::PraseJson
     *
     *  Description:
     *      Parses json and returns a pen_template_map
     *
     *  Parameters:
     *      data [in]
     *          json object to be parsed
     *
     *  Returns:
     *      UrlEncoder::pen_template_map - A map of PEN templates
     *
     *  Comments:
     */
    pen_template_map ParseJson(const json& data) const;

    /* Variables */
    pen_template_map templates;
};
