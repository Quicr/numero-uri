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
            std::string config_file_path = ConfigurationManager::PathToExecutable() + "../config";
        if (argc == 1)
        {
            std::cout << "Requires at least 1 argument\n";
            return 1;
        }

        HttpEncoder encoder;
        // TODO Filename should be an CLI argument
        // Get the config file and read the last file location?
        std::string filename = "./files/templates.json";
        // Templater
        UrlTemplater templater(filename);

        if (strcmp(argv[1], "encode") == 0)
        {
            // encode test - https://webex.com/1/meeting1234/user3213
            uint128 encoded = encoder.EncodeUrl(argv[2], templater.GetTemplates());
            std::cout << encoded.ToDecimalString() << "\n";
            return 0;
        }
        else if (strcmp(argv[1], "decode") == 0)
        {
            //decode test - 0000000112021553484800000000000000000000
            std::string decoded = encoder.DecodeUrl("0000000112021553484800000000000000000000", templater.GetTemplates());
            std::cout << decoded << "\n";
            return 0;
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
            return 0;
        }
        else if (strcmp(argv[1], "add-template") == 0)
        {

        }
        else if (strcmp(argv[1], "remove-template") == 0)
        {

        }
        else
        {
            // Print help statement..?
            std::cout << "Argument requires at least 1 argument\n";
            return 1;
        }

        // templater.SaveTemplates("test.json");
        // auto x = templater.GetTemplates();
        // std::cout << "url " << x[1].url
        //           << " bits size " << x[1].bits.size()
        //           << " " << x[1].bits[0]
        //           << " " << x[1].bits[1]
        //           << " " << x[1].bits[2]
        //           << std::endl;

        // // Get the url
        // std::string url = "https://webex.com/1/meeting1234/user3213";

        // // Encode

        // // Decode
        // std::string decoded = encoder.DecodeUrl(encoded.ToDecimalString(),
        //                                         templater.GetTemplates());
        // std::cout << "Decoded " << decoded << "\n";
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