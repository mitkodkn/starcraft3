//--------------------------------------------
// NAME: Dimitar Nikolov
// CLASS: XIa
// NUMBER: 7
// PROBLEM: #3
// FILE NAME: main.c
// FILE PURPOSE:
//	The whole game in one file (for now)
//---------------------------------------------

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "threads.h"
#include "types.h"
#include "utility.h"

int main(int argc, char const *argv[]) {

	int error;

	map_t map;
	error = initialize_map(&map, argc > 1 ? (int) strtol(argv[1], (char **)NULL, 10) : 2);

	if (error != 0) {
		return 1;
	}

	command_center_t center;
	error = initialize_center(&center);
	if (error != 0) {
		return 1;
	}

	pthread_t *scvs = (pthread_t*) malloc(sizeof(pthread_t) * center.scv);
	if (scvs == NULL) {
		perror("malloc");
		return 1;
	}

	center.scvs = scvs;

	pthread_t controller;

	struct arguments *controller_args = (struct arguments*) malloc(sizeof(struct arguments));

	if (controller_args == NULL) {
		perror("malloc");
		return 1;
	}

	controller_args->center = &center;
	controller_args->map = &map;

	error = pthread_create(&controller, NULL, control, (void*) controller_args);
	if (error != 0) {
		perror("pthread_create");
		return 1;
	}

	for (int scv_id = 0; scv_id < center.scv; scv_id++) {
		struct arguments *dig_args = (struct arguments*) malloc(sizeof(struct arguments));

		if (dig_args == NULL) {
			perror("malloc");
			return 1;
		}

		dig_args->id = scv_id;
		dig_args->center = &center;
		dig_args->map = &map;

		error = pthread_create(&scvs[scv_id], NULL, dig, (void*) dig_args);
		if (error != 0) {
			perror("pthread_create");
			return 1;
		}
	}

	error = pthread_join(controller, NULL);
	if (error != 0) {
		perror("pthread_join");
		return 1;
	}

	for (int scv_id = 0; scv_id < center.scv; scv_id++) {
		error = pthread_join(scvs[scv_id], NULL);
		if (error != 0) {
			fprintf(stderr, "%d: %s\n", scv_id, strerror(errno));
			return 1;
		}
	}

	printf("Map minerals %d, player minerals %d, SCVs %d, Marines %d\n", map.start_minerals, center.minerals, center.scv, center.marines);

	for (int i = 0; i < map.mineral_blocks_count; i++) {
		error = pthread_mutex_destroy(&map.mineral_blocks[i].digging);
		if (error != 0) {
			perror("pthread_mutex_destroy");
			return 1;
		}
	}

	error = pthread_mutex_destroy(&map.digging);
	if (error != 0) {
		perror("pthread_mutex_destroy");
		return 1;
	}

	free(map.mineral_blocks);
	free(scvs);

	return 0;
}
