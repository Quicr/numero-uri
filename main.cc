#include <iostream>
#include <string>
#include "HttpEncoder.hh"

int main()
{
    try
    {
        // Get the url
        std::string url = "https://webex.com/19/meeting1234/user3213";

        // Encode
        std::string encoded = HttpEncoder::EncodeUrl(url);
        std::cout << "Encoded " << encoded << "\n";

        // Decode
        std::string decoded = HttpEncoder::DecodeUrl(encoded);
        std::cout << "Decoded " << decoded << "\n";
    }
    catch (const char* ex)
    {
        std::cout << ex << std::endl;
    }
}