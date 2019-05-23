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



Pricer::Pricer(const int32_t msize_):
    msize(msize_)
{
}

Pricer::~Pricer()
{
}


void Pricer::launch_pricer()
{

    Line current_line;
    pl = &current_line;

    Tags tags;     // instantiate Tags
    tags.pl = &current_line;           // Tags are using amount, price and ntag so make them known

    // we have two Chunks objects for sell and buy
    Chunks buy_chunks('B'),sell_chunks('S'); // instantiate Chunks
    buy_chunks.pl = &current_line;    // Make current line and state known to Chunks
    sell_chunks.pl = &current_line;    // Make current line and state known to Chunks


    // may be cs structures should be par of Chunk class
    CurrentState cs_sell('S',msize),cs_buy('B',msize);


    // Start of loop over lines
    //
    // move fgets to Line class
    for (;;)
    {
        error_codes_t rcode;
        int32_t recompute_price;

        rcode = current_line.read_line();

        if (rcode == END_OF_FILE) break;         // finish

        if (rcode) {
            current_line.handle_error(rcode);    // if error print error message and proceed to next line
            continue;
        }

        if (pl->action == REM_ORDER)  // Remove a bid partually or completely
        {
            if (rcode = tags.remove()) {
                current_line.handle_error(rcode);
                continue;
            }
            cs = (tags.tag_side == SELL) ? &cs_sell : &cs_buy; 		// sell has been restored, so we can set pointer to the correct side
            ce = (tags.tag_side == SELL) ? &sell_chunks : &buy_chunks;       // set Chunk process pointer to correct instance of Chunks
            
            #ifndef SIMPLE
            cs->cptr = tags.tag_cptr;                           // imortant : get cptr from tag
            #endif

            ce->remove(cs);					// call chunk engine

            #ifndef SIMPLE
            process_remove();
            #else
            process_remove_opt0();
            #endif

            if (pl->side == BUY) pl->output_price = -pl->output_price;     	// negate price back after processing if "restored" side/action is BUY  ADD
        }
        else
            // action is ADD_ORDER
        {

            if (pl->side == BUY) pl->bid_price = -pl->bid_price;                     // negate price if side is BUY and action is ADD

            if (rcode = tags.add()) {
                current_line.handle_error(rcode);
                continue;
            }
            cs = (tags.tag_side == SELL) ? &cs_sell : &cs_buy; 	        // The action is add so we know imeediately which Chunk instance to use
            ce = (tags.tag_side == SELL) ? &sell_chunks : &buy_chunks;

            ce->add(cs);					// call chunk engine

            #ifndef SIMPLE
            tags.current_tptr->cptr = cs->cptr;				// important, link tag to chunk
            #endif

            #ifndef SIMPLE
            process_add();
            #else
            process_add_opt0();
            #endif

            if (pl->side == BUY) pl->output_price = -pl->output_price;     	// negate price back after processing if side is BUY and action is ADD
        }

        if (pl->print_action != NO_OUTPUT)
        {
            current_line.write_line();
        }



    }  // end while loop over input lines
} // end launch_pricer




// recompute remove and recompute add
// try to do optimsations to avoid total price recomputation
// I've handled here most important cases
// some are missed (namely total price recomputation will be performed when it's possible to to do only if any partual recomputation
// The comments here are NOT complete and might be out of sync in some cases
// to develop a simple version of recompute for debugging and for size of code

// try to do without full price recomputation
void Pricer::process_remove_opt0()
{
    int32_t recompute = -1;
    int32_t adjust_range = -1;
    pl->print_action  = NO_OUTPUT;
    cs->total_amount_of_bids -= pl->bid_amount;  // update total amount
    if (cs->state == AV)
    {
        if (cs->total_amount_of_bids >= msize)   // not crossing to NA
        {
            adjust_range = 1;
            recompute = 1;
        }
        else
        {   /// handle crossing to NA
            // state changed AV -> NA
            //
            // need to check for empty amount
            // need to updates amounts anyway so when we crossing treshold back
            // no need in recomputations
            // orders amount is already updated
            //
            // for furter optimisations we need to upadte things here
            // this needed when we cross back to AV from NA in addition handling

            cs->msize_price -= pl->bid_amount * pl->bid_price;  // shoul update it together with amount
            adjust_range = 0;
            recompute = 0;
            pl->print_action = PRINT_NA;
            cs->state = NA;
            // if actaion was NA should we print something ?
        }  // end of handle crossing to NA
    }
    else
    {   // handle NA here
        // state not changed NA -> NA
        // more removed
        // state was NA and remain NA - do nothing ?
        //
        // still need to kep things updated same arguments as crossing from AV to NA
        adjust_range = 0;
        recompute = 0;
    }

    // adjust might be safely skipped since
    // now we start recompute from first chunk
    // the only thing it makes sence to do if we can adjust without recompute
    // after remove state can only cross from AV to NA
    // if (adjust_range) ce->adjust(cs);
    if (recompute)
    {
        int32_t recompute_price = ce->recompute(cs);
        if (recompute_price != cs->msize_price)
        {
            cs->msize_price = recompute_price;
            pl->print_action = PRINT_PRICE;
        }
    }
    // here we need to check if it;s PRINT_PRICE
    // since it might not change as a result of recompute
    if (pl->print_action == PRINT_PRICE) pl->output_price = cs->msize_price;
    return;
}
#ifndef SIMPLE
void Pricer::process_remove()
{
    //
    // Need to decide if full recomputation needed
    //
    // 1. Amount of sell orders crossed treshold below below to above it might happen only of action was add
    //    in that case we need to do total recomputation (check if we can optimize this case
    //
    // 2. Newly added/or removed order is in the participation range or below it
    //    if order added to the upper boundary of participation range do nothing
    //    if order removed from the upper boundary of participation range
    //    special check is needed
    //    if removal doesn't cause total amount of particiapting order to go below treshold then do nothing
    //    otherwise need very partual recomputatition
    //    in order to know that we need to keep which amount of upper boundary participating order contributed
    //    to total amount of participating order
    //
    // 3. if newly removd order caused crossing treshold from above to below
    //    then we need to issue NA
    //    here we need to decide what to do with participation range
    //
    //
    //   need to keep min max updated all the time
    //
    // recompute and adjust range are asserted so in a way those are manes to assist debuggin a complicated state
    int32_t recompute = -1;
    int32_t adjust_range = -1;
    pl->print_action  = NO_OUTPUT;
    // fprintf(stdout," before cs->total_amount_of_bids = %d\n",cs->total_amount_of_bids);
    // fprintf(stdout," after cs->total_amount_of_bids = %d\n",cs->total_amount_of_bids);
    // fprintf(stdout," cs->state = %d\n",cs->state);

    cs->total_amount_of_bids -= pl->bid_amount;  // update total amount

    // denote that state still reflects amount before adition ie amount and state not synchronized
    if (cs->state == AV)
    {
        if (cs->total_amount_of_bids >= msize)   // not crossing to NA
        {
            if (pl->bid_price <= cs->xp_price)
            {
                if (pl->bid_amount > cs->lp_free)
                {
                    // removal strictly below maxp and we don't have enough free
                    // may be we need to check in chunk handling if we completely removed bp_price
                    // not sure at al depends if we completely remove min_p
                    // we can optimise it by checkin also remove from chunk
                    if (cs->total_remove_from_bit & (pl->bid_price == cs->np_price))   // if we completely removed min price we need to adjust range
                        // this is the only case we meed to adjust since we need to find new
                        // np_chunk
                        adjust_range = 1;
                    else
                        adjust_range = 0;
                    recompute = 1;
                    pl->print_action = PRINT_PRICE;

                } // end pl->bid_amount > cs->lp_free
                else
                {   // handle all xp_rtee optimisation here
                    if (pl->bid_price == cs->xp_price)
                    {
                        // we remove exactly from xp_price and less or equal of free amount so we should be OK
                        // no changes in price

                        cs->lp_free -= pl->bid_amount;

                        adjust_range = 0;
                        recompute    = 0;
                    } // end of removing exactly from xp
                    else
                    {
                        // here we handle removal below xp_price
                        //
                        // we need to issue new price
                        // but we can optimise if we have enough lp_free

                        // because of we remove less then free we can't have total removal from bit here
                        // update price it will increase becase we use more  of more expensive offering
                        // reduce lp_free increase maxp used
                        cs->msize_price = cs->msize_price - pl->bid_amount * ( pl->bid_price - cs->xp_price );
                        cs->lp_free -= pl->bid_amount;     // xp_free_amount reduced
                        cs->lp_used += pl->bid_amount;   // maxp_used increased
                        adjust_range = 0;
                        recompute = 0;
                        pl->print_action = PRINT_PRICE;
                    } // end of removal strictly below xp
                } // end of removal in case we heve enough xp_free
            } // end removal from and below xp_price
            else
            {   // hhandle here a simple case of removal strictly above xp_price
                // ok we have pl->total_remove_from_bit to the resque
                if (cs->total_remove_from_bit && (pl->bid_price == cs->x_price))
                {
                    // need to check cs->total_amount_of_bids
                    if (!cs->total_amount_of_bids)
                    {
                        // no orders
                        cs->n_price = 0;
                        cs->x_price = 0;
                    }
                    else
                    {
                        // what to do
                        // here we need to reset nes x-price
                        // search keys in reverse
                        // check if total removal from chunk ?
                        // adjust range
                    }
                }
                adjust_range = 0;
                recompute    = 0;
            }

        } // end cs->total_amount_of_bids >= msize
        else
        {   /// handle crossing to NA
            // state changed AV -> NA
            //
            // need to check for empty amount
            // need to updates amounts anyway so when we crossing treshold back
            // no need in recomputations
            // orders amount is already updated
            //
            // for furter optimisations we need to upadte things here
            // this needed when we cross back to AV from NA in addition handling

            cs->msize_price -= pl->bid_amount * pl->bid_price;  // shoul update it together with amount
            adjust_range = 0;
            recompute = 0;
            pl->print_action = PRINT_NA;
            cs->state = NA;
        }  // end of handle crossing to NA
    } // end cs->ste == AV
    else
    {   // handle NA here
        // state not changed NA -> NA
        // more removed
        // state was NA and remain NA - do nothing ?
        //
        // still need to kep things updated same arguments as crossing from AV to NA
        adjust_range = 0;
        recompute = 0;
    }

    // test if adjust range and recomute was set ! superimportant for debugging

    // adjust might be safely skipped since
    // now we start recompute from first chunk
    // the only thing it makes sence to do if we can adjust without recompute
    if (adjust_range) ce->adjust(cs);
    if (recompute)
    {
        int32_t recompute_price = ce->recompute(cs);
        if (recompute_price != cs->msize_price)
        {
            cs->msize_price = recompute_price;
            pl->print_action = PRINT_PRICE;
        }
    }
    if (pl->print_action == PRINT_PRICE) pl->output_price = cs->msize_price;
    return;
}
#endif

void Pricer::process_add_opt0(void)
{
    int32_t recompute = -1;
    int32_t adjust_range = -1;
    pl->print_action  = NO_OUTPUT;
    cs->total_amount_of_bids += pl->bid_amount;  // adjust amount
    if (cs->state == AV)
    {
        adjust_range = 1;
        recompute = 1;
    }
    else
    {   // NA state
        if (cs->total_amount_of_bids >= msize) // crossing the treshold from NA to AV
        {
            // state changed NA -> AV
            // very easy optimisations no need to recompute at all
            // but we need to update amounts correctly
            // add more for optimisations explanation
            // optimize this part with additional checking the only place ? we need to adjust np_price it's not valid
            //
            // FOR NOW BE safe do a total recompute  if we maintain things OK  when we were in NA we can optimize
            //
            adjust_range = 1;
            recompute = 1;
            pl->print_action = PRINT_PRICE;
            cs->state = AV;
        }
        else
        {
            // state not changed NA -> NA
            //
            // not enough added
            // do nothing don't print
            // check if additional updates are needed
            // but we need to update fro optimisations when we finally will cross back to AV
            adjust_range = 0;
            recompute = 0;
            // no output , no state changing
        }
    }
    // to keep everything in sync we need to adjust price no matter waht
    // if state is NA then price is just a price of all orders in system
    // that's problematic with recompute
    // may be if state cross from AV to NA, we just need to set it to 0
    // check case carefully since optimisations might be applied on crossin as well
    // we not always need to do a full recompute
    // gather statisticas on stage slippage
    // OK awe are here hopefully np_price is OK
    // if (adjust_range) ce->adjust(cs);
    if (recompute)
    {
        int32_t recompute_price = ce->recompute(cs);
        if (recompute_price != cs->msize_price)
        {
            cs->msize_price = recompute_price;
            pl->print_action = PRINT_PRICE;
        }
    }

    if (pl->print_action == PRINT_PRICE) pl->output_price = cs->msize_price;
    return;

}

#ifndef SIMPLE
void Pricer::process_add(void)
{
    int32_t i,j,k;

    int32_t recompute = -1;
    int32_t adjust_range = -1;
    pl->print_action  = NO_OUTPUT;
    //
    // ADDITION LOGIC
    //
    // all additions should be optimised using lp_used !
    cs->total_amount_of_bids += pl->bid_amount;  // adjust amount
    // can reduce nesting and made code readbale by returning immediately from cases
    if (cs->state == AV)
    {
        if (pl->bid_price < cs->xp_price)
        {
            if (pl->bid_amount >= cs->lp_used)                   // for now >= check if we can do strictly >
            {
                // FOR NOW TOTAL RECOMPUTE need optimise
                pl->print_action = PRINT_PRICE;
                adjust_range = 1; // why we need it here ? we need to adjust ONLY if new min price was added
                recompute = 1;
            }
            else
            {
                // easy case we can use used_xp optimisation
                // but we need to check if add below min_xp
                // optimise using maxp_used
                // if we add amount which is less then maxp_used
                // we don't need to recompute
                // can only use strict less otherwise need to find new xp_price price ?????
                //
                if (pl->bid_price < cs->np_price)
                {
                    // addition below the np_price
                    // need total recompute BUT ! might optimise we we don't need regular recompute
                    // price recomputation
                    // update price it will decrease because we use more of less expensive offering
                    cs->msize_price = cs->msize_price + pl->bid_amount * ( pl->bid_price - cs->xp_price );
                    cs->np_price = pl->bid_price;   // set new np_price here or rely on adjust range
                    cs->lp_free += pl->bid_amount;    //    xp_free_amount increased
                    cs->lp_used -= pl->bid_amount;    //    maxp_used reduced

                    adjust_range = 1;               // tricky case we need to run adjust but we need to optimise it since we know the new key is minimal
                    recompute = 0;                  // careful here
                    pl->print_action = PRINT_PRICE;

                }  // end adding below minimal price
                else
                {
                    // price recomputation
                    // update price it will decrease because we use more of less expensive offering
                    cs->msize_price = cs->msize_price + pl->bid_amount * ( pl->bid_price - cs->xp_price );

                    // increase lp_free reduce maxp used
                    cs->lp_free += pl->bid_amount;    //    xp_free_amount increased
                    cs->lp_used -= pl->bid_amount;    //    maxp_used reduced

                    // no recompute is needed but new price must be issued
                    adjust_range = 0;
                    recompute = 0;
                    pl->print_action = PRINT_PRICE;
                }
            } // end handling optimsation where addition is less then xp_used amount
        } // end handling addition below xp_price
        else
        {
            // trivial
            // additon was on or above xp_price

            // state was AV and
            // addition outside or touch the range do nothing
            // only update participating amount of upper boundary
            // if addition touches exact uppper boundary of participating range
            if (pl->bid_price == cs->xp_price)
            {
                // lp_free increased but maxp_used is the same
                cs->lp_free += pl->bid_amount;
            }
            else
            {
                if (pl->bid_price > cs->x_price) // only need to adjust max price not sure we need other then for statistics
                {
                    cs->x_price = pl->bid_price;
                }
            }
            adjust_range = 0;
            recompute = 0;
            // no output needed
        }

    }   // end handling for AV state for addition
    else
    {   // NA state
        if (cs->total_amount_of_bids >= msize) // crossing the treshold
        {
            // state changed NA -> AV
            // very easy optimisations no need to recompute at all
            // but we need to update amounts correctly
            // add more for optimisations explanation
            // optimize this part with additional checking the only place ? we need to adjust np_price it's not valid
            //
            // FOR NOW BE safe do a total recompute  if we maintain things OK  when we were in NA we can optimize
            //
            adjust_range = 1;
            recompute = 1;
            pl->print_action = PRINT_PRICE;
            cs->state = AV;
        }
        else
        {
            // state not changed NA -> NA
            //
            // not enough added
            // do nothing don't print
            // check if additional updates are needed
            // but we need to update fro optimisations when we finally will cross back to AV
            adjust_range = 0;
            recompute = 0;
            // no output , no state changing
        }
    } // and handling NA state for addition
    // OK awe are here hopefully np_price is OK
    if (adjust_range) ce->adjust(cs);
    if (recompute)
    {
        int32_t recompute_price = ce->recompute(cs);
        if (recompute_price != cs->msize_price)
        {
            cs->msize_price = recompute_price;
            pl->print_action = PRINT_PRICE;
        }
    }

    if (pl->print_action == PRINT_PRICE) pl->output_price = cs->msize_price;
    return;
}
#endif

// Possibly to gather it in a separate class
