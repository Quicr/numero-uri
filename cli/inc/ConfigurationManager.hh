#pragma once

#include <stdio.h>
#include <string>
#include <fstream>
#include <iostream>

#include "nlohmann/json.hpp"
// Just because writing nlohmann is hard.
using json = nlohmann::json;

#ifdef _WIN32
#include <Windows.h>
#else
#include <limits.h>
#include <unistd.h>
#endif
namespace ConfigurationManager
{
    std::string PathToExecutable()
    {
        const size_t len = FILENAME_MAX;
        std::int64_t bytes;
        char slash;
        char buf[len];
        #ifdef _WIN32
            bytes = GetModuleFileName(NULL, buf, len);
            slash = '\\';
        #else
            bytes = readlink("/proc/self/exe", buf, len);
            bytes = (bytes > 0) ? bytes : 0;
            slash = '/';
        #endif

        // Remove the name
        char ch = ' ';
        do
        {
            ch = buf[bytes - 1];
            bytes--;
        } while (ch != slash && bytes > 0);

        // Copy to a string
        std::string path;
        for (std::int64_t idx = 0; idx < bytes; idx++)
        {
            path.push_back(buf[idx]);
        }

        // Include the slash
        return path;
    }

    std::string InitConfig()
    {
        std::string config_filepath = PathToExecutable() + "/config.json";

        std::fstream config_file;

        config_file.open(config_filepath, std::fstream::in);

        if (!config_file)
        {
            std::cout << "Failed to open config.json creating an initial one\n";
            config_file.open(config_filepath,
                std::fstream::out | std::fstream::trunc);
            json data;
            data["template-file"] = PathToExecutable() + "/templates.json";
            config_file << std::setw(4) << data;

            // If I don't close the file the json parse complains thats
            // there is not contents in the file, even when using
            // config_file.flush()
            config_file.close();
        }

        return config_filepath;
    }

    template <typename T>
    void UpdateConfigFile(std::string key, T val)
    {
        std::string config_filepath = InitConfig();
        std::fstream config_file(config_filepath, std::fstream::in);

        // Get the whole config file
        json data = json::parse(config_file);
        // Have to close it because parse does something odd to the file
        // Where it cannot be written to afterwards. Even if the ::out
        // Flag is entered..
        config_file.close();

        config_file.open(config_filepath, std::fstream::out);

        data[key] = val;

        config_file << std::setw(4) << data << std::endl;
        config_file.close();
    }

    std::string GetTemplateFilePath()
    {
        std::ifstream config_file;

        config_file.open((InitConfig()));
        json data = json::parse(config_file);
        config_file.close();

        std::string template_filepath = data["template-file"];

        // Make sure the template file exists
        std::fstream template_file(template_filepath, std::fstream::in);

        if (!template_file)
        {
            std::cout << "Failed to open template file "
                      << "creating an empty one\n";
            template_file.open(template_filepath,
                std::fstream::out | std::fstream::trunc);
            template_file << "{\n}\n";
        }

        template_file.close();

        return template_filepath;
    }
}