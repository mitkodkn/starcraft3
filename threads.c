#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "controls.h"
#include "types.h"

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
//						printf("adding minerals...\n new amount: %d\n", args.center->minerals);
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
