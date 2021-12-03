#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "types.h"

int initialize_map(map_t* map, const int blocks) {
	int error;

	map->mineral_blocks_count = blocks;
	map->mineral_blocks = (mineral_block_t*) malloc(sizeof(mineral_block_t) * map->mineral_blocks_count);

	if (map->mineral_blocks == NULL) {
		perror("malloc");
		return 1;
	}

	map->start_minerals = map->minerals = map->mineral_blocks_count * 500; // constant

	error = pthread_mutex_init(&map->digging, NULL);

	if (error != 0) {
		perror("pthread_mutex_init");
		return 1;
	}

	for (int i = 0; i < map->mineral_blocks_count; i++) {
		map->mineral_blocks[i].minerals = 500; // constant
		error = pthread_mutex_init(&map->mineral_blocks[i].digging, NULL);

		if (error != 0) {
			perror("pthread_mutex_init");
			return 1;
		}
	}

	return 0;
}

int initialize_center(command_center_t *center) {
	int error;

	center->scv = 5;
	center->marines = 0;
	center->minerals = 0;

	error = pthread_mutex_init(&center->active, NULL);

	if (error != 0) {
		perror("pthread_mutex_init");
		return 1;
	}

	return 0;
}
