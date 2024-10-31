// config.h
#pragma once

#include <string>

// Check if credentials file exists
#if __has_include("credentials.h")
    #include "credentials.h"
#else
    #error "Please copy credentials_template.h to credentials.h and fill in your WiFi credentials"
#endif

namespace StockTracker {

struct WiFiConfig {
    const char* ssid;
    const char* password;
};

struct TimeConfig {
    const char* timezone;
    const char* ntpServer1;
    const char* ntpServer2;
};

struct MarketHours {
    int startHour;
    int startMinute;
    int endHour;
    int endMinute;
    bool enforceHours;
};

struct StockConfig {
    std::string symbol;
    bool useTestData;
    unsigned long updateIntervalMs;
    unsigned int candleDurationSec;
    unsigned int maxCandles;
    MarketHours marketHours;
};

class Config {
public:
    static Config& getInstance() {
        static Config instance;
        return instance;
    }

    // Default configuration
    WiFiConfig wifi = {
        .ssid = Credentials::WIFI_SSID,
        .password = Credentials::WIFI_PASSWORD
    };

    TimeConfig time = {
        .timezone = "PST8PDT",
        .ntpServer1 = "pool.ntp.org",
        .ntpServer2 = "time.nist.gov"
    };

    StockConfig stock = {
        .symbol = "SPY",
        .useTestData = true,
        .updateIntervalMs = 5000,
        .candleDurationSec = 60,
        .maxCandles = 20,
        .marketHours = {
            .startHour = 6,
            .startMinute = 30,
            .endHour = 13,
            .endMinute = 0,
            .enforceHours = true
        }
    };

private:
    Config() {} // Private constructor for singleton
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
};

} // namespace StockTracker