/* 
 * cachelab.h - Prototypes for Cache Lab helper functions
 */

#ifndef CACHELAB_TOOLS_H
#define CACHELAB_TOOLS_H

/* 
 * printSummary - This function provides a standard way for your cache
 * simulator * to display its final hit and miss statistics
 */ 

typedef struct{
    int valid;    
    int tag;         
    int lru;  
} line;

typedef struct{
    line* lines; 
    int fifo;  
} set;

typedef struct {
    int set_number;    
    int line_number;   
    set* sets;      
} Cache;

void printSummary(int hits,  /* number of  hits */
				  int misses, /* number of misses */
				  int evictions);

void readf(int argc,char **argv,int *s,int *E,int *b,char *tracefileName,int *LF,int *Verbose);  


int init_Cache(int s,int E,int b,Cache *cache);


void lru(Cache *cache,int set,int i);


int write_cache(Cache *cache,int set,int tag);

int write_cachelf(Cache *cache,int set,int tag);

#endif /* CACHELAB_TOOLS_H */
