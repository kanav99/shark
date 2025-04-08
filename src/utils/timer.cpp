#include <iostream>

#include <shark/utils/timer.hpp>
#include <shark/protocols/common.hpp>

namespace shark {
    namespace utils {
        std::map<std::string, TimerStat> timers;

        void start_timer(const std::string& name)
        {
            timers[name].start_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            if (shark::protocols::peer)
                timers[name].start_comm = shark::protocols::peer->bytesReceived() + shark::protocols::peer->bytesSent();
            else
                timers[name].start_comm = 0;
        }

        void stop_timer(const std::string& name)
        {
            u64 end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            timers[name].accumulated_time += end - timers[name].start_time;
            if (shark::protocols::peer)
                timers[name].accumulated_comm += shark::protocols::peer->bytesReceived() + shark::protocols::peer->bytesSent() - timers[name].start_comm;
            else
                timers[name].accumulated_comm = 0;
        }

        void print_timer(const std::string& name)
        {
            std::cout << name << ": " << timers[name].accumulated_time << " ms, " << (timers[name].accumulated_comm / 1024.0) << " KB"  << std::endl;
        }

        void print_all_timers(const std::string& prefix)
        {
            for (auto& timer : timers)
            {
                if (prefix.empty() || timer.first.find(prefix) == 0)
                {
                    print_timer(timer.first);
                }
            }
        }

        // void __attribute__((destructor)) destruct_timers()
        // {
        //     print_all_timers();
        // }
    }
}