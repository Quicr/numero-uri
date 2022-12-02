#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <exception>

#include "UrlEncoder.hh"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

// Singleton object
namespace TemplateFileManager
{
    bool SaveTemplates(const std::string &filename,
                       const json &templates)
    {
        std::ofstream file(filename);
        file.close();

        return true;
    }

    json LoadTemplatesFromFile(const std::string &filename)
    {
        std::ifstream file;

        // Set exceptions
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        // If the file fails to open an exception should be thrown
        file.open(filename);

        json data = json::parse(file);
        file.close();
        return data;
    }

    json LoadTemplatesFromHttp(const std::string &url)
    {

        json data;

        return data;
    }
};
