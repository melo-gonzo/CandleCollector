// types.h
#pragma once

#include <ctime>
#include <vector>
#include <memory>

namespace StockTracker {

class CandleData {
public:
    CandleData(float openPrice, time_t startTime);

    void update(float price);
    void complete();
    
    float getOpen() const { return open; }
    float getHigh() const { return high; }
    float getLow() const { return low; }
    float getClose() const { return close; }
    time_t getTimestamp() const { return timestamp; }
    bool isCompleted() const { return completed; }
    time_t getEndTime() const { return endTime; }

    std::string toString() const;

private:
    float open;
    float high;
    float low;
    float close;
    time_t timestamp;  // start time
    time_t endTime;    // end time
    bool completed;
};

class CandleBuffer {
public:
    CandleBuffer(size_t maxSize, unsigned int candleDurationSec);
    
    void update(float price);
    const CandleData* getCurrentCandle() const;
    const CandleData* getLastCompletedCandle() const;
    size_t size() const { return candles.size(); }

private:
    std::vector<std::unique_ptr<CandleData>> candles;
    size_t maxSize;
    unsigned int candleDuration;

    time_t getNextCandleStart(time_t currentTime) const;
    bool shouldCreateNewCandle(time_t currentTime) const;
    static time_t alignToInterval(time_t t, unsigned int intervalSeconds);
};

} // namespace StockTracker