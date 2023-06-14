#include <UrlEncoder.h>
#include "ConfigurationManager.hh"
#include "TemplateFileManager.hh"

#include <nlohmann/json.hpp>

#include <iostream>
#include <string>

using json = nlohmann::json;

int main(int argc, char** argv) try
{
    // json d = TemplateFileManager::LoadTemplatesFromHttp("https://raw.githubusercontent.com/BrettRegnier/delete_me/main/templates.json");
    // std::cout << std::setw(4) << d << std::endl;
    if (argc == 1)
    {
        std::cout << "Requires at least 1 argument\n";
        return 1;
    }

    UrlEncoder encoder;

    // TODO add optional overwrite flag to options parser when it becomes
    // available

    // Get the template file from the configuration file
    std::string template_file = ConfigurationManager::GetTemplateFilePath();

    json data = TemplateFileManager::LoadTemplatesFromFile(template_file);
    encoder.TemplatesFromJson(data);
    if (strcmp(argv[1], "encode") == 0)
    {
        // encode test - https://webex.com/1/meeting1234/user3213
        quicr::Namespace encoded = encoder.EncodeUrl(argv[2]);
        std::cout << encoded.to_hex() << "\n";
    }
    else if (strcmp(argv[1], "decode") == 0)
    {
        //decode test - 0000000112021553484800000000000000000000
        std::string decoded = encoder.DecodeUrl(argv[2]);
        std::cout << decoded << "\n";
    }
    else if (strcmp(argv[1], "template-file") == 0)
    {
        ConfigurationManager::UpdateConfigFile(argv[1], argv[2]);
        std::string f_path = ConfigurationManager::GetTemplateFilePath();
        std::cout << "Updated template file location\n";
    }
    else if (strcmp(argv[1], "template-http") == 0)
    {
        // Get templates from http using curl
        data = TemplateFileManager::LoadTemplatesFromHttp(argv[2]);
        encoder.AddTemplate(data);
        data = encoder.TemplatesToJson();
        TemplateFileManager::SaveTemplates(template_file, data);
        std::cout << "Added templates from http" << std::endl;
    }
    else if (strcmp(argv[1], "add-template") == 0)
    {
        encoder.AddTemplate(std::string(argv[2]));

        // Save the template file
        data = encoder.TemplatesToJson();
        TemplateFileManager::SaveTemplates(template_file, data);
        std::cout << "Added template.\n";
    }
    else if (strcmp(argv[1], "remove-template") == 0)
    {
        std::uint64_t pen = std::stoull(argv[2]);
        if (!encoder.RemoveTemplate(pen))
            throw UrlEncoderException("Failed to remove template from list");

        // Save the template file
        TemplateFileManager::SaveTemplates(template_file,
                                            encoder.TemplatesToJson());
        std::cout << "Template with PEN " << pen << " has been removed.\n";
    }
    else if (strcmp(argv[1], "show-templates") == 0)
    {
        std::cout << std::setw(4) << encoder.TemplatesToJson() << std::endl;
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
