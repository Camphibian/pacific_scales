
#ifndef CD_REPORTING_SERVICE_H
#define CD_REPORTING_SERVICE_H

#include <string>
#include <vector>
#include <list>
#include <map>
#include <functional>
#include <memory>
#include <thread>
#include <chrono>
#include <atomic>
#include <condition_variable>
#include <mutex>

namespace scales_client {

/**
 * @brief 
 */
struct reporting_service {
using daemon_clock = std::chrono::high_resolution_clock;
// using scheduled_timepoint = system_clock::time_point;
// using scheduled_interval = std::chrono::duration<unsigned long, std::milli>;

    reporting_service(/*std::string port_*//*callback_function callback_ = nullptr*/);
    ~reporting_service();

    int listener(std::string port);

    private:
    enum struct pk_state {
        start = 0,
        payload
    };

    int configure_port(std::string port);
    int process_packet(std::string packet);
    
    std::atomic_bool exit_flag;
    int measure_index;      // indexes which buffer is being written
    int packet_count = 0;   // test finite looping


    static constexpr size_t measure_pool = 2;
    std::string measure[measure_pool];
    std::mutex measure_lock;
    std::mutex idle_lock;
    std::condition_variable cv;

    void dump_latest_measure();
    void save_latest_measure(std::string sample);

    std::thread* tReporter;
    int reporter();
};

}   //namespace scales_client
#endif //CD_REPORTING_SERVICE_H
