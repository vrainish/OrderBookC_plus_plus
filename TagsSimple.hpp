// Written by Vladimir Rainish

#ifndef TAG_H
#define TAG_H

#define TTC(a) ((char) (a + 'a' -1))
#define CTT(a) ((int)  (a - 'a' +1))

typedef struct tag_record tr_t;
typedef struct tag_record *tr_ptr_t;
struct tag_record
{
    int32_t side;
    uint64_t ntag;
    int32_t amount;
    int32_t price;
};

class Line;

// A simplified implementation of Tags
// all Tags a kept in the map
class Tags
{
    public :

        Tags();
        ~Tags();
        error_codes_t add();	// add tag
        error_codes_t remove(); // remove tag

        int32_t tag_price;                   //last processed tag_price  //restored in case of remove
        int32_t tag_amount;                  //last processed tag amount
        int32_t tag_side;                    //last processed taf side   // restored in case of remove

        tr_ptr_t  current_tptr;             // pointer to current tag structure;
        Line *pl;

    private :

        std::map<uint64_t,tr_ptr_t> tags;  // map to keep all tag records where ntag is used as a key
        std::stack<tr_ptr_t>        tags_for_reuse; // push removed tags here rather then delete
        char *print_tag(uint64_t mytag);

};
#endif
