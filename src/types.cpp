// types.cpp
#include "types.h"
#include "config.h"
#include <Arduino.h>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace StockTracker {

CandleData::CandleData(float openPrice, time_t startTime)
    : open(openPrice)
    , high(openPrice)
    , low(openPrice)
    , close(openPrice)
    , timestamp(startTime)
    , endTime(startTime + Config::getInstance().stock.candleDurationSec)
    , completed(false)
{}

void CandleData::update(float price) {
    high = std::max(high, price);
    low = std::min(low, price);
    close = price;
}

void CandleData::complete() {
    completed = true;
}

std::string CandleData::toString() const {
    std::stringstream ss;
    struct tm timeinfo;
    time_t now;
    time(&now);
    
    // Format start time
    localtime_r(&timestamp, &timeinfo);
    char startTimeStr[20];
    strftime(startTimeStr, sizeof(startTimeStr), "%H:%M:%S", &timeinfo);
    
    // Format current time
    localtime_r(&now, &timeinfo);
    char currentTimeStr[20];
    strftime(currentTimeStr, sizeof(currentTimeStr), "%H:%M:%S", &timeinfo);
    
    // Format end time
    localtime_r(&endTime, &timeinfo);
    char endTimeStr[20];
    strftime(endTimeStr, sizeof(endTimeStr), "%H:%M:%S", &timeinfo);
    
    ss << "[" << startTimeStr << " - " << currentTimeStr << " - " << endTimeStr << "]:\n"
       << "  Open:  $" << std::fixed << std::setprecision(2) << open << "\n"
       << "  High:  $" << high << "\n"
       << "  Low:   $" << low << "\n"
       << "  Close: $" << close << "\n"
       << "  Status: " << (completed ? "Completed" : "In Progress");
    
    return ss.str();
}

CandleBuffer::CandleBuffer(size_t maxSize, unsigned int candleDurationSec) 
    : maxSize(maxSize)
    , candleDuration(candleDurationSec)
{}

time_t CandleBuffer::alignToInterval(time_t t, unsigned int intervalSeconds) {
    return (t / intervalSeconds) * intervalSeconds;
}

time_t CandleBuffer::getNextCandleStart(time_t currentTime) const {
    time_t alignedCurrent = alignToInterval(currentTime, candleDuration);
    
    // If we're exactly on an interval, use that time
    if (alignedCurrent == currentTime) {
        return currentTime;
    }
    
    // Otherwise, return the next interval
    return alignedCurrent + candleDuration;
}

bool CandleBuffer::shouldCreateNewCandle(time_t currentTime) const {
    if (candles.empty()) {
        return true;
    }
    
    const CandleData* currentCandle = getCurrentCandle();
    if (!currentCandle) {
        return true;
    }

    // Check if current time has moved past the end time of the current candle
    return currentTime >= currentCandle->getEndTime();
}

void CandleBuffer::update(float price) {
    time_t now;
    time(&now);
    
    if (candles.empty() || shouldCreateNewCandle(now)) {
        if (!candles.empty()) {
            candles.back()->complete();
        }
        
        if (candles.size() >= maxSize) {
            candles.erase(candles.begin());
        }

        // For the first candle, align to the next interval
        time_t startTime = candles.empty() ? 
            alignToInterval(now, candleDuration) : 
            candles.back()->getEndTime();

        candles.push_back(std::unique_ptr<CandleData>(new CandleData(price, startTime)));
        
        Serial.printf("New candle started at: %ld (aligned from %ld)\n", 
                     startTime, now);
    } else {
        candles.back()->update(price);
    }
}

const CandleData* CandleBuffer::getCurrentCandle() const {
    return candles.empty() ? nullptr : candles.back().get();
}

const CandleData* CandleBuffer::getLastCompletedCandle() const {
    return (candles.size() < 2) ? nullptr : candles[candles.size() - 2].get();
}

} // namespace StockTracker