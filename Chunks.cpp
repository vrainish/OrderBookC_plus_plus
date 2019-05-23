// Written by Vladimir Rainish
#include <assert.h>
#include "common.hpp"
#include "Chunks.hpp"
#include "Tags.hpp"
#include "Pricer.hpp"
#include "Line.hpp"

using namespace std;

Chunks::Chunks(char id_):
    id(id_)
{
    // consider to initialize chunk list with dummey key record
    // first_chunk = NULL;
    // last_chunk = NULL;
    last_chunk_added = NULL;
    chunk_current_high = NULL;
    last_key_added = 0;
    current_size = 0;
    last_price_added = 0;
}

Chunks::~Chunks()
{
    // free all chunk keys
}


int32_t Chunks::get_current_size() {
    return current_size;
}

// remove need price, amount and pointer to chunk
void Chunks::remove(CurrentState *cs)
{
    uint32_t bitpos;

    cs->total_remove_from_bit   = 0;
    cs->total_remove_from_chunk = 0;
    chunk_ptr_t ip = cs->cptr;                // set chunk pointer from its tag
    // chunk_ptr_t ip = cs->cptr;                // set chunk pointer from its tag
    bitpos = pl->bid_price & CMAP;

    ip->bids[bitpos] -= pl->bid_amount;
    ip->chunk_amount -= pl->bid_amount;
    ip->chunk_price -= (pl->bid_amount * pl->bid_price); // pl->bid_price is restored so it's OK
    // if it's only partual bid removal, then it's done
    // also might need to update total price of chunk for optimisations of chunk hopping

    if (! ip->bids[bitpos])    // check if bid completely removed
    {
        // complete remove we need to update map
        ip->map &= ~((uint32_t)(1 << bitpos));
        //
        cs->total_remove_from_bit = 1;
        //
        //
        // we need to probably check if it happens from xp_price position and find new xp_price
        // if complete removal from minp or maxp need to reset
        if (!ip->chunk_amount)
        {
            cs->total_remove_from_chunk = 1;
            // fprintf(stdout,"key %d is empty\n",ip->key);
        }
    }
    last_key_added = (pl->bid_price & KEYMAP);
    last_chunk_added = ip;
}
// add need price and amount
void Chunks::add(CurrentState *cs)
{
    uint32_t bitpos;
    chunk_ptr_t ip;

    cs->total_remove_from_bit   = 0;
    cs->total_remove_from_chunk = 0;

    // ip = (pl->bid_price & KEYMAP == last_key_added) ? last_chunk_added : find_the_key();
    // need brackets since & has lower precedence then ==
    // in the future also check prev and next chunks to get over 99%
    if ((pl->bid_price & KEYMAP) == last_key_added)
    {
        ip = last_chunk_added;
        // no need to sort at all
    }
    else
    {
        ip = find_the_key();
        // print_ip_list("Sorted");
        // print_ip_map("Sorted");
    }
    //last_chunk_added = ip;
    bitpos = pl->bid_price & CMAP;
    ip->map |= ((uint32_t)(1 << bitpos));
    ip->bids[bitpos] += pl->bid_amount;
    ip->chunk_amount += pl->bid_amount;
    ip->chunk_price += (pl->bid_amount * pl->bid_price); // pl->bid_price is restored so it's OK
    // comment more about pl->bid_price restoration
    cs->cptr = ip;
    last_key_added = (pl->bid_price & KEYMAP);
    last_chunk_added = ip;

}
// split it to new and find
chunk_ptr_t Chunks::find_the_key()
{
    chunk_ptr_t ip;
    int32_t search = pl->bid_price & KEYMAP;
    int32_t i;
    map<int32_t, struct chunk_record *>::iterator ip_it = ip_map.find(search);
    ip_it = ip_map.find(search);
    if (ip_it != ip_map.end())
    {
        ip = ip_it->second;
        return ip;
    }
    // check in GC map
    ip = new struct chunk_record;

    if (!ip) exit_pricer("\n Error: no space left to allocate basic structures \n\n");

    // without removing empty chunks they are the same
    chunk_current_high++;// not sure why we need it and why is ++
    current_size++;      // not sure why need it may be for optimizations in search thru chunks
    // Not needed now but need if we don't use list or map template
    // ip->prev = NULL;
    // ip->next = NULL;
    ip->key    = search;
    ip->map    = 0;
    ip->chunk_amount  = 0;
    ip->chunk_price = 0;
    for (i = 0; i < CHUNK_SIZE; i++) ip->bids[i] = 0;

    // print_ip_list("Before push ");
    // ip_list.push_front(ip);
    // check if to use [] !
    ip_map.insert(std::make_pair(ip->key,ip));
    return ip;
}

bool Chunks::sort_ip(const chunk_ptr_t & i, const chunk_ptr_t  & j)
{
    return (i->key < j->key);
}

void Chunks::print_ip_map(string message)
{
    chunk_ptr_t ip;
    cout << "ID " << id << ": ";
    cout << message << " " << flush;
    if (ip_map.empty()) {
        cout << "Empty \n";
        return;
    }
    for( map<int32_t, struct chunk_record *>::iterator ip_it = ip_map.begin(); ip_it != ip_map.end(); ip_it++)
    {
        // cout << "Iter print\n" << flush;
        if (!ip_it->second->chunk_amount) {
            cout << "*";
        }
        else {
            cout << " ";
        }
        cout << ip_it->second->key << "," << flush;
    }
    cout << "\n";
}

void Chunks::adjust(CurrentState *cs)
{

    chunk_ptr_t ip;
    map<int32_t, struct chunk_record *>::iterator ip_it;
    for( ip_it = ip_map.begin(); ip_it != ip_map.end(); ip_it++ )
    {
        // ip = *ip_it;
        ip = ip_it->second;
        if (ip->map) break;                          // got first non_empty chunk
    }

    // it uses bit scan forward to find first bit which is set to 1
    // that is ORed to key in order to form a minimal price
    // asm("bsfl %1, %0" : "=r"(posf) : "r"(ip->map));
    int32_t  register posf;
    asm("bsf %1, %0" : "+r"(posf) : "rm"(ip->map));
    cs->np_price = ip->key | posf;         // setting new min price
    cs->n_price = cs->np_price;         // probably redundant see if still need to keep cs.n_price might be needed if below treshold
    cs->np_chunk = ip;                     // results of adjust
    // adjust is setting from where to start
    // does it happens only when we do complete remove from min bit
}

// do we need to call adjust before
// it would be nice if in case of total recompute asjust is not needed
// complete scan over all orders need to be avoided since expensive
// create a simplified version
int32_t Chunks::recompute(CurrentState *cs)
{
    chunk_ptr_t ip;

    int32_t  to_sell_price  = 0;
    int32_t  amount_needed  = cs->msize;
    int32_t  finish = 0;
    int32_t  last_posf;   // needed in last bid processing
    std::map<int32_t, struct chunk_record *>::iterator ip_it;
    ip_it = ip_map.begin();
    while( ip_it != ip_map.end() )
    {
        // ip = *ip_it;
        ip = ip_it->second;
        uint32_t  map;
        if (!ip->map )
        {

        }
        if (ip->chunk_amount > amount_needed)
        {
            finish = 1;                            // if we got here this is the last chunk to be processed and we don't need all of it so can set finish to 1
            // fprintf(stdout,"before map to sell price %d %d\n",to_sell_price,i);
            // need to investigate the size of chunk influence on performance
            for (map = ip->map; map > 0;)
            {
                int32_t  register posf;
                asm("bsf %1, %0" : "+r"(posf) : "rm"(map));
                map &= ~(1 << posf);
                //  __asm__("bsf %1, %0" : "+r"(posf) : "rm"(map));
                //  __asm__("btr  %1, %0" : "+rm"(map)  : "r"(posf));


                // possible optimisation we might want to move multiplication out of loop
                // so we might keep bids prices in addition to bids amount
                if (ip->bids[posf] < amount_needed)
                {
                    amount_needed  -= ip->bids[posf];
                    to_sell_price  += ip->bids[posf] * (ip->key | posf) ;
                }
                else
                {
                    last_posf = posf;
                    break;
                }
            }
        }
        else
        {
            // hop over chunk add all the orders and that's it
            // we also handle the case here if we have just enogh orders so we can break;
            amount_needed  -= ip->chunk_amount;
            to_sell_price  += ip->chunk_price;
            // OK check here if we reach the end exactly on chunk boundary so we willl never be agin in the map loop
            // need to move it out of here to keep this loop small
            if (amount_needed == 0)
            {
                finish = 2;
                break;
            }
        }
        // fprintf(stdout,"after map to sell price %d %d\n",to_sell_price,i);
        if (finish) break;
        ++ip_it;
    }  // end of key loop
    //
    //
    // adjustments depends of finish
    if ( finish == 1 )
    {
        // we break out from inside the loop over bits in map
        // so we have posf set correctly and ip is OK as well
        // ok here we reach boundary
        // need to check if we have lp_free and update it

        cs->lp_free = ip->bids[last_posf] - amount_needed;
        cs->lp_used = amount_needed;

        // future optimisation move multiplication out of the loop
        to_sell_price += amount_needed * (ip->key | last_posf);

        // also need to set how much we used xp_price
        // together lp_free and max_p used is total amount of orders in xp_price price

        // consider to update it also in case of removal
        // it's simple and good for optimisations

        cs->xp_price = ip->key | last_posf; // update max participating price
    }
    else if ( finish == 2 )
    {
        // we were lucky so the last chunk fit perfectly to amount needed
        // that means our target price is already correct
        // no lp_free left and last bit was completely used
        int32_t  register posr;
        uint32_t  map;
        // now we need to update things
        map = ip->map;

        asm("bsr %1, %0" : "+r"(posr) : "rm"(map));  // we scan IN REVERSE to GET a max price of chunk

        cs->lp_free = 0;                // no xp_free_amount
        cs->lp_used = ip->bids[posr];
        cs->xp_price = ip->key | posr; // update max participating price
    }
    return to_sell_price;
}
