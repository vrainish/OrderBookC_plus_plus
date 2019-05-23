// Written by Vladimir Rainish
#include <iostream>

#include <assert.h>
#include "common.hpp"
#include "ChunksSimple.hpp"
#include "TagsSimple.hpp"
#include "Pricer.hpp"
#include "Line.hpp"

using namespace std;

// To develop a simple version of Tags for debug and for
// simplicity. Use a map with an appropriate function
// interesting to compare performance
// need to design in a way taht cam be easily changed
Tags::Tags()
{
}

Tags::~Tags()
{
}

error_codes_t Tags::remove()
{
    tr_ptr_t found;
    map<uint64_t,tr_ptr_t>::iterator t_it;

    t_it = tags.find(pl->ntag);
    if (t_it == tags.end())
    {
        return(NO_TAG_FOUND);
    }
    found = t_it->second;

    found->amount -= pl->bid_amount;             // update amount in tag record

    pl->bid_price = found->price;                // restore price
    pl->side  = found->side;                 // restore action

    tag_price = found->price;                // restore price
    tag_amount  = found->amount;             // restore action
    tag_side  = found->side;                 // restore pointer to chunk record

    if (!found->amount) // if tag removed completely, tag_record going into stack of free tag record for reusage
    {
        tags_for_reuse.push(t_it->second);
        tags.erase(t_it);
    }
    return(NO_ERROR);
}

error_codes_t Tags::add()
{
    tr_ptr_t new_tr;

    if (tags_for_reuse.empty())
    {
        new_tr = new tr_t;
    }
    else
    {
        new_tr = tags_for_reuse.top();
        tags_for_reuse.pop();
    }

    tags.insert(make_pair(pl->ntag,new_tr));
    new_tr->amount      = pl->bid_amount;
    new_tr->ntag        = pl->ntag;
    new_tr->side        = pl->side;
    new_tr->price       = pl->bid_price;

    tag_price = new_tr->price;                // restore price
    tag_amount  = new_tr->amount;                 // restore action
    tag_side  = new_tr->side;                 // restore pointer to chunk record

    // pl->tptr = new_tr;
    current_tptr = new_tr;

    return(NO_ERROR);

}

// fix ctaf bad style
// probably just should return long long int
char *Tags::print_tag(uint64_t mytag)
{
    static char ctag[9];
    int32_t i;
    uint64_t ntag;
    ntag = mytag;

    ctag[0] = ' ';
    ctag[1] = ' ';
    ctag[2] = ' ';
    ctag[3] = ' ';
    ctag[4] = ' ';
    ctag[5] = ' ';
    ctag[6] = ' ';
    ctag[7] = ' ';
    ctag[8] = '\0';

    for(i=0; i < 9; i++)
    {
        int32_t cch;

        cch = (ntag >> (i*8)) & 0xff;
        if (!cch) break;
        ctag[i] = cch;
    }
    return ctag;
}
