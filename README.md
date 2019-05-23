Order Book Implementation in C++


4 classes Pricer, Chunks, Tgas and Line




class Pricer (Pricer.hpp Pricer.cpp)

is a driver which invokes
Tags and Chunks engine
it also have two methods add and remove which try to see if it's possible to do without travelling
 over chunks to recompute to price (that's optimisations)
Class CurrentState

class Chunks (Chunks.hpp and Chunks.cpp)

is dealing with hadling of chunks where each bit corresponds to a particular price
and also handle the recompute of price if full recompute is needed
I use a map template to hold chunks (in C version they were hold as sorted linked list I maintain
myseld, now map do all the maintenance)

we have two instances of this class for buy and for sell
class Tags (Tags.hpp and Tags.cpp)
 
handles tags which a completely separate from chunks.
the is one instanse of class Tags
class Line (Line.hpp and Line.cpp)

handles input/output of results


use make compile/ make test to build run

===================================================

I also wrote an alternative classes TagsSimple ChunksSimple

which do the exact same job as Tgas ( same interface , but in a much simplex way (just about 40 lines of actual code)
 using std::map template to do all the job)

use make compile_ts / make test_ts  to build/run pricer ts which uses TagsSimple

