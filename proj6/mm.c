#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "mm.h"
#include "memlib.h"

#define WSIZE 4
#define DSIZE 8           
#define CHUNKSIZE (1<<12) 

#define MAX(x,y)    ((x)>(y)?(x):(y))

#define PACK(size,alloc)    ((size) | (alloc))

#define GET(p)  (*(unsigned int *)(p))
#define PUT(p,val)  (*(unsigned int *)(p) = (val))

#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p)    (GET(p) & 0x1)

#define HDRP(bp)    ((char *)(bp)-WSIZE)
#define FTRP(bp)    ((char *)(bp)+GET_SIZE(HDRP(bp))-DSIZE)


#define NEXT_BLKP(bp)   ((char *)(bp)+GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLKP(bp)   ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))

#define PREV(bp) ((char*)(bp))
#define NEXT(bp) ((char*)(bp)+WSIZE)

 void *extend_heap(size_t db);
 void *coalesce(void *bp);
 void place(void *bp,size_t asize);
void fix(char *p);

 char *heap_listp = NULL;
 char *root = NULL;


int mm_init(void)
{
   if((heap_listp = mem_sbrk(6*WSIZE))==(void *)-1) return -1;

   PUT(heap_listp,0);
   PUT(heap_listp+(1*WSIZE),0);
   PUT(heap_listp+(2*WSIZE),0);
   PUT(heap_listp+(3*WSIZE),PACK(DSIZE,1));
   PUT(heap_listp+(4*WSIZE),PACK(DSIZE,1));
   PUT(heap_listp+(5*WSIZE),PACK(0,1));
   root = heap_listp + (1*WSIZE);

   heap_listp += (4*WSIZE);

   if((extend_heap(CHUNKSIZE/DSIZE))==NULL) return -1;

   return 0;
}
 void *extend_heap(size_t db)
{
   char *bp;
   size_t size;

   size = (db % 2) ? (db+1) * DSIZE : db * DSIZE;

   if((long)(bp = mem_sbrk(size))==(void *)-1)
       return NULL;

   PUT(HDRP(bp),PACK(size,0));
   PUT(FTRP(bp),PACK(size,0));
   PUT(NEXT(bp),0);
   PUT(PREV(bp),0);
   PUT(HDRP(NEXT_BLKP(bp)),PACK(0,1));

   return coalesce(bp);
}

void *mm_malloc(size_t size)
{
   size_t asize;
   size_t extendsize;
   char *bp;
   if(size ==0) return NULL;

   if(size <= DSIZE)
   {
       asize = 2*(DSIZE);
   }
   else
   {
       asize = (DSIZE)*((size+(DSIZE)+(DSIZE-1)) / (DSIZE));
   }

   char *temp = GET(root);
   bp = NULL;
   while(temp != NULL)
   {
       if(GET_SIZE(HDRP(temp))>=asize) {
       	bp = temp;
       	break;
       }
       temp = GET(NEXT(temp));
   }


   if(bp!= NULL)
   {
       place(bp,asize);

       return bp;
   }

   /*apply new block*/
   extendsize = MAX(asize,CHUNKSIZE);
   if((bp = extend_heap(extendsize/DSIZE))==NULL)
   {
       return NULL;
   }
   place(bp,asize);

   return bp;
}


void mm_free(void *bp)
{

   size_t size = GET_SIZE(HDRP(bp));

   PUT(HDRP(bp), PACK(size, 0));
   PUT(FTRP(bp), PACK(size, 0));
   PUT(NEXT(bp),0);
   PUT(PREV(bp),0);
   coalesce(bp);

}

void *mm_realloc(void *ptr, size_t size)
{
   size_t oldsize;
   void *newptr;

   if(size == 0)
   {
       mm_free(ptr);
       return 0;
   }

   if(ptr == NULL)
   {
       return mm_malloc(size);
   }
   oldsize = GET_SIZE(HDRP(ptr));

   newptr = mm_malloc(size);

   if(!newptr)
   {
       return 0;
   }

   oldsize = GET_SIZE(HDRP(ptr));
   if(size < oldsize) oldsize = size;
   memcpy(newptr, ptr, oldsize);
   mm_free(ptr);

   return newptr;
}

 void *coalesce(void *bp)
{
   size_t  prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
   size_t  next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
   size_t size = GET_SIZE(HDRP(bp));

   if(prev_alloc && next_alloc)
   {
   }
   else if(prev_alloc && !next_alloc)
   {
       size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
       fix(NEXT_BLKP(bp));/*remove from empty list*/
       PUT(HDRP(bp), PACK(size,0));
       PUT(FTRP(bp), PACK(size,0));
   }
   else if(!prev_alloc && next_alloc)
   {
       size += GET_SIZE(HDRP(PREV_BLKP(bp)));
       fix(PREV_BLKP(bp));
       PUT(FTRP(bp),PACK(size,0));
       PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
       bp = PREV_BLKP(bp);
   }
   else
   {
       size +=GET_SIZE(FTRP(NEXT_BLKP(bp)))+ GET_SIZE(HDRP(PREV_BLKP(bp)));
       fix(PREV_BLKP(bp));
       fix(NEXT_BLKP(bp));
       PUT(FTRP(NEXT_BLKP(bp)),PACK(size,0));
       PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
       bp = PREV_BLKP(bp);
   }
;

   char *next = GET(root);
   if(next != NULL)
       PUT(PREV(next),bp);

   PUT(NEXT(bp),next);

   PUT(root,bp);

   return bp;
}


void fix(char *p)
{
   char *prevp = GET(PREV(p));
   char *nextp = GET(NEXT(p));
   if(prevp == NULL)
   {
       if(nextp != NULL)PUT(PREV(nextp),0);
       PUT(root,nextp);
   }
   else
   {
       if(nextp != NULL)PUT(PREV(nextp),prevp);
       PUT(NEXT(prevp),nextp);
   }
   PUT(NEXT(p),0);
   PUT(PREV(p),0);
}


 void place(void *bp,size_t asize)
{
   size_t csize = GET_SIZE(HDRP(bp));
   fix(bp);
   if((csize-asize)>=(2*DSIZE))
   {
       PUT(HDRP(bp),PACK(asize,1));
       PUT(FTRP(bp),PACK(asize,1));
       bp = NEXT_BLKP(bp);

       PUT(HDRP(bp),PACK(csize-asize,0));
       PUT(FTRP(bp),PACK(csize-asize,0));
       PUT(NEXT(bp),0);
       PUT(PREV(bp),0);
       coalesce(bp);
   }
   else
   {
       PUT(HDRP(bp),PACK(csize,1));
       PUT(FTRP(bp),PACK(csize,1));
   }
}


