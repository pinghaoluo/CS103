/*
 * cachelab.c - Cache Lab helper functions
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "cachelab.h"
#include <time.h>
#include <getopt.h>
#include <unistd.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
/* 
 * printSummary - Summarize the cache simulation statistics. Student cache simulators
 *                must call this function in order to be properly autograded. 
 */
void printSummary(int hits, int misses, int evictions)
{
    printf("hits:%d misses:%d evictions:%d\n", hits, misses, evictions);
    FILE* output_fp = fopen(".csim_results", "w");
    assert(output_fp);
    fprintf(output_fp, "%d %d %d\n", hits, misses, evictions);
    fclose(output_fp);
}


int init_Cache(int s,int E,int b,Cache *cache){
	cache->set_number = 2<<s;
	cache->line_number = E;
	cache->sets = (set *)malloc(cache->set_number * sizeof(set));

    for(int i=0; i< cache->set_number; i++)
    {
        cache->sets[i].lines = (line *)malloc(E*sizeof(line));
        cache->sets[i].fifo=0;
        for(int j=0; j < E; j++){
            cache->sets[i].lines[j].valid = 0;
            cache->sets[i].lines[j].lru = 0;
        }
    }
    return 1;
}

void readf(int argc,char **argv,int *s,int *E,int *b,char *tracefileName,int *LF,int *Verbose){
    int c;
    while((c = getopt(argc,argv,"vFLs:E:b:t:"))!=-1)
    {
        switch(c)
        {
        case 'v':
            *Verbose = 1;
            break;
        case 'F':
            *LF = 0;
            break;
        case 'L':
            *LF = 1;
            break;
        case 's':
            *s = atoi(optarg);
            break;
        case 'E':
            *E = atoi(optarg);
            break;
        case 'b':
            *b = atoi(optarg);
            break;
        case 't':
            strcpy(tracefileName,optarg);
            break;
        }
    }
    return;
}



void lru(Cache *cache,int set,int i){
        cache->sets[set].lines[i].lru = 500;
        //printf("%d\n", set);
        for(int j=0;j<cache->line_number;j++){
            if(j!=i) cache->sets[set].lines[j].lru--;
        }
}


int write_cache(Cache *cache,int set,int tag){
	int full = 1;
	int i =0;
    for(i=0;i<cache->line_number;i++){
    	//printf(" %d %d %d %d %d %d\n",set,i,cache->sets[set].lines[i].valid,cache->sets[set].lines[i].tag,tag,cache->sets[set].lines[i].lru);
        if(cache->sets[set].lines[i].valid == 0){
        	full = 0;
            break;
        }
    }
    if(full == 0){
        cache->sets[set].lines[i].valid = 1;
        cache->sets[set].lines[i].tag = tag;
        lru(cache,set,i);
    }else{
	    int ei=0;
	    int minLru = 500;
	    for(int i=0;i<cache->line_number;i++){
	        if(cache->sets[set].lines[i].lru < minLru){
	            ei = i;
	            minLru = cache->sets[set].lines[i].lru;
	        }
	    }
        cache->sets[set].lines[ei].tag = tag;
        lru(cache,set,ei);
    }
    return full;
}

int write_cachelf(Cache *cache,int set,int tag){
	int full = 1;
	int i =0;
    for(i=0;i<cache->line_number;i++){
    	//printf(" %d %d %d %d %d %d\n",set,i,cache->sets[set].lines[i].valid,cache->sets[set].lines[i].tag,tag,cache->sets[set].lines[i].lru);
        if(cache->sets[set].lines[i].valid == 0){
        	full = 0;
            break;
        }
    }
    if(full == 0){
        cache->sets[set].lines[i].valid = 1;
        cache->sets[set].lines[i].tag = tag;
    }else{
    	int fifo =  cache->sets[set].fifo;
        cache->sets[set].lines[fifo].tag = tag;
        if(fifo == (cache->line_number-1)){
        	cache->sets[set].fifo = 0;
        }
        else{
        	cache->sets[set].fifo++;
        }
    }
    return full;
}

