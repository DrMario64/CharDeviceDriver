#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define WR_VALUE _IOW('a', 'a', struct runner *)
#define RD_VALUE _IOR('a', 'b', struct runner *)

struct runner {
	int number;
	int position;
	int lane;
	int lap;
	char name[20];
	char school[10];
	struct runner *next;

	unsigned long start_time;
	unsigned long cur_lap_time;
	unsigned long total_time;
};

int runners = 0; //the total number of runners in the race

void menu(char *input);
void create_runner(struct runner **head);
void print_entries(struct runner **head);
void tests(int fd);

/* uses ioctl to transfer linked list to and from kernel space.
 *
 * by dereferencing a pointer (to the struct), we are able to actually pass
 * a struct to kernel space despite the fact that this is
 * normally not possible since it is now a pointer to a pointer to a struct.
 */
int main()
{
	int fd;
	char input;
	struct runner *entries = malloc(sizeof(struct runner));
	struct runner *ans = malloc(sizeof(struct runner));
	
	printf("Openining Driver\n\n");
	fd = open("/dev/etx_device", O_RDWR);
	if (fd < 0) {
		printf("Cannot open device file :(\n");
		return 0;
	}
	while (1) {
		input = ' ';
		menu(&input);
		switch (input) {
			case 'a':
				create_runner(&entries);
				printf("Enter the runner's name\n");

				printf("\nWriting value to driver\n");
				ioctl(fd, WR_VALUE, &entries);
				break;
			case 'd':
				printf("\nReading value from driver\n\n");
				ioctl(fd, RD_VALUE, (struct runner *) &ans);
				//printf("\nValue is %d: %s\n", ans->size, ans->buf);
				print_entries(&ans);
				break;
			case 't':
				tests(fd);
				break;
			case 'q':
				close(fd);
				printf("Closing driver\n");
				exit(1);
			default:
				printf("Invalid response, please respond with either 'a', 'd', or 'q'\n");
				break;
		}
	}
}

/* Displays a menu where the user may select the operation to perform.
 *
 * Add runner: appends a runner to the linked list and updates the driver via ioctl.
 * Display runners: reads the linked list from the driver via ioctl and prints it out.
 * Quit: exit program.
 */
void menu(char *input)
{
	char buffer[5];
	printf("Add a runner    | enter 'a'\n");
	printf("Display runners | enter 'd'\n");
	printf("Run tests       | enter 't'\n");
	printf("Quit            | enter 'q'\n");
	fgets(buffer, 5, stdin);
	sscanf(buffer, "%c", input);
}

/* adds a runner to the end of the linked list.
 *
 * accepts user input for bib number, name, and school.
 * all other variables are hard coded because they have to do with time or placement.
 */
void create_runner(struct runner **head)
{
	char int_buffer[5], name_buffer[20], school_buffer[10];
	char name[20], school[10];
	int bib = 0;
	struct runner *tmp = *head;
	struct runner *new_runner;
	while (bib <= 0 || bib > 99) {
		printf("Enter the runner's bib number (must be positive and not taken).\n");
		fgets(int_buffer, 5, stdin);
		sscanf(int_buffer, "%d", &bib);

		while (tmp != NULL) {
			if (tmp->number == bib) {
				bib = 0;
				printf("Number is already in use, choose another.\n");
				break;
			}
			tmp = tmp->next;
		}
	}

	printf("Enter the runner's name\n");
	fgets(name_buffer, 20, stdin);
	sscanf(name_buffer, "%[^,]", name);

	printf("Enter the runner's school\n");
	fgets(school_buffer, 10, stdin);
	sscanf(school_buffer, "%[^,]", school);

	new_runner = (struct runner *)malloc(sizeof(struct runner));
	new_runner->number = bib;
	new_runner->position = runners + 1;
	new_runner->lane = runners + 1;
	new_runner->lap = 1;
	new_runner->start_time = 0;
	new_runner->cur_lap_time = 0;
	new_runner->total_time = 0;
	strcpy(new_runner->name, name);
	strcpy(new_runner->school, school);

	tmp = *head;
	if (tmp == NULL) { *head = new_runner; }
	else {
		while (tmp->next != NULL) { tmp = tmp->next; }
		tmp->next = new_runner;
	}
		
	runners++; //increment total number of runners
}

/* prints out all runners and their attributes.*/
void print_entries(struct runner **head)
{
	if (head == NULL) return;
	struct runner *tmp = *head;
	tmp = tmp->next;
	while (tmp != NULL) {
		printf("NAME: %s\nPOSITION: %d\nLANE: %d\nNUMBER: %d\nSCHOOL: %s\nCURRENT LAP TIME: %lu\nLAP: %d\nTOTAL TIME: %lu\n",
				tmp->name, tmp->position, tmp->lane, tmp->number, tmp->school, tmp->cur_lap_time,
				tmp->lap, tmp->total_time);
		printf("--------------------\n");
		tmp = tmp->next;
	}
}

/* test method.
 *
 * unfortunately create_runner requires user input so we have
 * to hardcode values for each struct but print_entries can still
 * be tested.
 */
void tests(int fd)
{
	struct runner *tests = malloc(sizeof(struct runner));
	struct runner *ans = malloc(sizeof(struct runner));

	printf("TEST 1: print_entries() with empty list\n----------------------------------------\n");
	print_entries(&tests);

	printf("TEST 2: add runner and display them\n------------------------------------------\n");
	struct runner test2 = {
		.number = 43,
		.position = 1,
		.lane = 1,
		.lap = 1,
		.name = "Lucas A",
		.school = "GMU",
		.start_time = 0,
		.cur_lap_time = 0,
		.total_time = 0
	};
	tests->next = &test2;
	printf("Writing value to driver\n");
	ioctl(fd, WR_VALUE, &tests);

	printf("Reading value from driver\n");
	ioctl(fd, RD_VALUE, (struct runner *) &ans);
	print_entries(&ans);

	printf("TEST 3: add another runner\n------------------------------------\n");
	struct runner test3 = {
		.number = 11,
		.position = 2,
		.lane = 2,
		.lap = 1,
		.name = "George M",
		.school = "GMU",
		.start_time = 0,
		.cur_lap_time = 0,
		.total_time = 0
	};
	tests->next->next = &test3;
	printf("Writing value to driver\n");
	ioctl(fd, WR_VALUE, &tests);

	printf("Reading value from driver\n");
	ioctl(fd, RD_VALUE, (struct runner *) &ans);
	print_entries(&ans);
}
