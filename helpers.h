#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

typedef curl_off_t Item;
typedef struct Mediator_t
{
   Item* data;  //circular queue of values
   int*  pos;   //index into `heap` for each value
   int*  heap;  //max/median/min heap holding indexes into `data`.
   int   N;     //allocated size.
   int   idx;   //position in circular queue
   int   minCt; //count of items in min heap
   int   maxCt; //count of items in max heap
} Mediator;

struct memory {
    char *response;
    size_t size;
};

extern Mediator* MediatorNew(int nItems);
extern Item MediatorMedian(Mediator* m);
extern void MediatorInsert(Mediator* m, Item v);