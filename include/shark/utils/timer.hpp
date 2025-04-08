#pragma once

#include <chrono>
#include <map>

#include <shark/types/u128.hpp>

namespace shark {
    namespace utils {
        struct TimerStat
        {
            u64 accumulated_time;
            u64 accumulated_comm;
            u64 start_time;
            u64 start_comm;
        };

        extern std::map<std::string, TimerStat> timers;

        void start_timer(const std::string& name);
        void stop_timer(const std::string& name);
        void print_timer(const std::string& name);
        void print_all_timers(const std::string& name = "");
    }
}