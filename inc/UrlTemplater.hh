#pragma once

#include <string>
#include <map>

class UrlTemplaterException : std::runtime_error
{
public:
    explicit UrlTemplaterException(const std::string &what_arg) :
        std::runtime_error(what_arg)
    {
    }

    explicit UrlTemplaterException(const char *what_arg) :
        std::runtime_error(what_arg)
    {
    }
};

class UrlTemplater
{
public:
    typedef struct {
        std::string url;
        std::vector<std::uint32_t> bits;
    } url_template;

    typedef std::map<std::uint32_t, const url_template> template_list;

    UrlTemplater(std::string template_file);
    bool Add(std::string new_template);
    bool Remove(std::uint32_t key);
private:
    std::string filename;
    template_list templates;
};