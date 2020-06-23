
#ifndef PHY_STATS
#define PHY_STATS

#include "Symbols.hpp"
#include "config.hpp"
#include "memory_manage.h"

class PhyStats {
public:
    PhyStats(Config* cfg);
    ~PhyStats();
    void print_phy_stats();
    void update_bit_errors(size_t, size_t, uint8_t, uint8_t);
    void update_decoded_bits(size_t, size_t, size_t);
private:
    Config* config_;
    Table<size_t> decoded_bits_count_;
    Table<size_t> bit_error_count_;
};

#endif
