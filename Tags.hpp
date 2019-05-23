// Written by Vladimir Rainish

#ifndef TAG_H
#define TAG_H


#define REGSIZE 256                      // 256 "tag registers" 
#define TCSIZE  128                      // 128 "tag cache sets" 
#define TCLINE    8                      // 8 lines per set
#define TDSIZE   64                      // 64 tag records per one allocation of tag depot 
#define REGMASK (REGSIZE-1)
#define TCMASK (TCSIZE-1)
#define LPMASK ((uint32_t)(1 << TCLINE) -1)                           // LPMASK has 1 bit for each line 
#define TEST_LINE(pos)        ((mline->lbitm >> pos) & 1)        // test bit in line  
#define MARK_LINE_FREE(pos)    (mline->lbitm &= ~(1<<pos))       // clear bit in line
#define MARK_LINE_BUSY(pos)    (mline->lbitm |= (1 << pos))      // mark line as busy
#define NEXT_LINE(cp)    ((cp + 1) & (TCLINE-1))                 // pointer to next line
#define PREV_LINE(cp)    ((cp - 1) & (TCLINE-1))                 // pointer to prev line 

// #define SCRAMBLE(n)    (n >> 7)
// #define SCRAMBLE(n)    (((n & 0x33333333) << 2)  |  ((n & 0xcccccccc) >> 2))
// #define SCRAMBLE(n)    (((n & 0x55555555) << 1)  |  ((n & 0xaaaaaaaa) >> 1))
#define SCRAMBLE(n)    (n)
#define TTC(a) ((char) (a + 'a' -1))
#define CTT(a) ((int)  (a - 'a' +1))


// Need to keep tag_record here since it's used in cs
// tag pointer should be defined in Tag class header
// We can simplify Tags using map template or hash table

typedef struct tag_record tr_t;
typedef struct tag_record *tr_ptr_t;
struct tag_record
{
    int32_t side;
    uint64_t ntag;
    int32_t amount;
    int32_t price;
    chunk_ptr_t cptr;  // connection to chunk pointer
    tr_ptr_t next;  // needed for memory, onece memory implemented as list template , remove
};

class Line;

class Tags
{
    public :

// Should we have a pointer to Line and get rid of parameters
        Tags();
        ~Tags();
        void print_init() const;
        // error_codes_t remove(uint64_t ntag, int32_t &sell, int32_t &amount, int32_t &price, chunk_ptr_t &cptr);
        // error_codes_t add(uint64_t ntag, int32_t sell, int32_t amount, int32_t price, tr_ptr_t &tptr);
        error_codes_t remove();
        error_codes_t add();

        int32_t tag_price;                   //last processed tag_price  //restored in case of remove
        int32_t tag_amount;                  //last processed tag amount
        int32_t tag_side;                    //last processed taf side   // restored in case of remove
        chunk_ptr_t tag_cptr;
        tr_ptr_t  current_tptr;             // pointer to current tag structure;
        Line *pl;                            // points to current line so we don'y need to pass parameters to add and remove

    private :
        struct tag_depot                     // here actual tag records are kept. "registers" "cache" and "memory" points to here
        {
            int32_t count;
            tr_t td[TDSIZE];
        };
        struct tag_cache
        {
            int32_t cp;                     // pointer to current item
            uint32_t lbitm;         // bitmask of lines
            // implement memory as a map template and not list
            tr_ptr_t mem;               // pointer to memory
            tr_ptr_t tagar[TCLINE];     // lines implement as vectors ?
        };
        typedef struct tag_cache *tc_line_ptr_t;
        typedef struct tag_cache  tc_line_t;

        // in the future template to handle my_tag_depot and my_gree_list
        // should use stack for tag depot since we don't really traverse it
        // we never delete tag_depot record because of free_list
        // in the future we might count elements of tag depots in free list for garbage collection
        // should use stack template and get reed of next
        // should possibly use stack template for free list as well
        std::list<struct tag_record *> my_free_list;
        // use stack template for tag_depot
        // might switch back to list if necessary
        std::stack<struct tag_depot *> my_tag_depot;

        // todo use vector templates for treg and tcache
        tr_ptr_t   treg[REGSIZE];   // registers vectors ?
        struct tag_cache  tcache[TCSIZE];  // cache vectors ?

        void move_register_to_cache(int32_t rt);
        // tr_ptr_t remove_order_from_t_cache_or_memory(uint64_t ntag,int32_t amount);
        tr_ptr_t remove_order_from_t_cache_or_memory();
        int32_t get_free_line(int32_t nt);
        tr_ptr_t add_to_td();
        // for debug only
        void print_register(char *title);
        void print_cache(char *title,int32_t print_empty);
        void print_cache_line(int32_t set);
        char *print_tag(uint64_t mytag);

};
#endif
