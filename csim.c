/* Mingxi Liu*/

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
	char* op;
	int addr;
	int size;
}Operation;

typedef struct{
	int h; //optional help flag that prints usage info
	int v; //optional verbose flag that displays trace info
	int s; //number of set index bits 2^s
	int E; //number of lines per set
	int b; //number of block bits 2^b
	const char*t; //name of valgrind
}Arguments;

/*function proto*/
int getTag(Operation *op, Arguments* args);
int getSet(Operation *op, Arguments* args);
int get_arg(int argc, char* argv[], Arguments *args);
void usage();
void initCache(Arguments *args, Cache* cache);
void parseTrace(Operation *op, char* tracename);

/*initialize arguments*/



int main(int agrc, char* argv[]){
	int miss,hit,eviction = 0;

	Cache cache;
	Arguments *args;
	Operation *op;


	args = malloc(sizeof(Arguments));
	int h,v,s,E,b = 0;
	char*t;
	args->h = h; args->v = v;
	args->s = s; args->E = E;
	args->b = b;
	get_arg(argc, argv, args);
	initCache(args, &cache);
	parseTrace(op, args->t);
	//    printSummary(0, 0, 0);
	return 0;
}

int getTag(Operation *op, Arguments* args){
	int mask = args->b+args->s;
	int tag = (op->addr)>>mask;
	return tag;
}

int getSet(Operation *op, Arguments* args){
	int addr = (op->addr) >> (args->b);
	int mask = (1<<(args->s))-1;
	return addr & mask;
}

int get_arg(int argc, char* argv[], Arguments *args){
	int c = 0;
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
	cache->num_set = (2<<args->s); //2^s
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

void parseTrace(Operation *operation, const char* tracename){
	FILE *fp = fopen(tracename, "rt");
	char op;
	int addr,size;
	int index = 0;

	while(fscanf(fp, "%c %x, %d", &op, &addr, &size) != EOF) {
		if (strcmp(*op, 'I') == 0) //skip I operation
			continue;
		operation[index].op = op;
		operation[index].size = size;
		operation[index].addr = addr;
		index++;
	}
	fclose(fp);
}
