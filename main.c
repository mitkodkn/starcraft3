//--------------------------------------------
// NAME: Dimitar Nikolov
// CLASS: XIa
// NUMBER: 7
// PROBLEM: #3
// FILE NAME: main.c
// FILE PURPOSE:
//	The whole game in one file
//---------------------------------------------

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

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

int initialize_map(map_t*, const int);
int initialize_center(command_center_t*);
void construct_marine(command_center_t*);
int train_scv(command_center_t*, map_t*);

void *control(void*);
void *dig(void*);

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

void *control(void* arg) {
	int error;

	struct arguments *args_p = (struct arguments*) arg;
	struct arguments args = *args_p;

	char buffer[2];

	while (args.center->marines < 20) {

		fgets(buffer, 2, stdin);

		if (strcmp(buffer, "m") == 0) {
			while (pthread_mutex_trylock(&args.center->active));
			construct_marine(args.center);
			pthread_mutex_unlock(&args.center->active);
		} else if (strcmp(buffer, "s") == 0) {
			while (pthread_mutex_trylock(&args.center->active));
			error = train_scv(args.center, args.map);

			if (error != 0) {
				return (void*)1;
			}
			pthread_mutex_unlock(&args.center->active);
		}
	}

	free(args_p);

	return NULL;
}

void *dig(void* arg) {
	struct arguments *args_p = (struct arguments*) arg;
	struct arguments args = *args_p;

	int amount_added = 0;

	while (args.map->minerals > 0) {
		for (int i = 0; i < args.map->mineral_blocks_count && args.map->minerals > 0; i++) {
			if (args.map->mineral_blocks[i].minerals > 0) {
//				sleep(3);
				if (pthread_mutex_trylock(&args.map->mineral_blocks[i].digging) == 0) {
					printf("SCV %d is mining from mineral block %d\n", args.id + 1, i + 1);

					if (args.map->mineral_blocks[i].minerals > 0) {
						if (args.map->mineral_blocks[i].minerals - 8 <= 0) {
							amount_added = args.map->mineral_blocks[i].minerals;
							args.map->mineral_blocks[i].minerals = 0;
						} else {
							amount_added = 8;
							args.map->mineral_blocks[i].minerals -= amount_added;
						}

						pthread_mutex_unlock(&args.map->mineral_blocks[i].digging);

						while (pthread_mutex_trylock(&args.map->digging));
						args.map->minerals -= amount_added;
						pthread_mutex_unlock(&args.map->digging);

						printf("SCV %d is transporting minerals\n", args.id + 1);
	//					sleep(2);

						while (pthread_mutex_trylock(&args.center->active));
						args.center->minerals += amount_added;
						printf("adding minerals...\n new amount: %d\n", args.center->minerals);
						pthread_mutex_unlock(&args.center->active);
						printf("SCV %d delivered minerals to the Command center\n", args.id + 1);
					}
				}
			}
		}
	}

	free(args_p);

	return NULL;
}

void construct_marine(command_center_t *center) {
	if (center->minerals >= 50) {
//		sleep(1);
		center->minerals -= 50;
		printf("removing minerals...\n new amount: %d\n", center->minerals);
		center->marines += 1;
		printf("adding marine...\n new amount: %d\n", center->marines);
		printf("You wanna piece of me, boy?\n");
	} else {
		printf("Not enough minerals.\n");
	}
}

int train_scv(command_center_t *center, map_t *map) {
	int error;

	if (center->minerals >= 50) {
//		sleep(4);
		center->minerals -= 50;
		center->scv += 1;
//		printf("removing minerals...\n new amount: %d\n", center->minerals);
//		printf("adding scv...\n new amount: %d\n", center->scv);

		center->scvs = (pthread_t*) realloc(center->scvs, sizeof(pthread_t) * center->scv);

		if (center->scvs == NULL) {
			perror("realloc");
			return 1;
		}

		struct arguments *dig_args = (struct arguments*) malloc(sizeof(struct arguments));

		if (dig_args == NULL) {
			perror("malloc");
			return 1;
		}

		dig_args->id = center->scv - 1;
		dig_args->center = center;
		dig_args->map = map;
//
		error = pthread_create(&center->scvs[center->scv - 1], NULL, dig, (void*) dig_args);
		if (error != 0) {
			perror("pthread_create");
			return 1;
		}

		error = pthread_join(center->scvs[center->scv - 1], NULL);
		if (error != 0) {
			perror("pthread_join");
			return 1;
		}

		printf("SCV good to go, sir.\n");
	} else {
		printf("Not enough minerals.\n");
	}

	return 0;
}
