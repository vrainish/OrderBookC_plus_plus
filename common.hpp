// Written by Vladimir Rainish

#include <iostream>
// #include <cstdint>
#include <list>
#include <stack>
#include <map>
#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

#define REM_ORDER 0
#define ADD_ORDER 1

#define BUY  0
#define SELL 1

#define NA  0  // not enough orders
#define AV  1  // enough orders 

typedef enum { NO_OUTPUT =0, PRINT_PRICE, PRINT_NA } print_status_t;
typedef enum {NO_ERROR =0, BAD_ACTION,BAD_TAG, BAD_SIDE, BAD_PRICE, BAD_AMOUNT, ZERO_AMOUNT, NO_TAG_FOUND, END_OF_FILE} error_codes_t;

void exit_pricer(std::string);

