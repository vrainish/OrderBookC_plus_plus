// Written by Vladimir Rainish
#ifndef LINE_H
#define LINE_H
#include <iostream>
// #include <cstdint>
#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

#define LINESIZE       128                    // total 
#define PRICE_UTOA     100                    // back of the line will be used for itoa  
#define INPUT_AREA      16                    // beginning of the line from 0 to 15 is used for error messages 
#define OK_OUTPUT       16                    // print regular output from here ALIGNED
#define ERROR_OUTPUT     0                    // print error message and original line 

class Line
{

    public :
        Line();
        ~Line();
        error_codes_t read_line();
        void write_line();
        void handle_error(error_codes_t);

        int32_t mil;                             // unused , preserved unchanged in input line
        int32_t action;                          // remove or add
        int32_t side;                            // in case of add - side of order; in case of remove side of removed order, restored by tag/chank processing
        int32_t bid_price;                       // in case of add - addded price; in case of remove the price of removed order, restored by tag/chunk processing
        int32_t bid_amount;                      // how many stocks
        uint64_t ntag;                           // do we need it here or it's needed only in tag chunk ?
        int32_t endmil;                          // position of end of milliseconds for printing
        int32_t output_price;                    // used by output
        print_status_t print_action;             // specify if to print32_t NA or price  migt be not needed

    private:
        char line[LINESIZE];

};
#endif
