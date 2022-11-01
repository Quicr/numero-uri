#pragma once

#include <string>
#include <vector>
#include <map>
#include <regex>

#include "uint128.hh"

class HttpEncodeOutOfRangeException : public std::runtime_error
{
public:
    explicit HttpEncodeOutOfRangeException(const std::string &what_arg,
                                           const std::uint64_t group,
                                           const std::uint64_t value) :
        std::runtime_error(what_arg)
    {
        det.group = group;
        det.value = value;
    }

    explicit HttpEncodeOutOfRangeException(const char *what_arg,
                                           const std::uint64_t group,
                                           const std::uint64_t value) :
        std::runtime_error(what_arg)
    {
        det.group = group;
        det.value = value;
    }

    struct HttpEncodeOutOfRangeDetails
    {
        std::uint64_t group;
        std::uint64_t value;
    };

    const HttpEncodeOutOfRangeDetails details() const
    {
        return det;
    }

private:
    HttpEncodeOutOfRangeDetails det;
};

class HttpEncodeNoMatchException : public std::runtime_error
{
public:
    explicit HttpEncodeNoMatchException(const std::string &what_arg) :
        std::runtime_error(what_arg)
    {
    }

    explicit HttpEncodeNoMatchException(const char *what_arg) :
        std::runtime_error(what_arg)
    {
    }
};

class HttpDecodeNoMatchException : public std::runtime_error
{
public:
    explicit HttpDecodeNoMatchException(const std::string &what_arg) :
        std::runtime_error(what_arg)
    {
    }

    explicit HttpDecodeNoMatchException(const char *what_arg) :
        std::runtime_error(what_arg)
    {
    }
};

class HttpEncoder
{
public:
    static uint128 EncodeUrl(const std::string &url);
    static std::string DecodeUrl(const std::string code_str,
                                 const std::uint32_t bit_format=64);
private:
    typedef struct {
        std::string url;
        std::vector<std::uint32_t> bits;
    } url_template;

    // TODO load these in from some sort of file?
    static inline std::map<std::uint32_t, const url_template> templates = {
        { 11259375,
            {
                "https://[www.]?webex.com/(\\d+)/meeting(\\d+)/user(\\d+)",
                { 24, 16, 16 }
            }
        },
        { 1,
            {
                "https://[www.]?webex.com/(\\d+)/(\\d+)/meeting(\\d+)/user(\\d+)",
                { 24, 16, 16, 16 }
            }
        }
    };
};