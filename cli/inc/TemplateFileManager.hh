#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <exception>
#include <curl/curl.h>
#include "UrlEncoder.hh"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

// Singleton object
namespace TemplateFileManager
{
    namespace
    {
        size_t write_data(char *buf, size_t sz, size_t nmemb, void *userdata)
        {
            std::stringstream *stream = (std::stringstream*)userdata;
            size_t count = sz * nmemb;
            stream->write(buf, count);
            return count;
        }
    }

    void SaveTemplates(const std::string &filename,
                       const json &templates)
    {
        std::ofstream file(filename);
        file << std::setw(4) << templates << std::endl;
        file.close();
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
        CURL *curl;
        CURLcode res;
        std::stringstream stream;

        curl = curl_easy_init();
        if (!curl)
        {
            std::cout << "curl_easy_init() failed" << std::endl;
            return data;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &stream);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            std::cout << "curl_easy_perform() failed "
                        << curl_easy_strerror(res) << std::endl;
        }
        curl_easy_cleanup(curl);

        stream >> data;

        return data;
    }
};
