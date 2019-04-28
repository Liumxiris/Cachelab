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
//void printSummary(int hit,int miss, int eviction);
int needEviction(Cache* cache, int set);

/*initialize arguments*/
int miss;
int hit;
int eviction;

int main(int argc, char* argv[]){
	puts("running");
	miss = hit = eviction = 0;
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

	FILE *fp = fopen(args->t, "r");
	char str[80];

	while(fgets(str,80,fp)!=NULL) {
		char op[10];
		unsigned long addr = -1;int size = -1;
		sscanf(str," %c %lx,%d", op, &addr, &size);
		printf("%s, %lx ", op,addr);
		if (strcmp(op, "I") == 0) continue;
		if (strcmp(op, "L") == 0)
			cacheLoad(&cache, args, addr);
		else if (strcmp(op, "M") == 0){
			cacheModify(&cache, args, addr);
		}
		else if (strcmp(op, "S") == 0)
			cacheLoad(&cache, args, addr);
		printf("\n");
	}
	fclose(fp);
	printSummary(hit, miss, eviction);
	return 0;
}
void cacheModify(Cache* cache, Arguments* args, unsigned long addr){
	cacheLoad(cache,args,addr);
	cacheLoad(cache,args,addr);
}

void cacheLoad(Cache* cache, Arguments* args, unsigned long addr){
	int tag = getTag(addr, args);
	int set = getSet(addr, args);
	int missIdx = isMiss (cache, addr, args);
	if( missIdx == -1){ //miss
		miss++;
		printf("Miss ");
		int evicIdx = needEviction(cache, set);
		if (evicIdx == -1){//need eviction
			printf("Eviction ");
			eviction++;
			int idx = getLeastLine(cache, set);//find lru index
			cache->sets[set].lines[idx].counter = 0;
			updateCache (cache, tag, set,idx, addr);
		}
		else{ //doesn't need eviction
			//cache->sets[set].lines[evicIdx].counter = 0;
			updateCache (cache, tag, set,evicIdx, addr);
		}
	}
	else{ // hit
		hit++;
		printf("Hit ");
		updateCache (cache,tag, set, missIdx, addr);
	}
}

void updateCache (Cache* cache, int tag, int set, int idx, unsigned long addr) {
	cache->sets[set].lines[idx].tag_bit = tag;
	cache->sets[set].lines[idx].valid_bit = 1;
	cache->sets[set].lines[idx].counter+=1;
}

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

int needEviction(Cache* cache, int set){
	int flag = -1;
	for (int i = 0; i < (cache->num_line); i++) {
	   Cacheline line = (cache -> sets[set]).lines[i];
	   if (line.valid_bit == 0){
		   flag = i;
		   break;
	   }
	}
	 return flag;
}

int getLeastLine(Cache* cache, int setIdx) {
    int minIdx = -1;
    int min = (cache -> sets[setIdx]).lines[0].counter;
        for (int j = 0; j < (cache->num_line); j++) {
            int count = (cache -> sets[setIdx]).lines[j].counter;
            if (count <= min) {
                min = count;
                minIdx = j;
            }
    }
    return minIdx;
}

int getTag(unsigned long address, Arguments* args){
	int mask = args->b+args->s;
	int tag = address>>mask;
	return tag;
}

int getSet(unsigned long address, Arguments* args){
	int addr = (address) >> (args->b);
	int mask = (1<<(args->s))-1;
	return addr & mask;
}

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
			args->t = (char*)malloc(sizeof(char)*strlen(optarg));
			strcpy(args->t, optarg);
			break;
		default:
			printf("Wrong Argument!");
		}
	}
}

void usage(){
puts("Usage: [-hv] -s <s> -E <E> -b <b> -t <tracefile>");
}

void initCache(Arguments *args, Cache* cache){
	cache->num_set = (1<<args->s); //2^s
	cache->num_line = args->E; //E lines per set
	cache->sets = (Cacheset*)malloc(cache->num_set*sizeof(Cacheset));

	int x,y;
	for (x = 0; x < cache->num_set; x++){
		cache->sets[x].lines = (Cacheline*)malloc((args->E)*sizeof(Cacheline));
		for (y = 0; y < cache->num_line; y++){
			cache->sets[x].lines[y].valid_bit = 0;
			cache->sets[x].lines[y].tag_bit = -1;
			cache->sets[x].lines[y].counter = 0;
		}
	}
}
