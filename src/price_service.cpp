// price_service.cpp
#include "price_service.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

namespace StockTracker {

YahooFinancePriceService::YahooFinancePriceService(const std::string &symbol)
    : symbol(symbol) {}

float YahooFinancePriceService::getPrice() {
  HTTPClient http;
  String url = "https://query1.finance.yahoo.com/v8/finance/chart/" +
               String(symbol.c_str()) + "?interval=1d&range=1d";
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

/**
 * Generates a simulated price using a random walk algorithm.
 *
 * The price follows a random walk pattern where:
 * 1. Random change is generated between -2.0 and +2.0
 * 2. Change is applied to last price
 * 3. Price is prevented from going negative
 *
 * Formula: change = (random(-100,101) / 100.0) * 2.0
 * - Random value between -100 and 100, divided by 100 gives -1.0 to 1.0
 * - Multiplied by 2.0 gives final range of -2.0 to +2.0
 *
 * @return A simulated stock price that randomly walks from the previous price,
 *         with a minimum value of 0.0
 */
float TestPriceService::getPrice() {
  float change = (random(-100, 101) / 100.0f) * 2.0f;
  lastPrice += change;
  lastPrice = std::max(0.0f, lastPrice);
  return lastPrice;
}

std::unique_ptr<PriceService>
PriceServiceFactory::createService(bool useTestData,
                                   const std::string &symbol) {
  if (useTestData) {
    // Replace std::make_unique with new
    return std::unique_ptr<PriceService>(new TestPriceService());
  }
  return std::unique_ptr<PriceService>(new YahooFinancePriceService(symbol));
}

} // namespace StockTracker