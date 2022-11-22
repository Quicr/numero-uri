#pragma once

#include <vector>
#include <string>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

// Singleton object
class TemplateManager
{
    public:
        TemplateManager(TemplateManager& other) = delete;
        void operator=(const TemplateManager& other) = delete;

        static TemplateManager* Instance()
        {
            if (TemplateManager::ptr == nullptr)
                ptr = new TemplateManager();
            return ptr;
        }

        bool SaveTemplates(const std::vector<json> &templates) const
        {

        }

        json LoadTemplatesFromFile(const std::string &filename) const
        {

        }

        json LoadTemplatesFromHttp(const std::string &url) const
        {

        }

    private:
        TemplateManager(){}
        static TemplateManager* ptr;
};

TemplateManager* TemplateManager::ptr = nullptr;

