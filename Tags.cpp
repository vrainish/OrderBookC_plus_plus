// Written by Vladimir Rainish
#include <iostream>

#include <assert.h>
#include "common.hpp"
#include "Chunks.hpp"
#include "Tags.hpp"
#include "Pricer.hpp"
#include "Line.hpp"

using namespace std;

// To develop a simple version of Tags for debug and for
// simplicity. Use a hash table with an appropriate function
// interesting to compare performance
// need to design in a way taht cam be easily changed
// should go away with registers , cache and memory and just make a dictionary
Tags::Tags()
{
    int32_t i;


    for (i=0; i < TCSIZE; i++) {
        tcache[i].cp = 0;
        tcache[i].lbitm = 0;
        tcache[i].mem = NULL;
    }
    for (i=0; i <REGSIZE; i++) {
        treg[i] = NULL;
    }


    // initialize first tag depot so no need to check each time
    struct tag_depot *new_td = new struct tag_depot;
    new_td->count = 0;
    // my_tag_depot.push_front(new_td);
    my_tag_depot.push(new_td);
}

Tags::~Tags()
{
    // do we need to free depots and free lists here ?
}

void Tags::print_init() const
{
    // fprintf(stdout,"Tag Cache Size %d (%d lines in set) Tag Mask %08x\n\n",TCSIZE,TCLINE,TCMASK);
}

// error_codes_t Tags::remove(uint64_t ntag, int32_t &sell, int32_t &amount, int32_t &price,chunk_ptr_t &dcptr)
error_codes_t Tags::remove()
{
    uint32_t rt = pl->ntag & REGMASK;
    tr_ptr_t found;

    // fprintf(stdout,"Before remove action TREG[%d] = %d [%s] 0x%08"PRIu64"\n",rt,treg[rt].amount,print_tag(treg[rt].ntag),treg[rt].ntag);
    if (treg[rt] && (treg[rt]->ntag == pl->ntag))
    {
        // register hit
        // fprintf(stdout,"[%s]%d (0x%08"PRIu64") is removed from treg[%d] [%s]%d (0x%08"PRIu64")\n",print_tag(pl.ntag),pl.amount,pl.ntag, rt, print_tag(treg[rt]->ntag),treg[rt]->amount,treg[rt]->ntag);
        found = treg[rt];
        if (pl->bid_amount == treg[rt]->amount) treg[rt]=NULL;    // register is free if everything is removed
    }
    else
    {
        found = remove_order_from_t_cache_or_memory();
        if (!found) return(NO_TAG_FOUND);
    }

    // need to check this part , what happens in case of complete removal and deleting structure
    // need to change and don't do removal itseld until copy
    // so just return pointer on removed tptr.
    found->amount -= pl->bid_amount;         // update amount in tag record
    pl->bid_price = found->price;            // restore price
    pl->side  = found->side;                 // restore action

    tag_price = found->price;                // restore price
    tag_amount  = found->amount;             // restore action
    tag_side  = found->side;                 // restore pointer to chunk record
    tag_cptr = found->cptr;

    // no need to update pl.tptr : it's not used when action is 'R'

    if (!found->amount) // if tag removed completely, tag_record going into linked list of free tag record for reusage
    {
        my_free_list.push_front(found);
    }
    return(NO_ERROR);
}



error_codes_t Tags::add()
{
    uint32_t rt = pl->ntag & REGMASK;
    // cout << "Add new tag !!!\n" << flush;
    tr_ptr_t new_tr = add_to_td();


    new_tr->amount      = pl->bid_amount;
    new_tr->ntag        = pl->ntag;
    new_tr->side        = pl->side;
    new_tr->price       = pl->bid_price;

    tag_price = new_tr->price;                // restore price
    tag_amount  = new_tr->amount;                 // restore action
    tag_side  = new_tr->side;                 // restore pointer to chunk record

    if (treg[rt]) move_register_to_cache(rt); // free register by moving it to cache

    treg[rt] = new_tr; // now "move" new order into register
    // fprintf(stdout,"treg[%d] <- [%s]%d\n",rt,print_tag(new_tr->ntag),new_tr->amount);

    // pl->tptr = new_tr; // after chunk addition cptr will be added to tag record
    current_tptr = new_tr;
    tag_cptr = NULL; // we don't know it yet, may be we don'y need to set it , unless for debugging and to be sure
    // it will be set in in pricer after calling chunk;
    return(NO_ERROR);

}

// check cache implementationshould try circular buffer
void Tags::move_register_to_cache(int32_t rt)
{
    uint32_t nt;
    int32_t i;
    tc_line_ptr_t mline;

    nt = SCRAMBLE(treg[rt]->ntag) & TCMASK;
    mline = &tcache[nt];
    i = mline->cp;
    // print_cache("Cache before addition",1);
    if (TEST_LINE(mline->cp)) i = get_free_line(nt);  		// if line is busy get next available line

    mline->tagar[i] = treg[rt]; 					//current pointer is pointing to free line  add there

    MARK_LINE_BUSY(i);                                            	// mark line as busy
    if ( (i == mline->cp) || (i == NEXT_LINE(mline->cp)) )
        mline->cp = NEXT_LINE(mline->cp);                   	// advance pointer so it points to next free

    return;
}

//tr_ptr_t Tags::remove_order_from_t_cache_or_memory(uint64_t ntag,int32_t amount)
tr_ptr_t Tags::remove_order_from_t_cache_or_memory()
{
    // return pointer to tag in depot
    // Remove register miss, check cache
    uint32_t nt;
    int32_t i;
    int32_t last_added;

    tc_line_ptr_t mline;
    nt = SCRAMBLE(pl->ntag) & TCMASK;
    mline = &tcache[nt];

    last_added = PREV_LINE(mline->cp);
    // fprintf(stdout,"last added %d\n",last_added);
    // fprintf(stdout,"tag [%s]%d (0x%08"PRIu64") removing from line %d \n",print_tag(pl.ntag),pl.amount,pl.ntag,nt);
    // print_cache("Cache before search",1);
    // first check last added
    if ( TEST_LINE(last_added) && (mline->tagar[last_added]->ntag == pl->ntag) )
    {
        // fprintf(stdout,"tag [%s]%d (0x%08"PRIu64") removed from line %d [%s]%d (0x%08"PRIu64")\n",print_tag(pl.ntag),pl.amount,pl.ntag,nt,print_tag(mline->tagar[last_added]->ntag),mline->tagar[last_added]->amount,mline->tagar[last_added]->ntag );
        // Just added removed
        if (mline->tagar[last_added]->amount == pl->bid_amount)  // free line
        {
            MARK_LINE_FREE(last_added);    // mark line as free
            mline->cp = last_added;  // set pointer
        }
        return (mline->tagar[last_added]);
    }
    // if we here last added line was a "miss"
    // loop over lines , in the future use count of zeros or mask
    // need to search search backward
    for (i = 0; i < TCLINE; i++)
    {
        if ( TEST_LINE(i) && (mline->tagar[i]->ntag == pl->ntag))
        {
            if (mline->tagar[i]->amount == pl->bid_amount) // complete remove
            {
                MARK_LINE_FREE(i); // mark line as free
            }
            return (mline->tagar[i]);

        }

    }
    // print_cache_line(nt,"Remove failed");
    // not in cache need to check memory
    if (mline->mem)
    {
        // print_cache_line(nt);
        tr_ptr_t tr_mem = mline->mem;
        tr_ptr_t prev;
        while(tr_mem)
        {
            if (tr_mem->ntag == pl->ntag)
            {
                if (tr_mem->amount == pl->bid_amount)
                {
                    if (tr_mem == mline->mem)
                        mline->mem = tr_mem->next;
                    else
                        prev->next = tr_mem->next;

                }
                return (tr_mem);
            }
            prev = tr_mem;
            tr_mem = tr_mem->next;
        }

    }
    return NULL;  // if here failed to find order ; ignore the line
}

int32_t Tags::get_free_line(int32_t nt)  // revert mask and use bitscan
{
    tc_line_ptr_t mline = &tcache[nt];
    int32_t i;
    int32_t next_line;

    if (mline->lbitm  != LPMASK) // check if we have at last one free line
    {
        // here try advance pointer and try or use bit scan to find free line
        next_line = NEXT_LINE(mline->cp);       // advance pointer
        if ( !TEST_LINE(next_line) )
            return next_line;
        else
            for (i = 0; i < TCLINE; i++)
                if ( !TEST_LINE(i) ) return i;
    }
    else
    {
        for (i = 0; i < TCLINE - 1; i++)
            mline->tagar[i]->next = mline->tagar[i+1]; // link them

        mline->tagar[i]->next = (mline->mem) ? mline->mem : NULL;  // if memory had something, chain it the last line
        mline->mem = mline->tagar[0];                        // line now first in mememory
        mline->lbitm = 0;                                    // clear and reset count
        mline->cp =0;
        // print_cache("Cache after move to memory",1);
        return 0;
        // move_line_to_memory()
    }
}

tr_ptr_t Tags::add_to_td()
{
    if (!my_free_list.empty())
    {
        // fprintf(stdout,"add to free %p->\n",tr_free_list,tr_free_list->next);
        tr_ptr_t free_tr = my_free_list.front();
        my_free_list.pop_front();
        return free_tr;
    }
    else
    {
        struct tag_depot *temp;
        temp = my_tag_depot.top();
        if (temp->count < TDSIZE)
        {
            tr_ptr_t tret;
            tret = &temp->td[temp->count];
            temp->count++;
            return tret;
        }
        else
        {
            // no space in free list or in depot we are working with
            // fprintf(stdout,"new depot allocated\n");
            struct tag_depot *new_td;
            new_td = new struct tag_depot;
            if (!new_td) exit_pricer("\n Error: no space left to allocate basic structures exit pricer \n\n");
            my_tag_depot.push(new_td);
            new_td->count = 0;

            // fprintf(stdout,"added to depot %d\n",td_curr->count);
            tr_ptr_t tret;
            tret = &new_td->td[new_td->count];
            new_td->count++;
            return tret;
        }
    }
}


