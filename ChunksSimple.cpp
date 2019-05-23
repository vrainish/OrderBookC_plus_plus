// Written by Vladimir Rainish
#include <assert.h>
#include "common.hpp"
#include "ChunksSimple.hpp"
#include "TagsSimple.hpp"
#include "Pricer.hpp"
#include "Line.hpp"

using namespace std;

Chunks::Chunks(char id_):
    id(id_)
{
}

Chunks::~Chunks()
{
}

void Chunks::remove(CurrentState *cs)
{
    map<int32_t,int32_t>::iterator it;

    it = price_map.find(pl->bid_price);  // must be in the map
    it->second -= pl->bid_amount;             // update amount in tag record
    if (!it->second) // if price removed completely
    {
        price_map.erase(it);
    }
    return;
}

void Chunks::add(CurrentState *cs)
{
    map<int32_t,int32_t>::iterator it;

    it = price_map.find(pl->bid_price);  // must be in the map
    if (it == price_map.end())
    {
        price_map.insert(make_pair(pl->bid_price,pl->bid_amount));
    }
    else
    {
        it->second += pl->bid_amount;
    }
}

void Chunks::print_price_map(string message)
{
    for( map<int32_t, int32_t>::iterator it = price_map.begin(); it != price_map.end(); it++)
    {
        cout << it->first << " " << it->second << flush;
    }
    cout << "\n";
}

int32_t Chunks::recompute(CurrentState *cs)
{
    int32_t  to_sell_price  = 0;
    int32_t  amount_needed  = cs->msize;

    for( map<int32_t, int32_t>::iterator it = price_map.begin(); it != price_map.end(); it++)
    {
       if (it->second < amount_needed)
       {
            to_sell_price += it->first * it->second;
            amount_needed -= it->second;
       }
       else
       {
            to_sell_price += it->first * amount_needed;
            break;
       }
    }
    return to_sell_price;
}
