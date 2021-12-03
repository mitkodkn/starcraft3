#ifndef TYPES_H_
#define TYPES_H_

#include <pthread.h>

typedef struct mineral_block {
	int minerals;
	pthread_mutex_t digging;
} mineral_block_t;

typedef struct map {
	int start_minerals;
	int minerals;
	int mineral_blocks_count;
	pthread_mutex_t digging;
	mineral_block_t *mineral_blocks;
} map_t;

typedef struct command_center {
	int scv;
	int marines;
	int minerals;
	pthread_t *scvs;
	pthread_mutex_t active;
} command_center_t;

struct arguments {
	int id;
	map_t *map;
	command_center_t *center;
};

#endif /* TYPES_H_ */
