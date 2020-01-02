#include "cachelab.h"
#include "dogfault.h"


int hit = 0, miss = 0, eviction = 0 ,LF=0;
void ans(Cache *cache,int address,int size,int set,int tag,int Verbose);

int main(int argc,char **argv)
{
    int s,E,b,Verbose,address,size;
    char tracefileName[90],instruction[30];
    Cache cache;
    readf(argc,argv,&s,&E,&b,tracefileName,&LF, &Verbose);
    init_Cache(s,E,b,&cache);
    FILE *tracefile = fopen(tracefileName,"r");


    while(fscanf(tracefile,"%s %x,%d",instruction,&address,&size) != EOF){
    	int set = (address>>b) & ((1<<s)-1);
        int tag = address >> (s+b);
        
        if(strcmp(instruction,"I")==0)continue;

        if(Verbose == 1) printf("%s %x,%d ",instruction,address,size);

        if(strcmp(instruction,"S")==0) {
			ans(&cache,address,size,set,tag,Verbose);
        }

        if(strcmp(instruction,"M")==0) {
        	ans(&cache,address,size,set,tag,Verbose);
        	ans(&cache,address,size,set,tag,Verbose);
        }

        if(strcmp(instruction,"L")==0) {
			ans(&cache,address,size,set,tag,Verbose);
        }

        if(Verbose == 1) printf("\n");
    }

    printSummary(hit, miss, eviction);
    return 0;
}


void ans(Cache *cache,int address,int size,int set,int tag,int Verbose){
	int check_hit = 1;
    for(int i=0;i<cache->line_number;i++){
        if(cache->sets[set].lines[i].valid == 1 && cache->sets[set].lines[i].tag == tag){
        	//printf("%d %d %d\n",set,i,cache->sets[set].lines[i]);
            check_hit = 0;
            lru(cache,set,i);
        }
    }

    if(check_hit==1){ 
        if(Verbose == 1) printf("miss ");
        	if(LF==1){
		        if(write_cache(cache,set,tag) == 1){
		            eviction++;
		            if(Verbose==1) printf("eviction ");
		        }
		      }
		     else{
		     	if(write_cachelf(cache,set,tag) == 1){
		            eviction++;
		            if(Verbose==1) printf("eviction ");
		        }
		     }
       	miss++;
    }
    else {
       hit++;
       if(Verbose == 1) printf("hit ");
    }
}




