// market_hours.h
#pragma once

#include <time.h>
#include "config.h"

namespace StockTracker {

class MarketHoursChecker {
public:
    static bool isMarketOpen() {
        const auto& config = Config::getInstance();
        
        // If using test data or market hours are not enforced, always return true
        if (config.stock.useTestData || !config.stock.marketHours.enforceHours) {
            return true;
        }

        time_t now;
        time(&now);
        struct tm timeinfo;
        localtime_r(&now, &timeinfo);

        // Check if it's weekend
        if (timeinfo.tm_wday == 0 || timeinfo.tm_wday == 6) {
            return false;
        }

        // Convert current time to minutes since midnight
        int currentMinutes = timeinfo.tm_hour * 60 + timeinfo.tm_min;

        // Convert market hours to minutes since midnight
        const auto& market = config.stock.marketHours;
        int marketStartMinutes = market.startHour * 60 + market.startMinute;
        int marketEndMinutes = market.endHour * 60 + market.endMinute;

        return currentMinutes >= marketStartMinutes && currentMinutes < marketEndMinutes;
    }

    static std::string getNextMarketOpen() {
        const auto& config = Config::getInstance();
        const auto& market = config.stock.marketHours;

        time_t now;
        time(&now);
        struct tm timeinfo;
        localtime_r(&now, &timeinfo);

        // Set time to next market open
        timeinfo.tm_hour = market.startHour;
        timeinfo.tm_min = market.startMinute;
        timeinfo.tm_sec = 0;

        // If we're past today's market open, move to next day
        if (!isMarketOpen() && timeinfo.tm_hour * 60 + timeinfo.tm_min < 
            market.endHour * 60 + market.endMinute) {
            timeinfo.tm_mday += 1;
        }

        // Skip weekends
        while (timeinfo.tm_wday == 0 || timeinfo.tm_wday == 6) {
            timeinfo.tm_mday += 1;
            mktime(&timeinfo);  // Normalize date and get proper day of week
        }

        char buffer[30];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S %Z", &timeinfo);
        return std::string(buffer);
    }
};

} // namespace StockTracker