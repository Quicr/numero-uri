#include <iostream>
#include <string>
#include "UrlEncoder.hh"
#include "big_uint.hh"
#include "ConfigurationManager.hh"
#include "TemplateFileManager.hh"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

int main(int argc, char** argv)
{

    try
    {
        if (argc == 1)
        {
            std::cout << "Requires at least 1 argument\n";
            return 1;
        }

        UrlEncoder encoder;

        // Get the template file from the configuration file
        std::string template_file = ConfigurationManager::GetTemplateFilePath();

        json data = TemplateFileManager::LoadTemplatesFromFile(template_file);
        encoder.TemplatesFromJson(data);
        if (strcmp(argv[1], "encode") == 0)
        {
            // encode test - https://webex.com/1/meeting1234/user3213
            big_uint encoded = encoder.EncodeUrl(argv[2]);
            std::cout << encoded.ToHexString() << "\n";
        }
        else if (strcmp(argv[1], "decode") == 0)
        {
            //decode test - 0000000112021553484800000000000000000000
            std::string decoded = encoder.DecodeUrl(argv[2]);
            std::cout << decoded << "\n";
        }
        else if (strcmp(argv[1], "config") == 0)
        {
            ConfigurationManager::UpdateConfigFile(argv[2], argv[3]);
            if (strcmp(argv[2], "template-file") == 0)
            {
                std::string f_path = ConfigurationManager::GetTemplateFilePath();
                encoder.Clear();
                json data = TemplateFileManager::LoadTemplatesFromFile(f_path);
                encoder.TemplatesFromJson(data);
                std::cout << "Updated template file location\n";
            }
        }
        else if (strcmp(argv[1], "add-template") == 0)
        {
            encoder.AddTemplate(argv[2]);

            // Save the template file
            json data = encoder.TemplatesToJson();
            TemplateFileManager::SaveTemplates(template_file, data);
            std::cout << "Added template.\n";
        }
        else if (strcmp(argv[1], "remove-template") == 0)
        {
            std::uint64_t pen = std::stoull(argv[2]);
            if (!encoder.RemoveTemplate(pen))
                throw UrlEncoderException("Failed to remove template from list");

            // Save the template file
            // encoder.SaveTemplates(template_file);
            std::cout << "Template with PEN " << pen << " has been removed.\n";
        }
        else
        {
            // Print help statement..?
            std::cout << "Argument requires at least 1 argument\n";
            return 1;
        }

        return 0;
    }
    catch (const std::invalid_argument &ex)
    {
        std::cout << ex.what() << std::endl;
    }
    catch (const std::runtime_error &ex)
    {
        std::cout << ex.what() << std::endl;
    }
}
