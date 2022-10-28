#include <iostream>
#include <string>
#include "HttpEncoder.hh"
#include "uint128.hh"

int main()
{
    try
    {
        // Get the url
        std::string url = "https://webex.com/11259375/meeting1234/user3213";

        // Encode
        uint128 encoded = HttpEncoder::EncodeUrl(url);
        std::cout << "Encoded " << encoded.ToString(' ') << "\n";

        // Decode
        std::string decoded = HttpEncoder::DecodeUrl(encoded);
        std::cout << "Decoded " << decoded << "\n";
    }
    catch (const std::string &ex)
    {
        std::cout << ex << std::endl;
    }
}