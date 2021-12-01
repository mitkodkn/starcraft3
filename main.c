#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define INITIAL_SCV_COUNT 5
#define INITIAL_WARRIORS_COUNT 0
#define INITIAL_MINERALS_COUNT 0
#define MAP_MINERALS_PER_BLOCK 500
#define FINAL_WARRIORS_COUNT 20

void *work(void*);

typedef struct mineral_blocks {
	unsigned int minerals_count;
	pthread_mutex_t digging;
} mineral_block_t;

typedef struct command_centers {
	unsigned int SCV_count;
	unsigned int marines_count;
	unsigned int minerals_count;
	pthread_mutex_t delivery;
} command_center_t;

struct arguments {
	intptr_t SCV_id;
	intptr_t mineral_blocks_count;
	command_center_t *cmd_center;
};

mineral_block_t *mineral_blocks;

int main(int argc, char const *argv[]) {

	int error;
	void *result;

	const intptr_t mineral_blocks_count = argc > 1 ? (intptr_t) strtol(argv[1], (char **)NULL, 10) : 2;
	mineral_blocks = (mineral_block_t*) malloc(sizeof(mineral_block_t) * mineral_blocks_count);

	command_center_t cmd_center;

	cmd_center.SCV_count = INITIAL_SCV_COUNT;
	cmd_center.marines_count = INITIAL_WARRIORS_COUNT;
	cmd_center.minerals_count = INITIAL_MINERALS_COUNT;
	pthread_mutex_init(&cmd_center.delivery, NULL);

	if (mineral_blocks == NULL) {
		perror("malloc");
		return 1;
	}

	for (unsigned int i = 0; i < mineral_blocks_count; i++) {
		mineral_blocks[i].minerals_count = MAP_MINERALS_PER_BLOCK;
		pthread_mutex_init(&mineral_blocks[i].digging, NULL);
	}

	pthread_t *SCV = (pthread_t*) malloc(sizeof(pthread_t) * cmd_center.SCV_count);

	if (SCV == NULL) {
		perror("malloc");
		return 1;
	}

	for (intptr_t id = 0; id < cmd_center.SCV_count; id++) {
		struct arguments *args = (struct arguments*) malloc(sizeof(struct arguments));

		args->SCV_id = id;
		args->mineral_blocks_count = mineral_blocks_count;
		args->cmd_center = &cmd_center;

		error = pthread_create(&SCV[id], NULL, work, (void*) args);

		if (error != 0) {
			perror("pthread_create");
			return 1;
		}
	}

	for (unsigned int i = 0; i < cmd_center.SCV_count; i++) {
		void *result;

		error = pthread_join(SCV[i], &result);

		if (error != 0) {
			perror("pthread_join");
			return 1;
		}
	}

	for (unsigned int i = 0; i < mineral_blocks_count; i++) {
		pthread_mutex_destroy(&mineral_blocks[i].digging);
	}

	pthread_mutex_destroy(&cmd_center.delivery);

	free(mineral_blocks);
	free(SCV);

	printf("Map minerals %ld, player minerals %d, SCVs %d, Marines %d\n", mineral_blocks_count * 500, cmd_center.minerals_count, cmd_center.SCV_count, cmd_center.marines_count);

	return 0;
}

void *work(void *arg) {
	struct arguments *args = (struct arguments*) arg;

//	while (true) {
		for (unsigned int i = 0; i < args->mineral_blocks_count; i++) {
			if (mineral_blocks[i].minerals_count > 0) {
	//			sleep(3);
				if (pthread_mutex_trylock(&mineral_blocks[i].digging) == 0) {

					printf("SCV %ld is mining from mineral block %d\n", args->SCV_id + 1, i + 1);
					mineral_blocks[i].minerals_count -= 8;
					pthread_mutex_unlock(&mineral_blocks[i].digging);

					printf("SCV %ld is transporting minerals\n", args->SCV_id + 1);
	//				sleep(2);

					while (pthread_mutex_trylock(&args->cmd_center->delivery));
					args->cmd_center->minerals_count += 8;
					pthread_mutex_unlock(&args->cmd_center->delivery);
					printf("SCV %ld delivered minerals to the Command center\n", args->SCV_id + 1);

					printf("Current minerals in center: %d\n", args->cmd_center->minerals_count);
				}
			}
		}
//	}

	free(args);

	return NULL;
}
