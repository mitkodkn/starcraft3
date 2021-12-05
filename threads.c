// Including standard libraries
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Including own libraries
#include "controls.h"
#include "types.h"

//--------------------------------------------
// FUNCTION: control
//	Receives commands from the player and executes
//	the corresponding functions
// PARAMETERS:
//	arg - A void pointer for
//			arguments (arguments structure)
//----------------------------------------------
void *control(void* arg) {

	// Declaring an error status code variable
	int error;

	// Casting the void arguments to a pointer type of
	// struct arguments
	struct arguments *args_p = (struct arguments*) arg;

	// Creating a local structure to hold the arguments,
	// passed to the function
	struct arguments args = *args_p;

	// Declaring a buffer for the player input
	char buffer[2];

	// Looping while the game has not yet ended (marine factor)
	while (args.center->marines < 20) {

		// Putting the user input in the buffer from
		// the standard input
		fgets(buffer, 2, stdin);

		// Checking for available commands, matching
		// the player input

		// Checking if the player wants to construct a marine
		if (strcmp(buffer, "m") == 0) {

			// Waiting for concurrent command center activities to end
			while (pthread_mutex_trylock(&args.center->active));

			// Constructing a marine
			construct_marine(args.center);

			// Stopping marine construction activity in command center
			pthread_mutex_unlock(&args.center->active);
		}
		// Checking if the player wants to train an SCV
		else if (strcmp(buffer, "s") == 0) {

			// Waiting for concurrent command center activities to end
			while (pthread_mutex_trylock(&args.center->active));

			// Trying to train a new SCV and saving the status in the error variable
			error = train_scv(args.center, args.map);

			// Checking for error
			if (error != 0) {
				// If an error has occurred, sending an error status code
				return (void*)1;
			}

			// Stopping SCV training activity in command center
			error = pthread_mutex_unlock(&args.center->active);
			if (error != 0) {
				// Printing the error with appropriate beginning
				printf("pthread_mutex_unlock error code %d\n", error);
				// If an error has occurred, sending an error status code
				return (void*)1;
			}
		}
	}

	// Freeing allocated resources
	free(args_p);

	// Return status code for success if everything went well
	return NULL;
}

//--------------------------------------------
// FUNCTION: dig
//	Organizes the job of an SCV
// PARAMETERS:
//	arg - A void pointer for
//			arguments (arguments structure)
//----------------------------------------------
void *dig(void* arg) {

	// Declaring an error status code variable
	int error;

	// Casting the void arguments to a pointer type of
	// struct arguments
	struct arguments *args_p = (struct arguments*) arg;

	// Creating a local structure to hold the arguments,
	// passed to the function
	struct arguments args = *args_p;

	// Defining a variable to hold the amount which
	// will be added to the command center after
	// successful digging
	int amount_added = 0;

	// Looping while the game has not yet ended (mineral factor)
	while (args.map->minerals > 0) {

		// Looking throughout all mineral blocks on the map
		for (int i = 0; i < args.map->mineral_blocks_count && args.map->minerals > 0; i++) {

			// Checking if the current block has minerals
			if (args.map->mineral_blocks[i].minerals > 0) {

				// Simulating activity
				sleep(3);

				// Checking if the current block is being user now
				if (pthread_mutex_trylock(&args.map->mineral_blocks[i].digging) == 0) {

					// If the blog is not used by another SCV at the
					// moment, printing a success message and indicating
					// job of current SCV
					printf("SCV %d is mining from mineral block %d\n", args.id + 1, i + 1);

					// Double checking if the current block has minerals
					if (args.map->mineral_blocks[i].minerals > 0) {

						// Checking if the minerals in the current block are
						// less than the default mineral digging number (8)
						if (args.map->mineral_blocks[i].minerals - 8 <= 0) {

							// Setting an amount of minerals which will be added to
							// the command center
							amount_added = args.map->mineral_blocks[i].minerals;

							// If the minerals were less than 8, setting the
							// current block minerals to 0
							args.map->mineral_blocks[i].minerals = 0;
						} else {
							// Setting an amount of minerals which will be added to
							// the command center
							amount_added = 8;

							// Removing the digged minerals from current block
							args.map->mineral_blocks[i].minerals -= amount_added;
						}

						// Stopping SCV digging activity in current mineral block
						error = pthread_mutex_unlock(&args.map->mineral_blocks[i].digging);

						// Checking for an error
						if (error != 0) {
							// Printing the error with appropriate beginning
							printf("pthread_mutex_unlock error code %d\n", error);
							// If an error has occurred, sending an error status code
							return (void*)1;
						}

						// Waiting for concurrent activities on the game map to finish
						while (pthread_mutex_trylock(&args.map->digging));

						// Removing the amount of minerals which were digged by
						// the SCV from the map
						args.map->minerals -= amount_added;

						// Stopping SCV digging activity on the map
						error = pthread_mutex_unlock(&args.map->digging);

						// Checking for an error
						if (error != 0) {
							// Printing the error with appropriate beginning
							printf("pthread_mutex_unlock error code %d\n", error);
							// If an error has occurred, sending an error status code
							return (void*)1;
						}

						// Printing success message
						printf("SCV %d is transporting minerals\n", args.id + 1);

						// Simulating activity
						sleep(2);

						// Waiting for concurrent activities in the command
						// center to finish
						while (pthread_mutex_trylock(&args.center->active));

						// Adding minerals to the command center
						args.center->minerals += amount_added;

						// Stopping SCV transporting activity in the
						// command center
						error = pthread_mutex_unlock(&args.center->active);

						// Checking for an error
						if (error != 0) {
							// Printing the error with appropriate beginning
							printf("pthread_mutex_unlock error code %d\n", error);
							// If an error has occurred, sending an error status code
							return (void*)1;
						}

						// Printing successful message if minerals were delivered
						printf("SCV %d delivered minerals to the Command center\n", args.id + 1);
					}
				}
			}
		}
	}

	// Freeing allocated resources
	free(args_p);

	// Returning status code for success if everything went well
	return NULL;
}
