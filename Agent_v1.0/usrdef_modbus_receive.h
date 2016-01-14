#ifndef FUNC_H
#define FUCN_H

#include "modbus.h"

int modbus_reply_userdef(modbus_t *ctx, const uint8_t *req,
        int req_length, modbus_mapping_t *mb_mapping, const char flag[]);

#endif
