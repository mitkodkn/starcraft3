//--------------------------------------------
// NAME: Dimitar Nikolov
// GROUP: 32
// FAC. NUMBER: 121218081
// FILE NAME: main.c
// FILE PURPOSE:
//	Combines the functionality of the game
//---------------------------------------------

// Including standard libraries
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Including own libraries
#include "threads.h"
#include "types.h"
#include "utility.h"

//--------------------------------------------
// FUNCTION: main
//	Combines the logic of all custom libraries
//	to create the game
// PARAMETERS:
//	argc - Number of program arguments
//	argv - Array of program arguments as strings
//----------------------------------------------
int main(int argc, char const *argv[]) {

	// Declaring an error status code variable
	int error;

	// Declaring the map of the game
	map_t map;

	// Saving the status code of initialize_map() to error
	error = initialize_map(&map, argc > 1 ? (int) strtol(argv[1], (char **)NULL, 10) : 2);

	// Checking for error
	if (error != 0) {
		// If an error has occurred, ending the program
		return 1;
	}

	// Declaring the command center for the player
	command_center_t center;

	// Saving the status code of initialize_center() to error
	error = initialize_center(&center);

	// Checking for error
	if (error != 0) {
		// If an error has occurred, ending the program
		return 1;
	}

	// Allocating memory for the threads of all beginning SCVs in the game
	pthread_t *scvs = (pthread_t*) malloc(sizeof(pthread_t) * center.scv);

	// Checking if allocating memory has failed
	if (scvs == NULL) {
		// Printing the error with appropriate beginning
		perror("malloc");
		// If an error has occurred, ending the program
		return 1;
	}

	// Save the threads array pointer to the player's command center
	center.scvs = scvs;

	// Declaring the controller thread
	pthread_t controller;

	// Allocating memory for the arguments, passed to the controller thread function
	struct arguments *controller_args = (struct arguments*) malloc(sizeof(struct arguments));

	// Checking if allocating memory has failed
	if (controller_args == NULL) {
		// Printing the error with appropriate beginning
		perror("malloc");
		// If an error has occurred, ending the program
		return 1;
	}

	// Passing center and map pointers to the controller arguments structure
	controller_args->center = &center;
	controller_args->map = &map;

	// Trying to create a new thread for the controller and saving the status in the error variable
	error = pthread_create(&controller, NULL, control, (void*) controller_args);

	// Checking for error
	if (error != 0) {
		// Printing the error with appropriate beginning
		printf("pthread_create error code %d\n", error);
		// If an error has occurred, ending the program
		return 1;
	}

	// Starting a loop for the creation of a thread of each SCV
	for (int scv_id = 0; scv_id < center.scv; scv_id++) {
		// Allocating memory for the arguments, passed to the dig function, executed by each SCV thread
		struct arguments *dig_args = (struct arguments*) malloc(sizeof(struct arguments));

		// Checking if allocating memory has failed
		if (dig_args == NULL) {
			// Printing the error with appropriate beginning
			perror("malloc");
			// If an error has occurred, ending the program
			return 1;
		}

		// Passing the current SCV id and center and map pointers to the SCV thread arguments structure
		dig_args->id = scv_id;
		dig_args->center = &center;
		dig_args->map = &map;

		// Trying to create a new thread for a single SCV and saving the status in the error variable
		error = pthread_create(&scvs[scv_id], NULL, dig, (void*) dig_args);

		// Checking for error
		if (error != 0) {
			// Printing the error with appropriate beginning
			printf("pthread_create error code %d\n", error);
			// If an error has occurred, ending the program
			return 1;
		}
	}

	// Trying to join the controller thread to the thread of the main function
	error = pthread_join(controller, NULL);

	// Checking for error
	if (error != 0) {
		// Printing the error with appropriate beginning
		printf("pthread_join error code %d\n", error);
		// If an error has occurred, ending the program
		return 1;
	}

	// Trying to join each SCV thread to the thread of the main function
	for (int scv_id = 0; scv_id < center.scv; scv_id++) {
		// Saving the status code in the error variable
		error = pthread_join(scvs[scv_id], NULL);

		// Checking for error
		if (error != 0) {
			// Printing the error with appropriate beginning
			printf("pthread_join error code %d\n", error);
			// If an error has occurred, ending the program
			return 1;
		}
	}

	// Print results from game after its end
	printf("Map minerals %d, player minerals %d, SCVs %d, Marines %d\n", map.start_minerals, center.minerals, center.scv, center.marines);

	// Destroying the mutex of each map block
	for (int i = 0; i < map.mineral_blocks_count; i++) {
		// Saving Saving the status code in the error variable
		error = pthread_mutex_destroy(&map.mineral_blocks[i].digging);

		// Checking for error
		if (error != 0) {
			// Printing the error with appropriate beginning
			printf("pthread_mutex_destroy error code %d\n", error);
			// If an error has occurred, ending the program
			return 1;
		}
	}

	// Destroying the mutex for the map minerals
	error = pthread_mutex_destroy(&map.digging);

	// Checking for error
	if (error != 0) {
		// Printing the error with appropriate beginning
		printf("pthread_mutex_destroy error code %d\n", error);
		// If an error has occurred, ending the program
		return 1;
	}

	// Freeing allocated resources
	free(map.mineral_blocks);
	free(scvs);

	// Returning status code for success if everything went well
	return 0;
}
