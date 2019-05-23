// Written by Vladimir Rainish

#ifndef CHUNK_H
#define CHUNK_H
#include <inttypes.h>


class Line;
class Pricer;
class CurrentState;

class Chunks
{
    public :
        Chunks(const char id='@');
        ~Chunks();
        void add(CurrentState *cs) ;
        void remove(CurrentState *cs) ;
        int32_t recompute(CurrentState *cs);

        char id;

        Line *pl;
    private:

        std::map<int32_t, int32_t> price_map;

        // for optimizing search by key
        // in general we can keep a small cache 2 or 4 items of recent key,chunk records pair
        // which we can implement as FIFO
        // use circular buffer for 2 or 4
        // check what to do if key is 0 can be for a very small price
        // need to check
        // also need to check if to do garbage collection
        // we need to add for that some sort of statistics
        // non-empty chunks vs empty chunks

        int32_t current_size;                     // for optimisationson
        void print_price_map(std::string message);
        };
#endif
