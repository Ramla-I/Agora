#include "ldpc_worker.hpp"
#include "utils.h"

void* run_worker(void* args)
{
    auto* cfg = *(static_cast<Config**>(args));
    auto tid = *(reinterpret_cast<int*>(static_cast<Config**>(args) + 1));
    auto* nexus = *(reinterpret_cast<erpc::Nexus**>(
        static_cast<int8_t*>(args) + sizeof(Config*) + sizeof(int)));
    pin_to_core_with_offset(
        ThreadType::kWorker, cfg->ldpc_worker_core_offset, tid);

    auto* worker = new LDPCWorker(cfg, tid, nexus);
    worker->serve();

    return NULL;
}

int main(int argc, char** argv)
{
    std::string confFile;
    if (argc == 2)
        confFile = std::string("/") + std::string(argv[1]);
    else
        confFile = "/data/tddconfig-sim-ul.json";
    std::string cur_directory = TOSTRING(PROJECT_DIRECTORY);
    std::string filename = cur_directory + confFile;
    auto* cfg = new Config(filename.c_str());
    cfg->genData();

    constexpr int kReqType = 2;
    constexpr int kUDPPort = 31850;

    // XXX: Avoid copying ldpc_worker_num
    // XXX: cfg->ldpc_worker_num could be cfg->ldpc_threads_per_remote_server
    size_t num_workers = cfg->ldpc_worker_num;

    auto* task_threads = new pthread_t[num_workers];

    std::string uri = cfg->ldpc_worker_addr + ":" + std::to_string(kUDPPort);
    auto* nexus = new erpc::Nexus(uri, 0, 0);
    nexus->register_req_func(kReqType, ldpc_req_handler);

    // XXX: Use size_t i instead of int i in all loops
    for (int i = 0; i < num_workers; i++) {
        auto* args
            = new char[sizeof(Config*) + sizeof(int) + sizeof(erpc::Nexus*)];
        *(reinterpret_cast<Config**>(args)) = cfg;
        *(reinterpret_cast<int*>(args + sizeof(Config*))) = i;
        *(reinterpret_cast<erpc::Nexus**>(args + sizeof(Config*) + sizeof(int)))
            = nexus;
        int ret;
        // XXX: Use std::thread instead of pthread_create
        ret = pthread_create(&task_threads[i], NULL, run_worker, args);
        if (ret != 0) {
            perror("task thread create failed");
            exit(0);
        }
    }

    for (int i = 0; i < num_workers; i++) {
        pthread_join(task_threads[i], NULL);
    }

    return 0;
}