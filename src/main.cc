#include <iostream>
#include <string>
#include "UrlTemplater.hh"
#include "HttpEncoder.hh"
#include "uint128.hh"

int main()
{
    try
    {
        // Filename should be an argument
        // filename
        std::string filename = "./files/templates.json";

        // Templater
        UrlTemplater templater(filename);
        templater.Add("https://webex.com/<int24=1>/meeting<int16>/user<int16>");

        auto x = templater.GetTemplates();
        std::cout << "url " << x[1].url
                  << " bits size " << x[1].bits.size()
                  << " " << x[1].bits[0]
                  << " " << x[1].bits[1]
                  << " " << x[1].bits[2]
                  << std::endl;

        // Get the url
        std::string url = "https://webex.com/11259375/meeting1234/user3213";

        // // Encode
        HttpEncoder encoder;
        uint128 encoded = encoder.EncodeUrl(url, templater.GetTemplates());
        std::cout << "Encoded size - "
                  << encoded.ToDecimalString().size()
                  << "\nEncoded val - "
                  << encoded.ToDecimalString()
                  << "\nEncoded binary - "
                  << encoded.ToBinaryString(' ')
                  << "\n";

        // // Decode
        std::string decoded = encoder.DecodeUrl(encoded.ToDecimalString(),
                                                templater.GetTemplates());
        std::cout << "Decoded " << decoded << "\n";
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