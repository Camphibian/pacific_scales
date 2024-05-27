#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <stdexcept>

#include "reporting_service.h"

std::string exec(char* cmd);

/**
 * @brief Pacific Scales Client reporting V1.0
 * 
 * Entry point into the client reporting service. 
 * 
 * Command line args:
 * Usage:
 * -h           List all commands
 * -l           List serial ports by name, case sensitive
 * -p <name>    Connects to named port and launches service. ^C to terminate service.
 * 
 * ex:
 * psclient -h
 * psclient -l
 * psclient -c /dev/ttyUSB0
 * 
 */
int main(int argc, char** argv) {
    std::cout << "Pacific Scales Client reporting V1.0" << std::endl;

    if (argc <= 1) {
        std::cout << "use -h for list of commands." << std::endl;
        exit(0);
    }

    if (std::string(argv[1]).compare("-h") == 0) {
        std::cout << "Usage:" << std::endl
        << "-h           List all commands" << std::endl
        << "-l           List serial ports by name, case sensitive." << std::endl
        << "-p <name>    Connects to named port and launches service. ^C to terminate service." << std::endl;

    } else if (std::string(argv[1]).compare("-l") == 0) {
        if (argc != 2) {
            std::cout << "Invalid arguments" << std::endl;
            exit(0);
        }
        std::cout << "listing comm ports:" << std::endl;
        // valid serial ports are enumerated through /dev/ttyS* and /dev/ttyUSB* (Linux)
        std::cout << exec((char*)"ls /dev | grep 'tty[S*|U*]'").c_str() << std::endl;

    } else if (std::string(argv[1]).compare("-c") == 0) {
        if (argc != 3) {
            std::cout << "Invalid arguments" << std::endl;
            exit(0);
        }
        try {
            scales_client::reporting_service rep_srv;
            rep_srv.listener(std::string(argv[2]));
            std::cout << "terminating" << std::endl;
        } catch (std::runtime_error &ex) {
            std::cout << "Exception: " << ex.what() << std::endl;
        } catch (...) {
            std::cout << "Exception: UNKNOWN" << std::endl;
        }
    }
}

/**
 * @brief Invoke popen to record the resultant command output
 * @note execl() was too awkward with '| grep <blah>'
 * https://man7.org/linux/man-pages/man3/popen.3.html
*/
std::string exec(char* cmd) {
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while(!feof(pipe)) {
        if(fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }
    pclose(pipe);
    return result;
}
