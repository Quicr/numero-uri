#include <iostream>
#include <string>
#include "HttpEncoder.hh"
#include "uint128.hh"

int main()
{
    try
    {
        // filename
        std::string filename = "./files/templates.json";
        // Get the url
        std::string url = "https://webex.com/1/123/meeting1234/user3213";

        // Encode
        HttpEncoder encoder;
        uint128 encoded = encoder.EncodeUrl(url);
        std::cout << "Encoded size - "
                  << encoded.ToDecimalString().size()
                  << "\nEncoded val - "
                  << encoded.ToDecimalString()
                  << "\nEncoded binary - "
                  << encoded.ToBinaryString(' ')
                  << "\n";

        // Decode
        std::string decoded = encoder.DecodeUrl(encoded.ToDecimalString());
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