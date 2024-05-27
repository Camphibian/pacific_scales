#include <iostream>
#include <stdexcept>

#include <cstring>
#include <fcntl.h>      // Contains file controls like O_RDWR
#include <errno.h>      // Error integer and strerror() function
#include <termios.h>    // Contains POSIX terminal control definitions
#include <unistd.h>     // write(), read(), close()

#include "reporting_service.h"
#include "parse.h"
#include "../imports/json/single_include/nlohmann/json.hpp"

namespace scales_client {
using namespace std::chrono;
using json = nlohmann::json;
using parser = string_tools::parser;

#define USES_WAIT_UNTIL
/**
 * @brief Reporting Service
 * Opens designated com port
 */
reporting_service::reporting_service() :
    exit_flag(false),
    measure_index(0) {
	//std::cout << "reporting_service::reporting_service()" << std::endl;

    tReporter = new std::thread(
        [&](reporting_service* rs) {
	        rs->reporter();
        },
        this);
}

/**
 * @brief cleanup
 */
reporting_service::~reporting_service() {
	std::cout << "reporting_service::~reporting_service()" << std::endl;
    exit_flag = true;
}

/**
 * @brief Constructs comm port and then listens for messages until app quit. 
 * Packets are assembled from fragments according to the packet format. 
 * Once a packet has been assembled it gets passed off to the JSON stage.
 * 
 * @throws serial exceptions
*/
int reporting_service::listener(std::string port) {
    //std::cout << "reporting_service::listener - Connecting to port '" << port << "'..." << std::endl;

    // create packetiser state machine
    using packetiser_function = std::function<pk_state(std::string& out_buffer__, char* raw_buffer__, int& length__, int& offset__)>;
    static std::map<pk_state, packetiser_function> packetise = {
        {pk_state::start,
            [this](std::string& out_buffer__, char* raw_buffer__, int& length__, int& offset__) {
                //return await_start_token(raw_buffer__, length__, offset__);
                //std::cout << "reporting_service::listener - waiting start " << length__ << " " << offset__ << std::endl;
                while (offset__ < length__)
                    if (raw_buffer__[offset__++] == '/') {
                        out_buffer__ = "";
                        //std::cout << "reporting_service::listener - found start " << offset__ << std::endl;
                        return pk_state::payload;
                    }
                //std::cout << "reporting_service::listener - get next buffer " << offset__ << std::endl;
                length__ -= offset__;
                offset__ = 0;
                return pk_state::start;
            }},
        {pk_state::payload,
            [this](std::string& out_buffer__, char* raw_buffer__, int& length__, int& offset__) {
                //return assemble_packet(raw_buffer__, length__, offset__);
                //std::cout << "reporting_service::listener - waiting end " << length__ << " " << offset__ << std::endl;
                while (offset__ < length__) {
                    //std::cout << "reporting_service::listener '" << raw_buffer__[offset__] << "'" << std::endl;
                    out_buffer__ += raw_buffer__[offset__];
                    if (raw_buffer__[offset__++] == '\\') {
                        out_buffer__.pop_back();
                        //std::cout << "reporting_service::listener - found end" << std::endl;
                        process_packet(out_buffer__);
                        return pk_state::start;
                    }
                }
                length__ -= offset__;
                offset__ = 0;
                return pk_state::payload;
            }}
    };

    int port_handle = configure_port(port);

    char read_buf [256];
    std::string transfer_buffer;
    pk_state packetiser_state = pk_state::start;
    while (packet_count < 5) {
        try {
            //std::cout << "reporting_service::listener - Polling" << std::endl;
            // assemble the packet from chunks read in
            // "/\r\nA    :   7000 Kg\r\nB    :  19000 Kg\r\nC    :  28000 Kg\r\nD    :  17000 Kg\r\nTOTAL:  71000 Kg\r\n\\\r\n"
            // start of message is '/' followed by a CR/LF
            // ended of message is '\' followed by a CR/LF
            int raw_index = 0;
            int num_bytes = read(port_handle, &read_buf, sizeof(read_buf));
            while (num_bytes > 0) {
                packetiser_state = packetise[packetiser_state](transfer_buffer, read_buf, num_bytes, raw_index);
            }
        } catch(...) {
            return -1;
        }
    }
    return 0;
}

/**
 * @brief Configure the serial port using termios
*/
int reporting_service::configure_port(std::string port) {
    int port_handle = open(port.c_str(), O_RDONLY);

    // Check for errors
    if (port_handle < 0) {
        throw new std::runtime_error("COM fault ");// + std::string(strerror(errno)));
    }
    //std::cout << "Connected!" << std::endl;

    struct termios tty;

    if(tcgetattr(port_handle, &tty) != 0) {
        std::cout << "reporting_service::configure_port - ERROR tcgetattr(" << errno << ") " << strerror(errno) << std::endl;
        throw new std::runtime_error("tcgetattr error ");// + std::string(strerror(errno)));
    }

    // wire format
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag |= CREAD | CLOCAL;

    // flow control & teminal settings
    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;
    tty.c_lflag &= ~ECHOE;
    tty.c_lflag &= ~ECHONL;
    tty.c_lflag &= ~ISIG;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);
    tty.c_oflag &= ~OPOST;
    tty.c_oflag &= ~ONLCR;

    // read/write timeouts set for single character interrupts. Probably very inefficient
    tty.c_cc[VTIME] = 0;
    tty.c_cc[VMIN] = 1;

    // Baud rate
    cfsetispeed(&tty, B2400);
    cfsetospeed(&tty, B2400);

    if (tcsetattr(port_handle, TCSANOW, &tty) != 0) {
        std::cout << "reporting_service::configure_port - ERROR tcsetattr(" << errno << ") " << strerror(errno) << std::endl;
        throw new std::runtime_error("tcgetattr error ");// + std::string(strerror(errno)));
    }
    return port_handle;
}

/**
 * @brief filter and parse input packet into JSON object
 * eg: {"weights":{"A":5800,"B":17800,"C":22800,"D":15800,"TOTAL":62200,"VALID":true}}
 * Invalid fields result in a discarded packet, corrected for peculiar negative notation
 * Update latest weights measure for reporting using double buffer scheme.
*/
int reporting_service::process_packet(std::string packet) {
    //std::cout << "reporting_service::process_packet - PACKET='" << packet << "'" << std::endl;

    std::string current_measure = "{\"weights\":{";
    long total_value = 0;
    bool valid = false;
    auto lines = string_tools::parser::split(packet, "\r\n");
    for (auto line : lines) {
        if (line == "")
            continue;
        //std::cout << "'" << line << "'" << std::endl;
        auto nvp = string_tools::parser::split(line, ":");
        auto channel = parser::trim(nvp[0]);
        auto vtp = parser::split(parser::trim(nvp[1]), " ");
        auto value = parser::trim(vtp[0]);
        auto units = parser::trim(vtp[1]);
        //std::cout << channel << " : " << value << " " << units << std::endl;

        // value is peculiar when negative. We will address this here
        value = parser::fix_negative(value);

        // generate JSON string
        std::string channel_measure = "\"" + channel + "\":" + value + "";
        //std::cout << channel_measure << std::endl;

        current_measure += channel_measure;
        current_measure += ",";

        // do the tally here
        auto field_value = std::stol(value);
        if(channel != "TOTAL") {
            total_value += field_value;
        } else {
            valid = (total_value == field_value);
        }
    }
    std::string valid_measure;
    valid_measure += "\"VALID\":";
    valid_measure += ((valid) ? "true" : "false");

    current_measure += valid_measure;
    current_measure += "}}";

    //std::cout << current_measure << std::endl;
    save_latest_measure(current_measure);
    //packet_count++;
    // try {
    //     json j =json::parse(current_measure);
    //     std::cout << j << std::endl;
    //     //measure[measure_index] = current_measure;
    // } catch (...) {
    //     std::cout << "reporting_service::process_packet EXCEPTION - bad JSON" << std::endl;
    // }
    return 0;
}

/**
 * @brief Dumps latest measure to console
 * Blocks while latest measure is being updated
 */
void reporting_service::dump_latest_measure() {
    //std::cout << "reporting_service::dump_latest_measure() - WAITING" << std::endl;
    std::unique_lock<std::mutex> sync_lock(measure_lock, std::adopt_lock);
    int latest_measure = (measure_index - 1) % measure_pool;
    if (measure[latest_measure] != "")
        std::cout << measure[latest_measure] << std::endl;
    //std::cout << "reporting_service::dump_latest_measure() - COMPLETED" << std::endl;
}

/**
 * @brief saves latest received measure into the active buffer
 * Blocks while latest measure is being updated
 */
void reporting_service::save_latest_measure(std::string sample) {
    //std::cout << "reporting_service::save_latest_measure() - WAITING" << std::endl;
    std::unique_lock<std::mutex> sync_lock(measure_lock, std::adopt_lock);
    measure[measure_index] = sample;    // FIXME: inefficient additional copy
    measure_index = (measure_index + 1) % measure_pool;
    //std::cout << "reporting_service::save_latest_measure() - COMPLETED" << std::endl;
}
/**
 * @brief Threaded reporter synched to 12:00:00 at 10 second intervals. 
 * Content is prepared by process packet which generates a JSON-esque blob. 
 * There are two blob buffers, Active and Sample.
 * Active buffer is what is reported on std out
 * Sample buffer is what gathers the latest valid measure
 * 
*/
int reporting_service::reporter() {
    //std::cout << "reporting_service::reporter() - LAUNCHED" << std::endl;

    // calculate initial wait period  synchronise to clock time 12:00:00 in 10 second intervals
    auto const now = std::chrono::system_clock::now();
    std::time_t tnow = std::chrono::system_clock::to_time_t(now);
    std::time_t delta = 10 - (tnow + 10) % 10;
    // // show the current system time
    // std::cout << std::ctime(&tnow);

#ifdef USES_WAIT_UNTIL
    auto time = system_clock::time_point(duration_cast<milliseconds>(now.time_since_epoch()) + milliseconds(delta*1000));
    // // show the intended awaken time
    // std::time_t ttime = std::chrono::system_clock::to_time_t(time);
    // std::cout << std::ctime(&ttime);
#endif
    //std::chrono::high_resolution_clock::time_point tik = now;

    while (!exit_flag) {
#ifdef USES_WAIT_UNTIL
        std::unique_lock<std::mutex> lock(idle_lock, std::adopt_lock);
        cv.wait_until(lock, time, [this]{return false;});
#else
        std::this_thread::sleep_for(std::chrono::seconds(delta));
        delta = 10;
#endif
        auto const now = std::chrono::system_clock::now();
        // // show the time actually activated
        // tnow = std::chrono::system_clock::to_time_t(now);
        // std::cout << std::ctime(&tnow);

#ifdef USES_WAIT_UNTIL
        time = system_clock::time_point(duration_cast<milliseconds>(now.time_since_epoch()) + milliseconds(10000));
        // // show the activation time
        // ttime = std::chrono::system_clock::to_time_t(time);
        // std::cout << std::ctime(&ttime);
#endif

        // // we see here that the dump time lags by about 65 microseconds each time when using simple wait
		// duration<double> time_span = duration_cast<duration<double>>(now - tik);
		// std::cout << std::fixed << std::setprecision(9) << time_span.count() << std::endl << std::flush;

        dump_latest_measure();
    }
    //std::cout << "reporting_service::reporter() - TERMINATEDED" << std::endl;
    return 0;
}

} // namespace scales_client