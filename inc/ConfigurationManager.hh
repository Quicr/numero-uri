#pragma once

#include <stdio.h>
#include <string>
#include <fstream>

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
        char buf[len];
        #ifdef _WIN32
            bytes = GetModuleFileName(NULL, buf, len);
        #else
            bytes = readlink("/proc/self/exe", buf, len);
            bytes = (bytes > 0) ? bytes : 0;
        #endif

        // Remove the name
        char ch = ' ';
        do
        {
            ch = buf[bytes - 1];
            bytes--;
        } while (ch != '/' && bytes > 0);

        // Include the slash
        return std::string(buf, bytes+1);
    }

    template <typename T>
    void UpdateConfigFile(std::string key, T val)
    {
        std::string config_filepath = PathToExecutable() + "config.json";

        std::fstream config_file;

        config_file.open(config_filepath, std::fstream::in);

        if (!config_file)
        {
            std::cout << "Failed to open config.json creating an empty one\n";
            config_file.open(config_filepath,
                std::fstream::out | std::fstream::trunc);
            config_file << "{\n}\n";

            // If I don't close the file the json parse complains thats
            // there is not contents in the file, even when using
            // config_file.flush()
            config_file.close();
            config_file.open(config_filepath, std::fstream::in);
        }

        // Get the whole config file
        json data = json::parse(config_file);
        // Have to close it because parse does something odd to the file
        // Where it cannot be written to afterwards. Even if the ::out
        // Flag is entered..
        config_file.close();

        config_file.open(config_filepath, std::fstream::out);

        data[key] = val;
        std::cout << std::setw(4) << data << std::endl;

        config_file << std::setw(4) << data << std::endl;
        config_file.close();
    }

    std::string GetTemplateFilePath()
    {
        std::string config_filepath = PathToExecutable() + "config.json";
        std::ifstream config_file(config_filepath);
        if (!config_file.good())
        {
            return "";
        }

        json data = json::parse(config_file);
        config_file.close();

        return std::string(data["template-file"]);
    }
}