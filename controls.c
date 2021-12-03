#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "threads.h"
#include "types.h"

void construct_marine(command_center_t *center) {
	if (center->minerals >= 50) {
		sleep(1);
		center->minerals -= 50;
//		printf("removing minerals...\n new amount: %d\n", center->minerals);
		center->marines += 1;
//		printf("adding marine...\n new amount: %d\n", center->marines);
		printf("You wanna piece of me, boy?\n");
	} else {
		printf("Not enough minerals.\n");
	}
}

int train_scv(command_center_t *center, map_t *map) {
	int error;

	if (center->minerals >= 50) {
		sleep(4);
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
