// Written by Vladimir Rainish
#include <assert.h>
#include "common.hpp"
#ifndef SIMPLE
#include "Chunks.hpp"
#include "Tags.hpp"
#else
#include "ChunksSimple.hpp"
#include "TagsSimple.hpp"
#endif
#include "Pricer.hpp"
#include "Line.hpp"

using namespace std;

#define LINESIZE       128                    // total 
#define PRICE_UTOA     100                    // back of the line will be used for itoa  
#define INPUT_AREA      16                    // beginning of the line from 0 to 15 is used for error messages 
#define OK_OUTPUT       16                    // print regular output from here ALIGNED
#define ERROR_OUTPUT     0                    // print error message and original line 

Line::Line()
{
}

Line::~Line()
{
}


error_codes_t Line::read_line(void)
{
//
//
// 1. timestamp                 has only digits and is followed by a single space : no information if it has leading zeroes so we assume it hasn't and may be of variable length
// 2. action                    has only 'R' or 'A' and is followed by a single space
// 3. order-id                  has up to 8 ASCII characters and is followed by a single space
// 4. side  (if action is 'A')  has only 'B' or 'S' and followed by a single space
// 5. price (if action is 'A')  has only digits and followed by '.' and ALWAYS two digits after '.' ( even zeroes like  '.00' or '.50' ) and is folllowed by a single space
// 6. amount                    has only digits and immediately followed by EOL
//
    int32_t i,j,n;
    uint64_t l;

    // read line from stdin
    if (fgets (&line[INPUT_AREA], LINESIZE-INPUT_AREA, stdin) == NULL) {
        return(END_OF_FILE);
    }


    for (i=INPUT_AREA; line[i] != ' '; i++);                                    // "process" timestamp : iskip it but remember where it ends,  we form output right after it
    endmil = i;

    i++;
    j = line[i++];
    if ((j != 'R') && (j != 'A')) return(BAD_ACTION);
    action = j & 1;                                                      // process action ASCII for 'R' is 0x52 for 'A' is 0x41 so we just use the last bit to get set it to 0 or 1

    l = 0;
    for (i++,j=0; line[i] != ' '; i++,j=j+8)                           // process tag up to 8 ASCII characters
    {
        int64_t m = line[i];
        l |= m << j;
    }
    ntag = l;

    //if (!remove)						   // check what happens with remove if it's reserved word
    if (action == ADD_ORDER)						   // if adding to book process side and price
    {
        i++;
        j = line[i++];
        if ((j != 'S') && (j != 'B')) return(BAD_SIDE);
        side = j & 1;                                           // process side ASCII for 'B' is x42 and for 'S' is 0x53 so use the last bit to set it to 0 or 1

        // need to add check if thers is a '.' and exactly two digits after it

        i++;
        n = 0;
        while ( line[i] != ' ')                                     // proces price (in cents) so ignore '.'
        {
            int32_t register m;
            m = line[i++];
            if (m == '.') continue;
            if ((m < '0') || (m > '9')) return(BAD_PRICE);
            n = (n<<3) + (n<<1) + m - '0';
        }
        if (!n) return(BAD_PRICE);
        bid_price = n;
    }

    i++;
    n = 0;
    while ( line[i] != 10)                                    // process amount, stop on EOL
    {
        int32_t register m = line[i++];
        if ((m < '0') || (m > '9')) return(BAD_AMOUNT);
        n = (n<<3) + (n<<1) + m - '0';
    }
    if (!n) return(ZERO_AMOUNT);
    bid_amount = n;
    return NO_ERROR;
} // end process_input

// stomp on input line right after timestamp ( timestamp itself reused abd will be printed)
// use back of the line for a primitive utoa to translate price to ASCII
void Line::write_line(void)
{
    int32_t i = ++endmil;                                                    // need to print timestamp

    line[i++] = (side == SELL) ? 'B' : 'S';                                  // denote that sell is restored if input action was remove so we are OK
    line[i++] = ' ';

    if (print_action == PRINT_PRICE)
    {
        uint32_t register n = output_price;                 // here price is always positive
        if (n > 99)					            // we reach a DOLLAR, so let's celebrate :-) !
        {
            int32_t register j = PRICE_UTOA;

            while (n > 0)                             // price itoa : price in cents (in reverse order) is in working buffer
            {
                int32_t register m,t;
                m = n/10;                      // can use fast divide by 10 algorthm which doesn't use idiv
                t = (m<<3) + (m<<1);
                t = n -t; // finding mod of 10
                line[j--] = t + '0';
                n = m;
            }

            for (j++; j < PRICE_UTOA-1;)                        // put price dollar part into the printing area
                line[i++] = line[j++];

            line[i++] = '.';                                    // insert '.' and copy cents - must be printed, even if zeroes
            line[i++] = line[j++];
            line[i++] = line[j++];
        }
        else
        {
            // price is between 1 and 99 cents separate handling not to mess with main case
            // note price is zero      - impossible (really ?)  but will be printed as 0.00

            line[i++] = '0';                                    // need a single leading zero if price is less then a dollar
            line[i++] = '.';
            if (n > 9)
            {
                int32_t register m,t;
                m = n/10;
                t = (m<<3) + (m<<1);
                t = n -t; // finding mod of 10
                line[i++] = t + '0';
                n = m;
            }
            else
                line[i++] = '0';                           // and another zero after dot  if we can't even squeeze a dime :-)

            line[i++] = n + '0';                               // nailing the last few cents !
        }
    }
    else
    {
        line[i++] = 'N';		      			  // handle NA case
        line[i++] = 'A';
    }
    line[i++] = '\n';
    line[i] = '\0';               					  // LAST ACTION - terminate the string !

    fputs(&line[OK_OUTPUT],stdout);
}

// print short error message together with current line
void Line::handle_error(error_codes_t rcode)
{
    const char *p_errors[] =
    {
        "123456789012345 ",   // it's a dummy string for convinience it corresponds to error code 0 - which is not erroe
        "bad action      ",
        "bad tag         ",
        "bad side        ",
        "bad price       ",
        "bad amount      ",
        "zero amount     ",
        "tag not found   ",
    };
    uint32_t *code_p;
    uint32_t *line_p;
    code_p = (uint32_t *) p_errors[rcode];
    line_p = (uint32_t *) &line[ERROR_OUTPUT];
    // the message length is 16 chatacters so we need 4 int copies to move it
    // to the ERROR_OUTPUT area
    *line_p++ = *code_p++;
    *line_p++ = *code_p++;
    *line_p++ = *code_p++;
    *line_p++ = *code_p++;
    fputs(&line[ERROR_OUTPUT],stdout);
}


