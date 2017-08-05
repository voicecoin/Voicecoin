#ifndef BCUS_DB_PROXY_H
#define BCUS_DB_PROXY_H

#include <cstdint>
#include <vector>
#include <map>
#include "dbwrapper.h"
#include "block_info.h"

class block_info_db : public dbwrapper
{
public:
    block_info_db();

    bool write_block_info(const block_info &block_info);
    bool load_block_info();
};

class tran_pos_db : public dbwrapper
{
public:
    tran_pos_db();

    bool write_tran_pos(
        const std::vector<std::pair<uint256, block_tran_pos>> &tran_pos_array);
    bool read_tran_pos(const uint256 &tranid, block_tran_pos &tran_pos);

};

#endif // BCUS_DB_PROXY_H