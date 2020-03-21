/**
 * Author: Jian Ding
 * Email: jianding17@gmail.com
 *
 */
#include "simulator.hpp"

int main(int argc, char const* argv[])
{
    std::string confFile;
    int thread_num, core_offset, delay;
    if (argc == 5) {
        thread_num = strtol(argv[1], NULL, 10);
        core_offset = strtol(argv[2], NULL, 10);
        delay = strtol(argv[3], NULL, 10);
        confFile = std::string("/") + std::string(argv[4]);
    } else {
        confFile = "/data/tddconfig-sim-dl.json";
        thread_num = 4;
        core_offset = 22;
        delay = 5000;
        printf("Wrong arguments (requires 4 arguments: 1. number of tx "
               "threads, 2. "
               "core offset, 3. frame duration, 4. config file)\n");
        printf("Arguments set to default: 4, 22, 5000, %s\n", confFile.c_str());
    }
    std::string cur_directory = TOSTRING(PROJECT_DIRECTORY);
    std::string filename = cur_directory + confFile;
    auto* cfg = new Config(filename.c_str());
    Simulator* simulator;
    int ret;
    try {
        SignalHandler signalHandler;

        // Register signal handler to handle kill signal
        signalHandler.setupSignalHandlers();
        simulator = new Simulator(cfg, thread_num, core_offset, delay);
        simulator->start();
        ret = EXIT_SUCCESS;
    } catch (SignalException& e) {
        std::cerr << "SignalException: " << e.what() << std::endl;
        ret = EXIT_FAILURE;
    }

    return ret;
}