/**
 * Author: Jian Ding
 * Email: jianding17@gmail.com
 * 
 */

#ifndef SIMULATOR_HEAD
#define SIMULATOR_HEAD

#include <unistd.h>
#include <memory>
#include <iostream>
#include <vector>
#include <fcntl.h>
#include <system_error>
#include <pthread.h>
#include <queue>
#include "mufft/fft.h"
// #include <complex.h>
#include <math.h>
#include <tuple>
// #include <armadillo>
#include <immintrin.h>
#include <emmintrin.h>
#include <stdint.h>
#include <signal.h>
#include <algorithm>
// #include <aff3ct.hpp>
// #include "mkl_dfti.h"
// #include <hpctoolkit.h>
// #include <cblas.h>
// #include <stdio.h>
#include "cpu_attach.hpp"
#include "packageReceiver.hpp"
#include "packageSender.hpp"
#include "buffer.hpp"
#include "concurrentqueue.h"
#include "gettime.h"
// #include "compute_common.hpp"
#include "offset.h"
// #include "dofft.hpp"
// #include "dodemul.hpp"
#include "memory_manage.h"
#include "config.hpp"
#include "signalHandler.hpp"

class Simulator
{
public:
    // static const int TASK_THREAD_NUM = 10;
    // static const int SOCKET_RX_THREAD_NUM = 4;
    // static const int SOCKET_TX_THREAD_NUM = 4;
    // static const int CORE_OFFSET = 21;
    /* optimization parameters for block transpose (see the slides for more details) */
    static const int transpose_block_size = 8;
    static const int transpose_block_num = 256;
    /* dequeue bulk size, used to reduce the overhead of dequeue in main thread */
    static const int dequeue_bulk_size = 32;
    static const int dequeue_bulk_size_single = 8;

    Simulator(Config *cfg, int in_task_thread_num, int in_socket_tx_num, int in_core_offset, int sender_delay);
    ~Simulator();

    void start();
    void stop();
    // while loop of task thread
    static void *taskThread(void *context);

    struct EventHandlerContext
    {
        Simulator *obj_ptr;
        int id;
    };

    inline void update_frame_count(int *frame_count);
    
    void update_rx_counters(int frame_id, int frame_id_in_buffer, int subframe_id, int ant_id);
    void print_per_frame_done(int task_type, int frame_id, int frame_id_in_buffer);
    
    void initialize_vars_from_cfg(Config *cfg);
    void initialize_queues();
    void initialize_uplink_buffers();
    void free_uplink_buffers();

private:
    int BS_ANT_NUM, UE_NUM;
    int OFDM_CA_NUM;
    int OFDM_DATA_NUM;
    int subframe_num_perframe, data_subframe_num_perframe;
    int ul_data_subframe_num_perframe, dl_data_subframe_num_perframe;
    int dl_data_subframe_start, dl_data_subframe_end;
    int package_length;

    int TASK_THREAD_NUM, SOCKET_RX_THREAD_NUM, SOCKET_TX_THREAD_NUM;
    int CORE_OFFSET;
    int demul_block_size, demul_block_num;

    /* lookup table for 16 QAM, real and imag */
    float **qam16_table_;
    // float *pilots_;
    Config *cfg_;
    int max_equaled_frame = 0;
    float csi_format_offset;
    int buffer_frame_num;
    int max_packet_num_per_frame;
    std::unique_ptr<PackageReceiver> receiver_;
    std::unique_ptr<PackageSender> sender_;
    pthread_t *task_threads;
    EventHandlerContext *context;
    /*****************************************************
     * Buffers
     *****************************************************/ 
    /* Uplink */   
    /** 
     * received data 
     * Frist dimension: SOCKET_THREAD_NUM
     * Second dimension of buffer (type: char): package_length * subframe_num_perframe * BS_ANT_NUM * SOCKET_BUFFER_FRAME_NUM
     * package_length = sizeof(int) * 4 + sizeof(ushort) * OFDM_FRAME_LEN * 2;
     * Second dimension of buffer_status: subframe_num_perframe * BS_ANT_NUM * SOCKET_BUFFER_FRAME_NUM
     */
    // char *socket_buffer_[SOCKET_RX_THREAD_NUM];
    // int *socket_buffer_status_[SOCKET_RX_THREAD_NUM];
    // SocketBuffer socket_buffer_[SOCKET_RX_THREAD_NUM];

    char **socket_buffer_;
    int **socket_buffer_status_;
    long long socket_buffer_size_;
    int socket_buffer_status_size_;
    

    /* Uplink status checkers used by master thread */ 
    /* used to check if RX for all antennas and all subframes in a frame is done (max: BS_ANT_NUM * subframe_num_perframe) */
    int *rx_counter_packets_;   

    
    /*****************************************************
     * Concurrent queues 
     *****************************************************/ 
    /* main thread message queue for data receiving */
    moodycamel::ConcurrentQueue<Event_data> message_queue_;
    /* main thread message queue for task completion*/
    moodycamel::ConcurrentQueue<Event_data> complete_task_queue_;

    /* Tokens */
    moodycamel::ProducerToken **rx_ptoks_ptr;
    moodycamel::ProducerToken **tx_ptoks_ptr;
    moodycamel::ProducerToken **task_ptoks_ptr;


    /*****************************************************
     * Timestamps and counters used in worker threads 
     *****************************************************/ 
    double **frame_start;
    double *frame_start_receive;
    double *frame_end_receive;
    double *frame_start_tx;
    double *frame_end_tx;

};

#endif
