#ifndef URLTEMPLATER_H
#define URLTEMPLATER_H

#include <string>
#include <vector>
#include <map>
#include <stdexcept>

class UrlTemplaterException : public std::runtime_error
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

    typedef std::map<std::uint64_t, url_template> template_list;

    UrlTemplater();
    UrlTemplater(std::string template_file);
    bool Add(std::string new_template);
    bool Remove(std::uint64_t key);
    bool SaveTemplates(std::string filename);
    bool LoadTemplates(std::string filename);
    const template_list& GetTemplates() const;
private:
    std::string filename;
    template_list templates;
};

#endif