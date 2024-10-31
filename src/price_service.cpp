// price_service.cpp
#include "price_service.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>

namespace StockTracker {

YahooFinancePriceService::YahooFinancePriceService(const std::string& symbol)
    : symbol(symbol)
{}

float YahooFinancePriceService::getPrice() {
    HTTPClient http;
    String url = "https://query1.finance.yahoo.com/v8/finance/chart/" + 
                 String(symbol.c_str()) + 
                 "?interval=1d&range=1d";
    http.begin(url);
    
    float price = 0.0;
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        StaticJsonDocument<1024> doc;
        
        DeserializationError error = deserializeJson(doc, payload);
        if (!error) {
            JsonObject chart = doc["chart"]["result"][0];
            JsonObject quote = chart["indicators"]["quote"][0];
            price = quote["close"][0];
        }
    }
    
    http.end();
    return price;
}

float TestPriceService::getPrice() {
    float change = (random(-100, 101) / 100.0f) * 2.0f;
    lastPrice += change;
    lastPrice = std::max(0.0f, lastPrice);
    return lastPrice;
}

std::unique_ptr<PriceService> PriceServiceFactory::createService(
    bool useTestData, 
    const std::string& symbol) 
{
    if (useTestData) {
        // Replace std::make_unique with new
        return std::unique_ptr<PriceService>(new TestPriceService());
    }
    return std::unique_ptr<PriceService>(new YahooFinancePriceService(symbol));
}

} // namespace StockTracker