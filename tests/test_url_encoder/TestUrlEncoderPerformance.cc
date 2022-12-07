#include <gtest/gtest.h>
#include <string>
#include <chrono>
#include "UrlEncoder.hh"

namespace
{
    TEST(TestUrlEncoderPerformance, OnlyPEN)
    {
        std::cout << "\n[UrlEncoder] Start OnlyPEN performance test\n";
        UrlEncoder encoder;

        std::string temp_str;
        for (uint32_t i = 0; i < 10000; i++)
        {
            temp_str = "https://webex.com<pen=";
            temp_str += std::to_string(i);
            temp_str += ">/meeting<int16>/chat<int16>/user<int16>/clan<int16>";

            encoder.AddTemplate(temp_str);
        }

        encoder.AddTemplate(std::string("https://webex.com<pen=10000>/meeting"
            "<int16>/chat<int16>/user<int16>/clan<int16>"));

        // Test with just PENs
        auto start = std::chrono::steady_clock::now();

        auto encoded = encoder.EncodeUrl(
            "https://webex.com/meeting1/chat1/user1/clan1");

        auto end = std::chrono::steady_clock::now();
        auto res = std::chrono::duration_cast<std::chrono::milliseconds>(
            end-start).count();

        std::cout << "[UrlEncoder] Result: " << encoded.ToHexString() << "\n";
        std::cout << "[UrlEncoder] Elapsed encoding time: " << res << "ms\n";

        std::cout << "[UrlEncoder] Finish OnlyPEN performance test\n\n";
    }

    TEST(TestUrlEncoderPerformance, OnlySubPEN)
    {
        std::cout << "\n[UrlEncoder] Start OnlySubPEN performance test\n";
        UrlEncoder encoder;

        // Add a bunch of sub PENs
        std::string temp_str;
        for (uint32_t i = 0; i < 255; i++)
        {
            temp_str = "https://webex.com<pen=0><sub_pen=";
            temp_str += std::to_string(i);
            temp_str += ">/meeting<int16>/chat<int16>/user<int16>/clan<int16>";

            encoder.AddTemplate(temp_str);
        }

        encoder.AddTemplate(std::string("https://webex.com<pen=0>"
            "<sub_pen=255>/meeting<int16>/chat<int16>/user<int16>/"
            "guild<int16>"));

        // Test with just PENs
        auto start = std::chrono::steady_clock::now();

        auto encoded = encoder.EncodeUrl(
            "https://webex.com/meeting1/chat1/user1/guild1");

        auto end = std::chrono::steady_clock::now();
        auto res = std::chrono::duration_cast<std::chrono::milliseconds>(
            end-start).count();

        std::cout << "[UrlEncoder] Result: " << encoded.ToHexString() << "\n";
        std::cout << "[UrlEncoder] Elapsed encoding time: " << res << "ms\n";

        std::cout << "[UrlEncoder] Finish OnlySubPEN performance test\n\n";
    }

    TEST(TestUrlEncoderPerformance, FullPerformance)
    {
        std::cout << "\n[UrlEncoder] Start Full Performance test\n";
        UrlEncoder encoder;

        // Add a bunch of sub PENs
        std::string temp_str;
        std::string sub_str;
        for (std::uint32_t i = 0; i < 100; i++)
        {
            temp_str = "https://webex.com<pen=";
            temp_str += std::to_string(i);
            temp_str += "><sub_pen=";
            for (std::uint32_t j = 0; j < 256; j++)
            {
                sub_str = std::to_string(j);
                sub_str +=
                    ">/meeting<int16>/chat<int16>/user<int16>/clan<int16>";

                encoder.AddTemplate(temp_str + sub_str);
            }
        }

        encoder.AddTemplate(std::string("https://webex.com<pen=10000>"
            "<sub_pen=255>/meeting<int16>/chat<int16>/user<int16>/"
            "guild<int16>"));

        // Test with just PENs
        auto start = std::chrono::steady_clock::now();

        auto encoded = encoder.EncodeUrl(
            "https://webex.com/meeting1/chat1/user1/guild1");

        auto end = std::chrono::steady_clock::now();
        auto res = std::chrono::duration_cast<std::chrono::milliseconds>(
            end-start).count();

        std::cout << "[UrlEncoder] Result: " << encoded.ToHexString() << "\n";
        std::cout << "[UrlEncoder] Elapsed encoding time: " << res << "ms\n";

        std::cout << "[UrlEncoder] Finish Full Performance test\n\n";
    }
}