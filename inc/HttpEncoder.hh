#pragma once

#include <string>
#include <vector>
#include <map>
#include <regex>
#include <stdexcept>

#include "uint128.hh"
#include "UrlTemplater.hh"

class HttpEncoderException : public std::runtime_error
{
public:
    explicit HttpEncoderException(const std::string &what_arg) :
        std::runtime_error(what_arg)
    {
    }

    explicit HttpEncoderException(const char *what_arg) :
        std::runtime_error(what_arg)
    {
    }
};

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
    uint128 EncodeUrl(const std::string &url,
                      const UrlTemplater::template_list& template_list);
    std::string DecodeUrl(const std::string code_str,
                          const UrlTemplater::template_list& template_list,
                          const std::uint32_t bit_format=64);
private:

    // TODO load these in from some sort of file?
    // UrlTemplater::template_list templates = {
    //     { 11259375,
    //         {
    //             "https://[www.]?webex.com/(\\d+)/meeting(\\d+)/user(\\d+)",
    //             { 24, 16, 16 }
    //         }
    //     },
    //     { 1,
    //         {
    //             "https://[www.]?webex.com/(\\d+)/(\\d+)/meeting(\\d+)/user(\\d+)",
    //             { 24, 16, 16, 16 }
    //         }
    //     }
    // };
};