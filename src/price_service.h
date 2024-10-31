// price_service.h
#pragma once

#include <string>
#include <memory>

namespace StockTracker {

class PriceService {
public:
    virtual ~PriceService() = default;
    virtual float getPrice() = 0;
};

class YahooFinancePriceService : public PriceService {
public:
    explicit YahooFinancePriceService(const std::string& symbol);
    float getPrice() override;

private:
    std::string symbol;
};

class TestPriceService : public PriceService {
public:
    float getPrice() override;

private:
    float lastPrice = 50.0f;
};

class PriceServiceFactory {
public:
    static std::unique_ptr<PriceService> createService(bool useTestData, const std::string& symbol);
};

} // namespace StockTracker