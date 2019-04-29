/* Mingxi Liu
 * Kepei Lei*/

#include "cachelab.h"
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct{
	int valid_bit;
	int tag_bit;
	int counter;
}Cacheline;

typedef struct{
	Cacheline* lines;
}Cacheset;

typedef struct{
	int num_line;
	int num_set;
	Cacheset* sets;
}Cache;

typedef struct{
	int h; //optional help flag that prints usage info
	int v; //optional verbose flag that displays trace info
	int s; //number of set index bits 2^s
	int E; //number of lines per set
	int b; //number of block bits 2^b
	char*t; //name of valgrind
}Arguments;

/*function proto*/
int getTag(unsigned long addr, Arguments* args);
int getSet(unsigned long addr, Arguments* args);
void get_arg(int argc, char* argv[], Arguments *args);
void usage();
void initCache(Arguments *args, Cache* cache);
void cacheModify(Cache* cache, Arguments* args, unsigned long addr);
void cacheLoad(Cache* cache, Arguments* args, unsigned long addr);
void updateCache (Cache* cache, int tag, int set, int idx, unsigned long addr);
int isMiss (Cache* cache, unsigned long addr, Arguments* args);
int getLeastLine(Cache* cache, int setIdx);
int needEviction(Cache* cache, int set);

/*initialize arguments*/
int miss;
int hit;
int eviction;
int RECENT_USE;

int main(int argc, char* argv[]){
	//initialize arguments
	miss = hit = eviction = RECENT_USE = 0;
	Cache cache;
	Arguments *args;

	args = malloc(sizeof(Arguments));
	int h = 0;
	int v = 0;
	int s = 0;
	int E = 0;
	int b = 0;
	args->h = h; args->v = v;
	args->s = s; args->E = E;
	args->b = b;
	get_arg(argc, argv, args);
	initCache(args, &cache);

	if (args->h == 1)
		usage();
	//open the trace file
	FILE *fp = fopen(args->t, "r");
	char str[80];

	while(fgets(str,80,fp)!=NULL) {
		char op[10];
		unsigned long addr = -1;int size = -1;
		//match lines with pattern[space][operation][space][address],[size]
		sscanf(str," %c %lx,%d", op, &addr, &size);
		//print trace if argument v is set
		if (args->v == 1)
			printf("%s, %lx ", op,addr);
		//ignore operation I
		if (strcmp(op, "I") == 0) continue;
		if (strcmp(op, "L") == 0)
			cacheLoad(&cache, args, addr);
		else if (strcmp(op, "M") == 0){
			cacheModify(&cache, args, addr);
		}
		else if (strcmp(op, "S") == 0)
			cacheLoad(&cache, args, addr);
		RECENT_USE++;
		if (args->v == 1)
			printf("\n");
	}
	fclose(fp);
	printSummary(hit, miss, eviction);
	return 0;
}
/**
 * when opration is modify, calls cacheload twice
 * @param Cache* cache
 * @param Arguments* agrs, user inputs
 * @param unsigned long addr, address of data
 * @return void
 */
void cacheModify(Cache* cache, Arguments* args, unsigned long addr){
	cacheLoad(cache,args,addr);
	cacheLoad(cache,args,addr);
}
/**
 * Load operation, update cache according to the address called on
 * @param Cache* cache
 * @param Arguments* agrs, user inputs
 * @param unsigned long addr, address of data
 * @return void
 */
void cacheLoad(Cache* cache, Arguments* args, unsigned long addr){
	//retrieve tag and set bits
	int tag = getTag(addr, args);
	int set = getSet(addr, args);
	//check whether there's a miss
	int missIdx = isMiss (cache, addr, args);
	if( missIdx == -1){ //miss
		miss++;
		if (args->v ==1)
			printf("Miss ");
		//check if needs eviction
		int evicIdx = needEviction(cache, set);
		if (evicIdx == -1){//need eviction
			if (args->v ==1)
				printf("Eviction ");
			eviction++;
			int idx = getLeastLine(cache, set);//find lru index
			cache->sets[set].lines[idx].counter = 0; //set the counter of evicted line to 0
			updateCache (cache, tag, set,idx, addr);
		}
		else{ //doesn't need eviction
			//cache->sets[set].lines[evicIdx].counter = 0;
			updateCache (cache, tag, set,evicIdx, addr);
		}
	}
	else{ // hit
		hit++;
		if (args->v ==1)
			printf("Hit ");
		updateCache (cache,tag, set, missIdx, addr);
	}
}
/**
 * update the cacheline:
 *   set the tag-bits to given value
 *   set the valid-bit to 1
 *   counter increment
 * @param Cache* cache
 * @param int tag
 * @param int set
 * @param int idx, the index of target cacheline
 * @param unsigned long addr, address of data
 * @return void
 */
void updateCache (Cache* cache, int tag, int set, int idx, unsigned long addr) {
	cache->sets[set].lines[idx].tag_bit = tag;
	cache->sets[set].lines[idx].valid_bit = 1;
	cache->sets[set].lines[idx].counter = RECENT_USE;
}
/**
 * check if there's a miss
 * @param Cache* cache
 * @param unsigned long addr, address of data
 * @param Arguments* agrs, user inputs
 * @return -1 if there's a miss
 *         otherwise there's a hit and return its index number
 */
int isMiss (Cache* cache, unsigned long addr, Arguments* args) {
	int tag = getTag(addr, args);
    int set = getSet(addr, args);
	for (int i = 0; i < (cache->num_line); i++) {
		if (((cache -> sets[set]).lines[i].tag_bit == tag)
			&&((cache -> sets[set]).lines[i].valid_bit==1))
				return i; //hit
	}
	return -1;
}
/**
 * check if eviction is needed
 * @param Cache* cache
 * @param int set
 * @return -1 if current cache set is full
 *         otherwise it return the index of available cacheline
 */
int needEviction(Cache* cache, int set){
	int flag = -1;
	for (int i = 0; i < (cache->num_line); i++) {
	   Cacheline line = (cache -> sets[set]).lines[i];
	   if (line.valid_bit == 0){ //if valid_bit is 0, this cacheline is available
		   flag = i;
		   break;
	   }
	}
	 return flag;
}
/**
 * return the index of least recently used cacheline
 * @param Cache* cache
 * @param int setIdx, index of given cache set
 * @return index of the LRU cacheline
 */
int getLeastLine(Cache* cache, int setIdx) {
    int minIdx = 0;
    int min = (cache -> sets[setIdx]).lines[0].counter;
        for (int j = 0; j < (cache->num_line); j++) {
            int count = (cache -> sets[setIdx]).lines[j].counter;
            //check if the counter of current cacheline is smaller than min
            if (count <= min) {
                min = count;
                minIdx = j;
            }
    }
    return minIdx;
}
/**
 * retrieve the tag bits of given address
 * @param unsigned long addr, address of data
 * @param Arguments* agrs, user inputs
 * @return int, tag bits
 */
int getTag(unsigned long address, Arguments* args){
	unsigned int mask = args->b+args->s;
	return address >> mask;
}
/**
 * retrieve the set bits of given address
 * @param unsigned long addr, address of data
 * @param Arguments* agrs, user inputs
 * @return int, set bits
 */
int getSet(unsigned long address, Arguments* args){
	unsigned int mask = (1<<(args->s+args->b))-(1<<(args->b));
	return (address & mask)>>args->b;
}
/**
 * read the arguments from user inputs
 * @param int argc, the number of user inputs
 * @param char* argv[]
 * @param Arguments* agrs, user inputs
 * @return void
 */
void get_arg(int argc, char* argv[], Arguments *args){
	char c = 0;
	while ((c = getopt(argc,argv, "hvs:E:b:t:")) != -1) {
		switch(c){
		case 'h':
			args->h = 1;
			break;
		case 'v':
			args->v = 1;
			break;
		case 's':
			args->s = atoi(optarg);
			break;
		case 'E':
			args->E = atoi(optarg);
			break;
		case 'b':
			args->b = atoi(optarg);
			break;
		case 't':
			//dynamically allocate memory for trace file name
			args->t = (char*)malloc(sizeof(char)*strlen(optarg));
			strcpy(args->t, optarg);
			break;
		default:
			printf("Wrong Argument!");
		}
	}
}
/**
 * display the usage information
 * @return int, tag bits
 */
void usage(){
puts("Options:");
puts("  -h         Print this help message.");
puts("  -v         Optional verbose flag.");
puts("  -s <num>   Number of set index bits.");
puts("  -E <num>   Number of lines per set.");
puts("  -b <num>   Number of block offset bits.");
puts("  -t <file>  Trace file.");
puts("Examples:");
puts("  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace");
puts("  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace");

}
/**
 * initiate the cache
 * @param Arguments* agrs, user inputs
 * @param Cache* cache
 * @return void
 */
void initCache(Arguments *args, Cache* cache){
	cache->num_set = (1<<args->s); //2^s
	cache->num_line = args->E; //E lines per set
	cache->sets = (Cacheset*)malloc(cache->num_set*sizeof(Cacheset));
	//initialize every cacheset and cacheline
	int x,y;
	for (x = 0; x < cache->num_set; x++){
		cache->sets[x].lines = (Cacheline*)malloc((args->E)*sizeof(Cacheline));
		for (y = 0; y < cache->num_line; y++){
			//set the default value for every cacheline
			cache->sets[x].lines[y].valid_bit = 0;
			cache->sets[x].lines[y].tag_bit = -1;
			cache->sets[x].lines[y].counter = 0;
		}
	}
}
