// Written by Vladimir Rainish
#ifndef PRICER_H
#define PRICER_H
#include <iostream>
// #include <cstdint>
#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

class Line;
#ifndef SIMPLE
class Chunks;
class Tags;
#else
class TagsSimple;
#endif

// current state has a common part for buy and sell
// like price,amount, pointer to chunk and tag, msize
// and specific parts for sell and buy

class CurrentState
{
    public:
        CurrentState(const char id_='@',const int32_t msize_ = 0):
            msize(msize_),
            id(id_)
        {
            // important to initialize, error without
            state = 0;
            msize_price = 0;
            total_amount_of_bids = 0;
            n_price = 0;
            np_price = 0;
            x_price = 0;
            xp_price = 0;
            n_price = 0;

            lp_free = 0;
            lp_used = 0;
            lpc_free = 0;
            lpc_used = 0;

            #ifndef SIMPLE
            np_chunk = NULL;
            #endif

        }

        const int32_t msize;
        const char    id;
        int32_t state;                      // last state of sell NA or AV, means if we hve enough orders in system, can be computed from msize and amount
        int32_t msize_price;              	// last total price of orders make sense only if state was AV
        int32_t total_amount_of_bids;       // current amount of sell orders
        //
        // Next 6 varaibles are used for optimizations
        // namely to avoid full recomputation of the price by scanning chunks
        //
        int32_t n_price;           		// minimal sell price of orders in system
        int32_t np_price;          		// minimal participateing price of orders in book - don't need it always equal to min_s (unless state_s is NA)
        //
        int32_t x_price;           		// maximal sell price of orders in system
        int32_t xp_price;          		// maximal participating price of orders in book
        int32_t lp_free;       	// amount of orders left free in last part participating bid   : needed to decide if we need recomputation
        int32_t lp_used;     	// amount of orders used in last participating bid             : needed to decide if we need recomputation
        int32_t lpc_free;       // amount of orders left free in last part participating chunk : needed to decide if we need to loop over keys
        int32_t lpc_used;     	// amount of orders used in last participating chunk           : needed to decide if we need to loop over keys 
        //
        // example assume the size is 200
        // if we have 30 orders of maximal price which participate in the sell
        // but only 20 of them needed , since other 180 orders are filled from lower prices
        // then lp_used is 20 and lp_free is 10
        // so that if we remove in next step 5 orders from anywhere in the participateing range
        // we still have 5  orders left in xp_used amount
        // in that case no need in full recomputation, we simply adjust the price
        // deductig the price of 5 removed orders and adding price of 15 orders in xp_price
        // so the prise increase will be 5*(xp_price - removed price)
        //

        #ifndef SIMPLE
        chunk_ptr_t np_chunk;	    	// pointer where chunk with minimal price is should be in the Chunk class but it's set and operatyed from recompute
        // more comments about meaning of np_chunk
        chunk_ptr_t cptr;                   // pointer to current chunk;
        tr_ptr_t    tptr;                   // pointer to current tag // make sense only if add (or partual remove)
        int32_t total_remove_from_bit;      // as a result of remove a bit in chink is 0
        int32_t total_remove_from_chunk;    // as a result of remove a chunk is empty
        #endif


};

class Pricer
{
    public :
        Pricer(const int32_t msize = 0);
        ~Pricer();
        void launch_pricer() ;
        CurrentState cs_sell,cs_buy; // current states of sell and buy books
        Line    *pl;                  // points to line class
        Chunks  *ce;		      // pointer to object Chunk_Engine
        CurrentState *cs;
        const int32_t msize;

    private:
        void process_remove();
        void process_add();
        void process_remove_opt0();
        void process_add_opt0();
};
#endif
