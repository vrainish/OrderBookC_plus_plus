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
#include <thread>

using namespace std;


// main only process cli arguments
int main (int argc, char **argv)
{

    if (argc <2) exit_pricer("\n Error: no size given \n\n");

    int32_t msize = atoi(argv[1]);
    if (msize <= 0) exit_pricer("\n Error: size must be positive integer \n\n");



    Pricer{msize}.launch_pricer();  // Unnamed object deleted upon return


    exit(EXIT_SUCCESS);
    return 0;
} // end main

// exit in case of failure to allocate memory for tags or chunks
// or any other "system" problem
void exit_pricer(string message = "")
{
    cerr << message;
    exit(EXIT_FAILURE);
}
