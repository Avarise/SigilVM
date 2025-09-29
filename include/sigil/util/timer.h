#pragma once

#include <chrono>

namespace sigil::util {

long long unix_time();

struct timer_t {
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

private:
    TimePoint start_time{};
    TimePoint end_time{};
    bool running{false};

public:
    inline void start() noexcept {
        start_time = Clock::now();
        running = true;
    }

    inline void stop() noexcept {
        if (running) {
            end_time = Clock::now();
            running = false;
        }
    }

    inline void reset() noexcept {
        running = false;
        start_time = TimePoint{};
        end_time = TimePoint{};
    }

    inline double elapsed_seconds() const noexcept {
        const TimePoint current = running ? Clock::now() : end_time;
        return std::chrono::duration<double>(current - start_time).count();
    }

    inline double elapsed_milliseconds() const noexcept {
        const TimePoint current = running ? Clock::now() : end_time;
        return std::chrono::duration<double, std::milli>(current - start_time).count();
    }

    inline double elapsed_microseconds() const noexcept {
        const TimePoint current = running ? Clock::now() : end_time;
        return std::chrono::duration<double, std::micro>(current - start_time).count();
    }
};



} // namespace sigil
