//--------------------------------------------
// NAME: Dimitar Nikolov
// GROUP: 32
// FAC. NUMBER: 121218081
// FILE NAME: controls.c
// FILE PURPOSE:
//	Definitions of the functions called by the controller
//---------------------------------------------

// Including standard libraries
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Including own libraries
#include "threads.h"
#include "types.h"

//--------------------------------------------
// FUNCTION: construct_marine
//	Creates a new marine and adds
//	it to the command center
// PARAMETERS:
//	center - A pointer to an existing command center
//----------------------------------------------
void construct_marine(command_center_t *center) {

	// Checking if there are enough minerals
	if (center->minerals >= 50) {
		// Simulating activity
		sleep(1);

		// Paying for the marine creation
		center->minerals -= 50;

		// Adding a marine to the command center
		center->marines += 1;

		// Printing a success message for constructing a marine
		printf("You wanna piece of me, boy?\n");
	} else {
		// Printing error message for not enough minerals for
		// the marine construction
		printf("Not enough minerals.\n");
	}
}

//--------------------------------------------
// FUNCTION: train_scv
//	Trains a new SCV and puts into work
// PARAMETERS:
//	center - A pointer to an existing command center
//	map - A pointer to an existing game map
//----------------------------------------------
int train_scv(command_center_t *center, map_t *map) {

	// Declaring an error status code variable
	int error;

	// Checking if there are enough minerals
	if (center->minerals >= 50) {
		// Simulating activity
		sleep(4);

		// Paying for the SCV training
		center->minerals -= 50;

		// Adding an SCV to the command center
		center->scv += 1;

		// Reallocating memory for the updated SCVs thread array
		center->scvs = (pthread_t*) realloc(center->scvs, sizeof(pthread_t) * center->scv);

		// Checking if reallocating memory has failed
		if (center->scvs == NULL) {
			// Printing the error with appropriate beginning
			perror("realloc");
			// If an error has occurred, send an error status code
			return 1;
		}

		// Allocating memory for the arguments, passed to the new SCV's thread function
		struct arguments *dig_args = (struct arguments*) malloc(sizeof(struct arguments));

		// Checking if allocating memory has failed
		if (dig_args == NULL) {
			// Printing the error with appropriate beginning
			perror("malloc");
			// If an error has occurred, send an error status code
			return 1;
		}

		// Passing the current SCV id and center and map pointers to the SCV thread arguments structure
		dig_args->id = center->scv - 1;
		dig_args->center = center;
		dig_args->map = map;

		// Trying to create a new thread for the new SCV and saving the status in the error variable
		error = pthread_create(&center->scvs[center->scv - 1], NULL, dig, (void*) dig_args);

		// Checking for error
		if (error != 0) {
			// Printing the error with appropriate beginning
			printf("pthread_create error code %d\n", error);
			// If an error has occurred, ending the program
			return 1;
		}

		// Printing success message if SCV was created successfully
		printf("SCV good to go, sir.\n");
	} else {
		// Printing error message for not enough minerals for
		// the SCV training
		printf("Not enough minerals.\n");
	}

	// Returning status code for success if everything went well
	return 0;
}
