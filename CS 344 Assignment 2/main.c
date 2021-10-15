// Sarah Welsh
// CS 344 Fall 2021
// Assignment 2: Files & directories
// Description: Allows the user to choose to parse the largest or smallest csv file with a given prefix,or to designate a file to parse.  Creates a directory and opens the CSV file and parses it line-by-line using tokens to copy the information into a separate movie struct for each movie and puts the movie structs into a linked list.  Writes the output to a file in the new directory.

// If you are not compiling with the gcc option --std=gnu99, then
// uncomment the following line or you might get a compiler warning
// #define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/errno.h>
#include <time.h>

#define PREFIX "movies_"
#define EXTENSION ".csv"
//extern int errno;

// MARK: - Function Prototypes
struct movie *createMovie(char *currLine);
struct movie *processFile(char *filePath);
void printMovie(struct movie* aMovie);
void printMovieList(struct movie *list);
char *getLargest(void);
char *getSmallest(void);
char *getNamed(char filename[255]);
struct movie *copy(struct movie *movie);
struct movie *getByYear(int year, struct movie *movie);
char *makeDirectory(void);
void moviesByYear (struct movie *movie);

// MARK: Movie struct
// struct for movie information
struct movie {
		char *title;
		int year;
		char *languages[5];
		double rating_value;
		struct movie *next;
};

// MARK: - createMovie()
// Parse the current line of the comma delimited file and create a movie struct with the data in this line
struct movie *createMovie(char *currLine) {
	struct movie *currMovie = malloc(sizeof(struct movie));

	// For use with strtok_r
	char *saveptr;

	// The first token is the title
	char *token = strtok_r(currLine, ",", &saveptr);
	currMovie->title = calloc(strlen(token) + 1, sizeof(char));
	strcpy(currMovie->title, token);

	// The next token is the year (which we convert to an int)
	token = strtok_r(NULL, ",", &saveptr);
	int year = atoi(token);
	currMovie->year = year;

	// The next token is the languages
	token = strtok_r(NULL, ",", &saveptr);

	// Copy the language token into its own area of memory so we can break it down into an array of separate languages
	char* allLangsToken = calloc(strlen(token) + 1, sizeof(char));
	// Remember the original allLangsToken so we can deallocate it later
	char* allLangsAllocated = allLangsToken;
	strcpy(allLangsToken, token);
	
	// Remove the first bracket from the string
	allLangsToken = allLangsToken + 1;
	// Replace the last bracket with a semicolon
	allLangsToken[strlen(allLangsToken)-1] = ';';
	
	// Initialize the languages array in the movie to NULL pointers (otherwise we get undefined behavior)
	for (int i = 0; i < 5; i++) {
		currMovie->languages[i] = NULL;
	}
	
	// Define pointers for the tokens of individual languages
	char * langSavePtr = NULL;
	// Define langToken as the first language to avoid undefined behavior
	char *langToken = strtok_r(allLangsToken, ";", &langSavePtr);
	
	for (int i = 0; langToken != NULL; i++) {
		// First allocate the memory because this is C
		currMovie->languages[i] = calloc(strlen(langToken) + 1, sizeof(char));
		// Copy the langToken into the languages array of the current movie
		strcpy(currMovie->languages[i], langToken);
		// Set the next language for langToken
		langToken = strtok_r(NULL, ";", &langSavePtr);
	}
	
	// Free the data used by the language specific pointers
	free(langToken);
	free(allLangsAllocated);
	
	// The last token is the rating
	token = strtok_r(NULL, "\n", &saveptr);
	currMovie->rating_value = atof(token);

	// Set the next node to NULL in the newly created student entry
	currMovie->next = NULL;
	
	return currMovie;
}

// MARK: processFile()
// Return a linked list of movies by parsing data from each line of the specified file.
struct movie *processFile(char *filePath) {
	
	// Open the specified file for reading only
	FILE *movieFile = fopen(filePath, "r");
	
	char *currLine = NULL;
	size_t len = 0;
	ssize_t nread;

	// Skip the first line
	getline(&currLine, &len, movieFile);

	// The head of the linked list
	struct movie *head = NULL;
	// The tail of the linked list
	struct movie *tail = NULL;

	// Read the file line by line
	while ((nread = getline(&currLine, &len, movieFile)) != -1) {
		// Get a new movie node corresponding to the current line
		struct movie *newNode = createMovie(currLine);
		
		// Is this the first node in the linked list?
		if (head == NULL) {
			// This is the first node in the linked list
			// Set the head and the tail to this node
			head = newNode;
			tail = newNode;
		} else {
			// This is not the first node.
			// Add this node to the list and advance the tail
			tail->next = newNode;
			tail = newNode;
		}
	}
	
	free(currLine);
	fclose(movieFile);
	return head;
}

// MARK: printMovie()
// Print data for the given movie
void printMovie(struct movie* aMovie) {
	printf("%s, ", aMovie->title);
	printf("%d, ", aMovie->year);
	
	// Check both that we don't go over the number of langs in the list, and we don't fall of the end
	for (int i = 0; aMovie->languages[i] != NULL && i < 5; i++) {
		if (aMovie->languages[i] == NULL) {
			printf("%s was null\n", aMovie->title);
		} else {
			printf("%s; ", aMovie->languages[i]);
		}
	}
	printf("%.1f\n", aMovie->rating_value);
}

// MARK: printMovieList()
// Print the linked list of movies
void printMovieList(struct movie *list) {
	while (list != NULL) {
		printMovie(list);
		list = list->next;
	}
}

// MARK: getLargest()
// getLargest
// Returns name of the largest file in the current directory
char *getLargest(void) {
	DIR* currDir = opendir("./");
	struct dirent *aDir;
	struct stat dirStat;
	int i = 0;
	off_t entrySize = 0;
	char *entryName = malloc(255);
	
	// Go through all the entries
	while((aDir = readdir(currDir)) != NULL){
		if (strncmp(PREFIX, aDir->d_name, strlen(PREFIX)) == 0 && strncmp(EXTENSION, (aDir->d_name + strlen(aDir->d_name) - strlen(EXTENSION)), strlen(EXTENSION)) == 0) {
			// Get meta-data for the current entry
			stat(aDir->d_name, &dirStat);

		// find largest file
			if (i == 0 || dirStat.st_size > entrySize) {
				entrySize = dirStat.st_size;
				strcpy(entryName, aDir->d_name);
			}
			i++;
		}
	}

	// Close the directory
	closedir(currDir);
	printf("\nNow processing the chosen file at name %s.\n", entryName);
	
	return entryName;
}

// MARK: getSmallest()
// void getSmallest
char *getSmallest(void) {
	DIR* currDir = opendir("./");
	struct dirent *aDir;
	struct stat dirStat;
	int i = 0;
	off_t entrySize = 0;
	char *entryName = malloc(255);
	
	// Go through all the entries
	while((aDir = readdir(currDir)) != NULL){
		if (strncmp(PREFIX, aDir->d_name, strlen(PREFIX)) == 0 && strncmp(EXTENSION, (aDir->d_name + strlen(aDir->d_name) - strlen(EXTENSION)), strlen(EXTENSION)) == 0) {
			// Get meta-data for the current entry
			stat(aDir->d_name, &dirStat);

		// find smallest file
			if (i == 0 || dirStat.st_size < entrySize) {
				entrySize = dirStat.st_size;
				strcpy(entryName, aDir->d_name);
			}
			i++;
		}
	}

	// Close the directory
	closedir(currDir);
	printf("\nNow processing the chosen file at name %s.\n", entryName);
	
	return entryName;
}

// MARK: getNamed()
char *getNamed(char filename[255]) {
	int found = 0;
	DIR* currDir = opendir("./");
	struct dirent *aDir;
	struct stat dirStat;
	char *entryName = malloc(255);

	// Go through all the entries
	while(found == 0 && (aDir = readdir(currDir)) != NULL){
		if (strncmp(filename, aDir->d_name, strlen(filename)) == 0) {
			// Get meta-data for the current entry
			stat(aDir->d_name, &dirStat);
			strcpy(entryName, aDir->d_name);
			++found;
		}
	}
	
	if (found == 0){
		free(entryName);
		printf("The file %s was not found. Try again: ", filename);
		return NULL;
	};
	
	// Close the directory
	closedir(currDir);
	printf("\nNow processing the chosen file at name %s.\n", entryName);

	return entryName;
}


// MARK: copy()
// Returns a deep copy of a movie (minus the `next` pointer, which is set to NULL)
struct movie *copy(struct movie *movie) {
	struct movie *newMovie = malloc(sizeof(struct movie));
	
	// Allocate the memory for the title
	newMovie->title = calloc(strlen(movie->title) + 1, sizeof(char));
	// Copy over the title
	strcpy(newMovie->title, movie->title);
	// Copy over the year
	newMovie->year = movie->year;
	// Copy over the languages
	for (int i = 0; movie->languages[i] != NULL && i < 5; ++i) {
		// Separately allocate the memory for the language
		newMovie->languages[i] = calloc(strlen(movie->languages[i]) + 1, sizeof(char));
		// Copy over each individual language
		strcpy(newMovie->languages[i], movie->languages[i]);
	}
	// Copy over the rating
	newMovie->rating_value = movie->rating_value;
	// Set the next pointer to NULL (so we don't point to some other existing movie)
	newMovie->next = NULL;
	
	return newMovie;
}


// MARK: getByYear()
// Returns a linked list of movies for a given year
struct movie *getByYear(int year, struct movie *movie) {
	// The head of the results list
	struct movie *resultList = NULL;
	// The tail of the results list
	struct movie *tail = NULL;
	
	// Go through all the movies
	while (movie != NULL) {
		// If the movie was released in the given year, make a copy of the movie using copy() function and add it to the results list
		if (year == movie->year) {
			struct movie *newMovie = copy(movie);
			
			// Is this the first node in the linked list?
			if (resultList == NULL) {
				// This is the first node in the linked list
				// Set the head and the tail to this node
				resultList = newMovie;
				tail = newMovie;
			} else {
				// This is not the first node.
				// Add this node to the list and advance the tail
				tail->next = newMovie;
				tail = newMovie;
			}
		}
		
		// Go to the next movie in the list
		movie = movie->next;
	}
	
	return resultList;
}

// MARK: makeDirectory()
char *makeDirectory(void) {
	int randNum = rand() % 100000;
	// 15 for the welshsa.movies., 5 for the random number
	char* newFilePath = calloc(15 + 5, sizeof(char));
	sprintf(newFilePath, "welshsa.movies.%d", randNum);
	
	// Returns the inode for your new directory
	int newDirectory = mkdir(newFilePath, 0750);
	printf("Created directory with name welshsa.movies.%d.\n", randNum);
	
	if (newDirectory == 0) {
		return newFilePath;
	} else {
		return NULL;
	}
}

// MARK: movieByYear()
// Print the movie(s) for every year
void moviesByYear (struct movie *movie) {
	// Remember the head of the main list of movies for passing into the getByYear() function
	struct movie *mainHead = movie;
	
	// Define our maximum and minimum years as the first movie's year
	int minYear = movie->year;
	int maxYear = movie->year;
	
	while (movie != NULL) {
		// If the current movie's year is less than the existing minYear, replace minYear with the current movie's year
		minYear = (minYear < movie->year) ? minYear : movie->year;
		// If the current movie's year is greater than the existing maxYear, replace maxYear with the current movie's year
		maxYear = (maxYear > movie->year) ? maxYear : movie->year;
		
		// Move to the next movie
		movie = movie->next;
	}
	
	char *dirFilePath = makeDirectory();
	
	// For each year, get the movies from that year with getByYear() function
	for (int i = minYear; i <= maxYear; i++) {
		struct movie *currYear = getByYear(i, mainHead);
		
		// Only analyze results lists that contain at least one movie
		if (currYear != NULL) {
			int file_descriptor;
			// 4 for the year length, 4 for the other text.
			char* newFilePath = calloc((strlen(dirFilePath) + 4 + 7), sizeof(char));
			sprintf(newFilePath, "./%s/%d.txt", dirFilePath, currYear->year);
			
			// Within a given year's list of movies
			while (currYear != NULL) {
				file_descriptor = open(newFilePath, O_RDWR | O_CREAT | O_APPEND, 0640);
				if (file_descriptor == -1){
					printf("open() failed on \"%s\"\n", newFilePath);
					perror("Error");
					exit(1);
				}
				
				// + 2 for the newline
				char *message = calloc((strlen(currYear->title) + 1), sizeof(char));
				sprintf(message, "%s\n", currYear->title);
				write(file_descriptor, message, strlen(message));
				
				// Close the file descriptor
				close(file_descriptor);
				
				// Move to the next movie
				currYear = currYear->next;
			}
		}
	}
	printf("\n");
}

// MARK: main()
int main(void) {
	// Use current time as seed for random generator
	srand((int)time(0));
	
	int chosen = 0;
	int chosen2 = 0;
	
	while (chosen == 0) {
		printf("1. Select file to process\n");
		printf("2. Exit the program\n");
		printf("Enter a choice 1 or 2: ");
		
		int selection;
		scanf("%d", &selection);
		
		if (selection == 1) {
			chosen2 = 0;
			while (chosen2 == 0) {
				printf("\n");
				printf("Which file you want to process?\n");
				printf("Enter 1 to pick the largest file\n");
				printf("Enter 2 to pick the smallest file\n");
				printf("Enter 3 to specify the name of a file\n");
				printf("Enter a choice from 1 to 3: ");
				
				int selection2;
				scanf("%d", &selection2);
				
				switch (selection2) {
				case 1:
					moviesByYear(processFile(getLargest()));
					chosen2 = 1;
					break;
				
				case 2:
					moviesByYear(processFile(getSmallest()));
					chosen2 = 1;
					break;
				
				case 3:
					printf("Enter the complete file name: ");
					char selection3[255];
					char* namedFile = NULL;
					while (namedFile == NULL) {
						scanf(" %s", selection3);
						namedFile = getNamed(selection3);
					}
					moviesByYear(processFile(namedFile));
					chosen2 = 1;
					break;
				
				default:
					printf("You entered an incorrect choice. Try again.\n\n");
				}
			}
		
		} else if (selection == 2) {
			printf("Goodbye.\n");
			chosen = 1;
			exit(0);
			break;
		} else {
			printf("You entered an incorrect choice. Try again.\n\n");
		}
	}

	return 0;
}
