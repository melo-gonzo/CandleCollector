// stock_tracker.cpp
#include "config.h"
#include "types.h"
#include "price_service.h"
#include "market_hours.h"
#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

namespace StockTracker {

class StockTracker {
public:
    static StockTracker& getInstance() {
        static StockTracker instance;
        return instance;
    }

    void begin() {
        setup();
    }

    void run() {
        static unsigned long lastUpdate = 0;
        static bool lastMarketState = false;
        static bool currentlyWaiting = false;
        
        bool isOpen = MarketHoursChecker::isMarketOpen();
        
        // If market state changed, print appropriate message
        if (isOpen != lastMarketState) {
            if (isOpen) {
                Serial.println("Market is now open. Resuming price tracking...");
                currentlyWaiting = false;
            } else {
                Serial.println("Market is now closed.");
                std::string nextOpen = MarketHoursChecker::getNextMarketOpen();
                Serial.printf("Next market open: %s\n", nextOpen.c_str());
                currentlyWaiting = true;
            }
            lastMarketState = isOpen;
        }
        
        if (!isOpen) {
            // Print waiting message periodically
            if (currentlyWaiting && millis() - lastUpdate >= 10000) {  // Every minute
                std::string nextOpen = MarketHoursChecker::getNextMarketOpen();
                Serial.printf("Waiting for market open at: %s\n", nextOpen.c_str());
                lastUpdate = millis();
            }
            delay(1000);  // Longer delay when market is closed
            return;
        }
        
        // Normal price update logic
        if (millis() - lastUpdate >= config.stock.updateIntervalMs) {
            float price = priceService->getPrice();
            
            if (price > 0) {
                candleBuffer.update(price);
                printStatus();
            }
            
            lastUpdate = millis();
        }
        
        delay(100);  // Small delay to prevent overwhelming the CPU
    }

private:
    Config& config;
    CandleBuffer candleBuffer;
    std::unique_ptr<PriceService> priceService;

    // Private constructor for singleton
    StockTracker() 
        : config(Config::getInstance())
        , candleBuffer(config.stock.maxCandles, config.stock.candleDurationSec)
        , priceService(PriceServiceFactory::createService(
            config.stock.useTestData, 
            config.stock.symbol))
    {}

    // Prevent copying
    StockTracker(const StockTracker&) = delete;
    StockTracker& operator=(const StockTracker&) = delete;

    void setup() {
        setupSerial();
        setupWiFi();
        setupTime();
        printInitialStatus();
    }

    void setupSerial() {
        Serial.begin(115200);
    }

    void setupWiFi() {
        WiFi.begin(config.wifi.ssid, config.wifi.password);
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }
        Serial.println("\nWiFi connected");
    }

    void setupTime() {
        configTime(0, 0, config.time.ntpServer1, config.time.ntpServer2);
        setenv("TZ", config.time.timezone, 1);
        tzset();
        
        time_t now = time(nullptr);
        while (now < 8 * 3600 * 2) {
            delay(500);
            now = time(nullptr);
        }
    }

    void printInitialStatus() {
        Serial.printf("\nStarting %s price tracking...\n", 
            config.stock.symbol.c_str());
        Serial.printf("Candle duration: %d seconds\n\n", 
            config.stock.candleDurationSec);
    }

    void printStatus() {
        if (const CandleData* lastCompleted = candleBuffer.getLastCompletedCandle()) {
            Serial.println("Previous Candle:");
            Serial.println(lastCompleted->toString().c_str());
        }
        
        if (const CandleData* current = candleBuffer.getCurrentCandle()) {
            Serial.println("Current Candle:");
            Serial.println(current->toString().c_str());
        }
        
        Serial.println("\n-------------------");
    }
};

} // namespace StockTracker

// Global instance pointer
static StockTracker::StockTracker* tracker = nullptr;

void setup() {
    // Initialize the tracker
    tracker = &StockTracker::StockTracker::getInstance();
    tracker->begin();
}

void loop() {
    // Run the tracker
    if (tracker) {
        tracker->run();
    }
}