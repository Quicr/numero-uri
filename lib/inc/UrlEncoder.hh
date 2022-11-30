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
    typedef struct {
        std::string url;
        std::vector<std::uint32_t> bits;
    } url_template;
    typedef std::map<std::uint64_t, url_template> template_list;

    uint128 EncodeUrl(const std::string &url);

    std::string DecodeUrl(const std::string code_str,
        const uint128::Representation rep=uint128::Representation::sym);

    bool AddTemplate(const std::string new_template);
    bool RemoveTemplate(const std::uint64_t key);
    json ToJson() const;
    bool FromJson(const json& data);
    void Clear();
    const template_list& GetTemplates() const;
    const url_template& GetTemplate(std::uint64_t pen) const;
private:
    template_list templates;
};