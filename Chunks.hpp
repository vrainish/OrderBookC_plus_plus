// Written by Vladimir Rainish

#ifndef CHUNK_H
#define CHUNK_H
// CHUNK STRUCTURES
#include <inttypes.h>

#define CHUNK_SIZE 32
#define CMAP (CHUNK_SIZE - 1)
#define KEYMAP ~CMAP
// KEYMAP is upper portion of price

class Line;
class Pricer;
class CurrentState;

typedef int32_t int32_t;

typedef struct chunk_record *chunk_ptr_t;
typedef struct chunk_record chunk_t;
struct chunk_record
{
    int32_t key;
    uint32_t map;
    uint32_t chunk_amount;
    int32_t chunk_price;   // total price of all orders in chunk
    int32_t bits_set;                 // how many bits in maps are set    -- for optimisations if 0 chunk is empty
    uint32_t bids[CHUNK_SIZE];
};

class Chunks
{
    public :
        int32_t get_current_size();

        Chunks(const char id='@');
        ~Chunks();
        void add(CurrentState *cs) ;
        void remove(CurrentState *cs) ;
        void adjust(CurrentState *cs);
        int32_t recompute(CurrentState *cs);

        char id;

        Line *pl;
    private:

        // implement chunk as list or map since we do need to search in it
        // we need access both by index and by key , Can we use map for it ?
        // maps keapt as balanced tree so it's good for search
        // list is not good since we need to traverse it from arbitrary element
        // so taht we can have map of pointers to chunk records, sorted by key
        std::map<int32_t, struct chunk_record *> ip_map;
        std::map<int32_t, struct chunk_record *> gc_ip_map;

        // for optimizing search by key
        // in general we can keep a small cache 2 or 4 items of recent key,chunk records pair
        // which we can implement as FIFO
        // use circular buffer for 2 or 4
        // check what to do if key is 0 can be for a very small price
        // need to check
        // also need to check if to do garbage collection
        // we need to add for that some sort of statistics
        // non-empty chunks vs empty chunks
        int32_t last_key_added;                     // for optimisationson
        int32_t last_price_added;
        chunk_ptr_t last_chunk_added;           // for optimisations pointer to chank with last added key;  // set in initialisation

        int32_t current_size;                     // for optimisationson
        // chunk_ptr_t first_chunk,last_chunk,chunk_current_high;
        chunk_ptr_t chunk_current_high;
        chunk_ptr_t find_the_key();
        static bool sort_ip(const chunk_ptr_t & i, const chunk_ptr_t & j);
        void print_ip_list(std::string message);
        void print_ip_map(std::string message);

            // for debug only
            // void print_chunk_list(cs_ptr_t s,char *title);
            // void print_chunk(cs_ptr_t s,chunk_ptr_t cptr);


        };
#endif
