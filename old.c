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

typedef struct mineral_block {
	unsigned int minerals_count;
	pthread_mutex_t digging;
} mineral_block_t;

typedef struct command_center {
	unsigned int SCV_count;
	unsigned int marines_count;
	unsigned int minerals_count;
	pthread_mutex_t delivery;
} command_center_t;

typedef struct map {
	unsigned int minerals;
	unsigned int mineral_blocks_count;
	mineral_block_t *mineral_blocks;
} map_t;

struct arguments {
	intptr_t SCV_id;
	map_t *map;
	command_center_t *cmd_center;
};

void *execute_command(void*);
void *dig(void*);

void train_marine(command_center_t*);

int main(int argc, char const *argv[]) {

	int error;
	void *result;

	command_center_t cmd_center;
	map_t map;

	map.mineral_blocks_count = argc > 1 ? (intptr_t) strtol(argv[1], (char **)NULL, 10) : 2;
	map.mineral_blocks = (mineral_block_t*) malloc(sizeof(mineral_block_t) * map.mineral_blocks_count);

	if (map.mineral_blocks == NULL) {
		perror("malloc");
		return 1;
	}

	map.minerals = map.mineral_blocks_count * MAP_MINERALS_PER_BLOCK;

	cmd_center.SCV_count = INITIAL_SCV_COUNT;
	cmd_center.marines_count = INITIAL_WARRIORS_COUNT;
	cmd_center.minerals_count = INITIAL_MINERALS_COUNT;

	pthread_mutex_init(&cmd_center.delivery, NULL);

	for (unsigned int i = 0; i < map.mineral_blocks_count; i++) {
		map.mineral_blocks[i].minerals_count = MAP_MINERALS_PER_BLOCK;
		pthread_mutex_init(&map.mineral_blocks[i].digging, NULL);
	}

	pthread_t reader;
	struct arguments *reader_args = (struct arguments*) malloc(sizeof(struct arguments));

	if (reader_args == NULL) {
		perror("malloc");
		return 1;
	}

	reader_args->cmd_center = &cmd_center;

	error = pthread_create(&reader, NULL, execute_command, (void*) reader_args);

	if (error != 0) {
		perror("pthread_create");
		return 1;
	}

	pthread_t *SCV = (pthread_t*) malloc(sizeof(pthread_t) * cmd_center.SCV_count);

	if (SCV == NULL) {
		perror("malloc");
		return 1;
	}

	for (intptr_t id = 0; id < cmd_center.SCV_count; id++) {
		struct arguments *dig_args = (struct arguments*) malloc(sizeof(struct arguments));

		if (dig_args == NULL) {
			perror("malloc");
		}

		dig_args->SCV_id = id;
		dig_args->map = &map;
		dig_args->cmd_center = &cmd_center;

		error = pthread_create(&SCV[id], NULL, dig, (void*) dig_args);

		if (error != 0) {
			perror("pthread_create");
			return 1;
		}
	}

	error = pthread_join(reader, &result);

	if (error != 0) {
		perror("pthread_join");
		return 1;
	}

	for (unsigned int i = 0; i < cmd_center.SCV_count; i++) {
		void *result;
		error = pthread_join(SCV[i], &result);

		if (error != 0) {
			perror("pthread_join");
			return 1;
		}
	}

	for (unsigned int i = 0; i < map.mineral_blocks_count; i++) {
		pthread_mutex_destroy(&map.mineral_blocks[i].digging);
	}

	pthread_mutex_destroy(&cmd_center.delivery);

	free(map.mineral_blocks);
	free(SCV);

	printf("Map minerals %d, player minerals %d, SCVs %d, Marines %d\n", map.mineral_blocks_count * 500, cmd_center.minerals_count, cmd_center.SCV_count, cmd_center.marines_count);

	return 0;
}

void *execute_command(void *arg) {
	struct arguments *args = (struct arguments*) arg;

	char symbol;

	while (true) {
		symbol = getchar();
		getchar();

		switch (symbol) {
			case 'm':
				train_marine(args->cmd_center);
				break;
			case 's':

				break;
		}
	}

	free(args);

	return NULL;
}

void *dig(void *arg) {
	struct arguments *args = (struct arguments*) arg;

	while (true) {

		while (pthread_mutex_trylock(&args->cmd_center->delivery));
		printf("current map minerals: %d\n", args->map->minerals);
		if (args->map->minerals <= 0) {
//			pthread_mutex_unlock(&args->cmd_center->delivery);
			break;
		}
		pthread_mutex_unlock(&args->cmd_center->delivery);

		for (unsigned int i = 0; i < args->map->mineral_blocks_count; i++) {

			while (pthread_mutex_trylock(&args->cmd_center->delivery));
			printf("current map minerals: %d\n", args->map->minerals);
			if (args->map->minerals <= 0) {
//				pthread_mutex_unlock(&args->cmd_center->delivery);
				break;
			}
			pthread_mutex_unlock(&args->cmd_center->delivery);

//			sleep(1);
			if (args->map->mineral_blocks[i].minerals_count > 0) {
//				sleep(3);
				if (pthread_mutex_trylock(&args->map->mineral_blocks[i].digging) == 0) {
					printf("SCV %ld is mining from mineral block %d\n", args->SCV_id + 1, i + 1);

					if (args->map->mineral_blocks[i].minerals_count - 8 <= 0) {
						args->map->mineral_blocks[i].minerals_count = 0;
					} else {
						args->map->mineral_blocks[i].minerals_count -= 8;
					}

					if (args->map->minerals - 8 <= 0) {
						args->map->minerals = 0;
					} else {
						args->map->minerals -= 8;
					}

					pthread_mutex_unlock(&args->map->mineral_blocks[i].digging);

					printf("SCV %ld is transporting minerals\n", args->SCV_id + 1);
//					sleep(2);

					while (pthread_mutex_trylock(&args->cmd_center->delivery));
					args->cmd_center->minerals_count += 8;
					pthread_mutex_unlock(&args->cmd_center->delivery);
					printf("SCV %ld delivered minerals to the Command center\n", args->SCV_id + 1);
				}
			}
		}
	}

	free(args);

	return NULL;
}

void train_marine(command_center_t *cmd_center) {
	while (pthread_mutex_trylock(&cmd_center->delivery));
	if (cmd_center->minerals_count >= 50) {
//		sleep(1);
		cmd_center->minerals_count -= 50;
		cmd_center->marines_count += 1;
		printf("You wanna piece of me, boy?\n");
	} else {
		printf("Not enough minerals.\n");
	}
	pthread_mutex_unlock(&cmd_center->delivery);
}

