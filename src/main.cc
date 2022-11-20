#include <iostream>
#include <string>
#include "UrlTemplater.hh"
#include "HttpEncoder.hh"
#include "uint128.hh"
#include "ConfigurationManager.hh"

int main(int argc, char** argv)
{

    try
    {
        if (argc == 1)
        {
            std::cout << "Requires at least 1 argument\n";
            return 1;
        }

        // Get the template file from the configuration file
        std::string template_file = ConfigurationManager::GetTemplateFilePath();

        // Templater
        UrlTemplater templater(template_file);

        HttpEncoder encoder;
        if (strcmp(argv[1], "encode") == 0)
        {
            // encode test - https://webex.com/1/meeting1234/user3213
            uint128 encoded = encoder.EncodeUrl(argv[2], templater.GetTemplates());
            std::cout << encoded.ToDecimalString() << "\n";
        }
        else if (strcmp(argv[1], "decode") == 0)
        {
            //decode test - 0000000112021553484800000000000000000000
            std::string decoded = encoder.DecodeUrl(argv[2], templater.GetTemplates());
            std::cout << decoded << "\n";
        }
        else if (strcmp(argv[1], "config") == 0)
        {
            ConfigurationManager::UpdateConfigFile(argv[2], argv[3]);
            if (strcmp(argv[2], "template-file") == 0)
            {
                std::string f_path = ConfigurationManager::GetTemplateFilePath();
                templater.Clear();
                templater.LoadTemplates(f_path);
                std::cout << "Updated template file location\n";
            }
        }
        else if (strcmp(argv[1], "add-template") == 0)
        {
            if (!templater.Add(argv[2]))
                throw UrlTemplaterException("Failed to add template to list");

            // Save the template file
            templater.SaveTemplates(template_file);
            std::cout << "Added template.\n";
        }
        else if (strcmp(argv[1], "remove-template") == 0)
        {
            std::uint64_t pen = std::stoull(argv[2]);
            if (!templater.Remove(pen))
                throw UrlTemplaterException("Failed to remove template from list");

            // Save the template file
            templater.SaveTemplates(template_file);
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
