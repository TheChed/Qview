#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define MAXBUFF 4096
#define M_PI 3.14159265358979323846
#define NM 1852			  // meters in a nm
#define EARTH_RAD 6371008 // earth radius in meters
#define FTM 0.3048		  // feet to meters
#define DEG2RAD (M_PI / 180.0)
#define NBITER 50000
#define VSSAMPLE 10

pthread_mutex_t mutex;
const char delim[2] = ";";

int quit = 0;

int socketID, socketID2;
size_t bufmain_used = 0;
char bufmain[MAXBUFF];

int init_connect(void)
{

	struct sockaddr_in PSXMAIN;

	socketID = socket(AF_INET, SOCK_STREAM, 0);

	PSXMAIN.sin_family = AF_INET;
	PSXMAIN.sin_port = htons(10747);
	PSXMAIN.sin_addr.s_addr = inet_addr("127.0.0.1");
	/* Now connect to the server */
	if (connect(socketID, (struct sockaddr *)&PSXMAIN, sizeof(PSXMAIN)) < 0) {
		perror("ERROR connecting to main server");
		exit(1);
	}
	return 0;
}

int umain(const char *Q)
{
	size_t bufmain_remain = sizeof(bufmain) - bufmain_used;
	if (bufmain_remain == 0) {
		printf("Main socket line exceeded buffer length! Discarding input");
		bufmain_used = 0;
		return 0;
	}

	int nbread =
		recv(socketID, (char *)&bufmain[bufmain_used], bufmain_remain, 0);

	if (nbread == 0) {
		printf("Main socket connection closed.");
		quit=1;
		return 0;
	}
	if (nbread < 0 && errno == EAGAIN) {
		printf("No data received.");
		/* no data for now, call back when the socket is readable */
		return 0;
	}
	if (nbread < 0) {
		printf("Main socket Connection error");
		return 0;
	}
	bufmain_used += nbread;

	/* Scan for newlines in the line buffer; we're careful here to deal with
	 * embedded \0s an evil server may send, as well as only processing lines
	 * that are complete.
	 */
	char *line_start = bufmain;
	char *line_end;
	while ((line_end = (char *)memchr((void *)line_start, '\n',
									  bufmain_used - (line_start - bufmain)))) {
		*line_end = 0;
		if (strstr(line_start, Q)) {
			printf("%s\n", line_start);
		}
		line_start = line_end + 1;
	}
	/* Shift buffer down so the unprocessed data is at the start */
	bufmain_used -= (line_start - bufmain);
	memmove(bufmain, line_start, bufmain_used);
	return nbread;
}

void *ptUmain(void *thread_param)
{
	while (!quit) {
		umain(thread_param);
	}
	printf("Exiting ptUmain\n");

	return NULL;
}

int main(int argc, char **argv)
{
	pthread_t t1;
	(void)argc;
	init_connect();
	if (pthread_create(&t1, NULL, &ptUmain, argv[1]) != 0) {
		printf("Error creating thread Umain");
	}

	pthread_join(t1, NULL);

	return 0;
}
