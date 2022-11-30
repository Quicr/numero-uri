/*
 *  UrlEncoder.hh
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

#include <string>
#include <vector>
#include <map>
#include <regex>
#include <stdexcept>

#include "uint128.hh"

#include "nlohmann/json.hpp"
// Just because writing nlohmann is hard.
using json = nlohmann::json;

class UrlEncoderException : public std::runtime_error
{
public:
    explicit UrlEncoderException(const std::string &what_arg) :
        std::runtime_error(what_arg)
    {
    }

    explicit UrlEncoderException(const char *what_arg) :
        std::runtime_error(what_arg)
    {
    }
};

class UrlEncoderOutOfRangeException : public std::runtime_error
{
public:
    explicit UrlEncoderOutOfRangeException(const std::string &what_arg,
                                           const std::uint64_t group,
                                           const std::string value) :
        std::runtime_error(what_arg)
    {
        det.group = group;
        det.value = value;
    }

    explicit UrlEncoderOutOfRangeException(const char *what_arg,
                                           const std::uint64_t group,
                                           const std::string value) :
        std::runtime_error(what_arg)
    {
        det.group = group;
        det.value = value;
    }

    struct UrlEncoderOutOfRangeDetails
    {
        std::uint64_t group;
        std::string value;
    };

    const UrlEncoderOutOfRangeDetails details() const
    {
        return det;
    }

private:
    UrlEncoderOutOfRangeDetails det;
};

class UrlEncoderNoMatchException : public std::runtime_error
{
public:
    explicit UrlEncoderNoMatchException(const std::string &what_arg) :
        std::runtime_error(what_arg)
    {
    }

    explicit UrlEncoderNoMatchException(const char *what_arg) :
        std::runtime_error(what_arg)
    {
    }
};

class UrlDecodeNoMatchException : public std::runtime_error
{
public:
    explicit UrlDecodeNoMatchException(const std::string &what_arg) :
        std::runtime_error(what_arg)
    {
    }

    explicit UrlDecodeNoMatchException(const char *what_arg) :
        std::runtime_error(what_arg)
    {
    }
};

class UrlEncoder
{
public:
    // Structure to describe a url template
    typedef struct {
        std::string url;
        std::vector<std::uint32_t> bits;
    } url_template;

    // Alias for url templates
    typedef std::map<std::uint64_t, url_template> template_list;

    /*
    *  UrlEncoder::EncodeUrl
    *
    *  Description:
    *      Receives a url and encodes it into a uint128 object. The url that is
    *      given will be matched against a set of urls that are saved in memory.
    *
    *  Parameters:
    *      url [in]
    *          The url to be encoded
    *
    *  Returns:
    *      uint128 - Contains the encoding
    *
    *  Comments:
    *      Note: To add a new template see UrlEncoder::AddTemplate
    */
    uint128 EncodeUrl(const std::string &url);

    /*
    *  UrlEncoder::DecodeUrl
    *
    *  Description:
    *      Receives a encoding in string format converts it into a uint128 and
    *      passes it to a overloaded UrlEncoder::DecoderUrl. The representation of
    *      the string matters if there is not a format given in the string.
    *      ex. Hexadecimal - 0xF123 | Binary - 0b0101 | Decimal - 0d1232
    *
    *  Parameters:
    *      code_str [in]
    *          A string of an encoded url
    *      rep [in]
    *          The number format representation that the string is
    *          - Default = uint128::Representation::sym - implies the number
    *              format is contained in the string
    *  Returns:
    *      std::string - A decoded url string
    *
    *  Comments:
    *      Note: rep can be sym, hex, dec, bin.
    */
    std::string DecodeUrl(const uint128& code);

    /*
    *  UrlEncoder::DecodeUrl
    *
    *  Description:
    *      Receives a uint128 encoding and attempts to decode and build a url based
    *      on its values from the templates that are loaded into memory.
    *
    *  Parameters:
    *      uint128 [in]
    *          An encoded uint128 object.
    *
    *  Returns:
    *      std::string - A decoded url string
    *
    *  Comments:
    */
    std::string DecodeUrl(const std::string &code_str,
        const uint128::Representation rep=uint128::Representation::sym);

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
    void AddTemplate(const std::string& new_template);

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
    *      bool - If the PEN number is found in the templates and it a template is
    *          deleted.
    *
    *  Comments:
    */
    bool RemoveTemplate(const std::uint64_t key);

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
    const template_list& GetTemplates() const;

    /*
    *  UrlEncoder::GetTemplate
    *
    *  Description:
    *      Gets the template with the id of pen
    *
    *  Parameters:
    *      pen [in]
    *          The PEN number key for a template
    *
    *  Returns:
    *      UrlEncoder::url_template - The url template object that was retrieved
    *
    *  Comments:
    */
    const url_template& GetTemplate(std::uint64_t pen) const;
private:
    template_list templates;
};