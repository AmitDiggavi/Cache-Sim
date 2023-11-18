////////////////////////////////////////////////////////////////////////////////
// Main File:        csim.c
// This File:        csim.c
// Other Files:      N/A
// Semester:         CS 354 Lecture 00? Fall 2023
// Grade Group:      gg 14  (See canvas.wisc.edu/groups for your gg#)
// Instructor:       deppeler
// 
// Author:           Amit Diggavi
// Email:            diggavi@wisc.edu
// CS Login:         diggavi
//
///////////////////////////  WORK LOG  //////////////////////////////
//  Document your work sessions on your copy http://tiny.cc/work-log
//  Download and submit a pdf of your work log for each project.
/////////////////////////// OTHER SOURCES OF HELP ////////////////////////////// 
// Persons:          Identify persons by name, relationship to you, and email.
//                   Describe in detail the the ideas and help they provided.
//
// Online sources:   avoid web searches to solve your problems, but if you do
//                   search, be sure to include Web URLs and description of 
//                   of any information you find.
// 
// AI chats:         save a transcript and submit with project.
//////////////////////////// 80 columns wide ///////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2013,2019-2020
// Posting or sharing this file is prohibited, including any changes/additions.
// Used by permission for Fall 2023
//
////////////////////////////////////////////////////////////////////////////////

/*
 * csim.c:  
 * A cache simulator that can replay traces (from Valgrind) and output
 * statistics for the number of hits, misses, and evictions.
 * The replacement policy is LRU.
 *
 * Implementation and assumptions:
 *  1. Each load/store can cause at most one cache miss plus a possible eviction.
 *  2. Instruction loads (I) are ignored.
 *  3. Data modify (M) is treated as a load followed by a store to the same
 *  address. Hence, an M operation can result in two cache hits, or a miss and a
 *  hit plus a possible eviction.
 */  

#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

/******************************************************************************/
/* DO NOT MODIFY THESE VARIABLES **********************************************/

//Globals set by command line args.
int b = 0; //number of (b) bits
int s = 0; //number of (s) bits
int E = 0; //number of lines per set

//Globals derived from command line args.
int B; //block size in bytes: B = 2^b
int S; //number of sets: S = 2^s

//Global counters to track cache statistics in access_data().
int hit_cnt = 0;
int miss_cnt = 0;
int evict_cnt = 0;

//Global to control trace output
int verbosity = 0; //print trace if set
/******************************************************************************/


//Type mem_addr_t: Use when dealing with addresses or address masks.
typedef unsigned long long int mem_addr_t;

//Type cache_line_t: Use when dealing with cache lines.
//TODO - COMPLETE THIS TYPE
typedef struct cache_line {                    
	char valid;
	mem_addr_t tag;
	//Add a data member as needed by your implementation for LRU tracking.
	int track_LRU;
} cache_line_t;

//Type cache_set_t: Use when dealing with cache sets
//Note: Each set is a pointer to a heap array of one or more cache lines.
typedef cache_line_t* cache_set_t;

//Type cache_t: Use when dealing with the cache.
//Note: A cache is a pointer to a heap array of one or more sets.
typedef cache_set_t* cache_t;

// Create the cache we're simulating. 
//Note: A cache is a pointer to a heap array of one or more cache sets.
cache_t cache;  

/* TODO - COMPLETE THIS FUNCTION
 * init_cache:
 * Allocates the data structure for a cache with S sets and E lines per set.
 * Initializes all valid bits and tags with 0s.
 */                    
void init_cache() {          

	S = pow(2,s); // using the s bits to initlaize S. 2^s if s = 2 there is 4 sets. 
	B = pow(2, b); // using the b bits to initlaize B. 2^b if b = 5 it is 32 bytes. 
	
	cache = malloc(S * sizeof(cache_set_t)); // allocating space for S sets
	
	for(int i = 0; i < S; i++) // iterating through the sets and allocating space for the lines
	{
		cache[i] = malloc(E * sizeof(cache_line_t)); // allocating space for each line in the set 
		if (cache[i] == NULL) 
		{
			printf("Error allocating memory\n");
			exit(1);
		}
			
		for(int j = 0; j < E; j++) // iterating through lines in the set
		{
			cache[i][j].valid = 0; // setting valid bits to 0 in each line. 
			cache[i][j].tag = 0; // setting tag bits to 0 in each line
			cache[i][j].track_LRU = 0;
		}
	}

}


/* TODO - COMPLETE THIS FUNCTION 
 * free_cache:
 * Frees all heap allocated memory used by the cache.
 */                    
void free_cache() {             
	for(int i = 0; i < S; i++)
	{
		free(cache[i]); 
	}
	free(cache);

}


/* TODO - COMPLETE THIS FUNCTION 
 * access_data:
 * Simulates data access at given "addr" memory address in the cache.
 *
 * If already in cache, increment hit_cnt
 * If not in cache, cache it (set tag), increment miss_cnt
 * If a line is evicted, increment evict_cnt
 */                    
void access_data(mem_addr_t addr) {      
	
	S = pow(2,s);
//	B = pow(2,b);
	
	mem_addr_t t = (addr >> (s+b)); // extracting the t-bit
	
	mem_addr_t s_index = ((addr >> b) & (S - 1));// extracting s-bit
	


	for(int i = 0; i < E; i++) // if in cache, inc hit_cnt
	{
		// valid and tags match, its a hit
		if(cache[s_index][i].valid && cache[s_index][i].tag == t)
		{
			hit_cnt++; // in cache, so hit 
			
			//int older_track = cache[s_index][i].track_LRU; // storing old one
			cache[s_index][i].track_LRU = 0; // setting to 0 because it is most recent

			for(int j = 0; j < E; j++) // iterating through lines in the set to update lru as needed
			{	
				//for each line valid and not the hit line, we increment the rest
				if(cache[s_index][j].valid == 1 && (i != j)) 
				{
					cache[s_index][j].track_LRU++; // keeping order of LRU
				}
			}
			return;
		}
		/*
		// if valid and based on count is the least recently used
		if(cache[s_index][i].valid == 1 && cache[s_index][i].track_LRU > highest)
		{
			highest = cache[s_index][i].track_LRU; // highest is least recently used. 
			// higher the count, the least recently it is used
			lru_index = i; // this is the least recently yline
		}*/	
	}

	
	miss_cnt++; // not in cache, so miss
		
	
	// finding empty place cache
	for(int i = 0; i < E; i++)
	{
		if(cache[s_index][i].valid == 0)
		{
			cache[s_index][i].valid = 1;
			cache[s_index][i].tag = t;
			cache[s_index][i].track_LRU = 0;//update LRU
			
			for(int j = 0; j < E; j++) // updating LRU for all lines, just not the ith one
			{	
				// if we valid and not looking at the line just accessed(i)
				if(i != j && cache[s_index][j].valid == 1) 
				{
					cache[s_index][j].track_LRU++; // keeping order of LRU 
				}
			}
			return;
		}	
	}
	



	// all lines are full so evict needed.
	evict_cnt++;
	
	int lru_index = 0;
	int highest = cache[s_index][0].track_LRU;

	for(int i = 0; i < E; i++)
	{	
		// if valid and based on count is the least recently used
		if(cache[s_index][i].valid == 1 && cache[s_index][i].track_LRU > highest)
		{
			highest = cache[s_index][i].track_LRU; // highest is least recently used.
							       // higher the count, the least recently it is used
			lru_index = i; // this is the least recently yline
		}
	}
	// all lines are full so evict needed. 
	//evict_cnt++;
	
	cache[s_index][lru_index].tag = t; // replacing the least recently used line with new tag
	cache[s_index][lru_index].track_LRU = 0; // just like above, set this to 0 since it is the most recently used line. 
	
	for(int i = 0; i < E; i++)
	{
		if((i != lru_index) && cache[s_index][i].valid == 1)
		{
			cache[s_index][i].track_LRU++; // keeping order of LRU
		}
	}


}


/* TODO - FILL IN THE MISSING CODE
 * replay_trace:
 * Replays the given trace file against the cache.
 *
 * Reads the input trace file line by line.
 * Extracts the type of each memory access : L/S/M
 * TRANSLATE each "L" as a load i.e. 1 memory access
 * TRANSLATE each "S" as a store i.e. 1 memory access
 * TRANSLATE each "M" as a load followed by a store i.e. 2 memory accesses 
 */                    
void replay_trace(char* trace_fn) {           
	char buf[1000];  
	mem_addr_t addr = 0;
	unsigned int len = 0;
	FILE* trace_fp = fopen(trace_fn, "r"); 

	if (!trace_fp) { 
		fprintf(stderr, "%s: %s\n", trace_fn, strerror(errno));
		exit(1);   
	}

	while (fgets(buf, 1000, trace_fp) != NULL) {
		if (buf[1] == 'S' || buf[1] == 'L' || buf[1] == 'M') {
			sscanf(buf+3, "%llx,%u", &addr, &len);

			if (verbosity)
				printf("%c %llx,%u ", buf[1], addr, len);

			// TODO - MISSING CODE
			// GIVEN: 1. addr has the address to be accessed
			//        2. buf[1] has type of acccess(S/L/M)
			// call access_data function here depending on type of access
			
			if(buf[1] == 'M')
			{
				access_data(addr);
			}
			access_data(addr);
			if (verbosity)
				printf("\n");
		}
	}

	fclose(trace_fp);
}  


/*
 * print_usage:
 * Print information on how to use csim to standard output.
 */                    
void print_usage(char* argv[]) {                 
	printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
	printf("Options:\n");
	printf("  -h         Print this help message.\n");
	printf("  -v         Verbose flag.\n");
	printf("  -s <num>   Number of s bits for set index.\n");
	printf("  -E <num>   Number of lines per set.\n");
	printf("  -b <num>   Number of b bits for word and byte offsets.\n");
	printf("  -t <file>  Trace file.\n");
	printf("\nExamples:\n");
	printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
	printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
	exit(0);
}  


/*
 * print_summary:
 * Prints a summary of the cache simulation statistics to a file.
 */                    
void print_summary(int hits, int misses, int evictions) {                
	printf("hits:%d misses:%d evictions:%d\n", hits, misses, evictions);
	FILE* output_fp = fopen(".csim_results", "w");
	assert(output_fp);
	fprintf(output_fp, "%d %d %d\n", hits, misses, evictions);
	fclose(output_fp);
}  


/*
 * main:
 * Main parses command line args, makes the cache, replays the memory accesses
 * free the cache and print the summary statistics.  
 */                    
int main(int argc, char* argv[]) {                      
	char* trace_file = NULL;
	char c;

	// Parse the command line arguments: -h, -v, -s, -E, -b, -t 
	while ((c = getopt(argc, argv, "s:E:b:t:vh")) != -1) {
		switch (c) {
			case 'b':
				b = atoi(optarg);
				break;
			case 'E':
				E = atoi(optarg);
				break;
			case 'h':
				print_usage(argv);
				exit(0);
			case 's':
				s = atoi(optarg);
				break;
			case 't':
				trace_file = optarg;
				break;
			case 'v':
				verbosity = 1;
				break;
			default:
				print_usage(argv);
				exit(1);
		}
	}

	//Make sure that all required command line args were specified.
	if (s == 0 || E == 0 || b == 0 || trace_file == NULL) {
		printf("%s: Missing required command line argument\n", argv[0]);
		print_usage(argv);
		exit(1);
	}

	//Initialize cache.
	init_cache();

	//Replay the memory access trace.
	replay_trace(trace_file);

	//Free memory allocated for cache.
	free_cache();

	//Print the statistics to a file.
	//DO NOT REMOVE: This function must be called for test_csim to work.
	print_summary(hit_cnt, miss_cnt, evict_cnt);
	return 0;   
}  

// Fall 202309

