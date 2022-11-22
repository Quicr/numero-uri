#ifndef URLTEMPLATER_H
#define URLTEMPLATER_H

#include <string>
#include <vector>
#include <map>
#include <stdexcept>

#include "nlohmann/json.hpp"
// Just because writing nlohmann is hard.
using json = nlohmann::json;

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
    UrlTemplater(const std::string template_file);
    bool Add(const std::string new_template);
    bool Remove(const std::uint64_t key);
    void SaveTemplates(const std::string filename) const;
    json ToJson() const;
    bool LoadTemplatesFromFile(const std::string filename);
    bool LoadTemplatesFromHttp(const std::string url);
    bool FromJson(const json& data);
    void Clear();
    const template_list& GetTemplates() const;
    const url_template& GetTemplate(std::uint64_t pen) const;
private:
    std::string filename;
    template_list templates;
};

#endif