#include <mpi.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

/*
 * Error checking macro
 * Eg:
 *     int fd = open(file_name, O_RDONLY);
 *     DIE(fd == -1, "open failed");
 */
#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)

#define COMMENT_LEN 47
#define MASTER_PROC 0

#define MIN(x, y) ((x < y) ? x : y)
#define MAX(x, y) ((x < y) ? y : x)
#define CLAMP(x, maxvalue) ((x > maxvalue) ? maxvalue : ((x < 0.f) ? 0 : (unsigned char) x))

/*
 * P5 = PGM = black & white
 * P6 = PNM = color
*/

#pragma pack(push, 1)
typedef struct _pixel {
    unsigned char r;
    unsigned char g;
    unsigned char b;
}pixel_t;

typedef struct _img {
    unsigned char P;
    int width;
    int height;
    char comment[COMMENT_LEN];
    unsigned char maxval;
    unsigned char* data;
}img_t;
#pragma pack(pop)


img_t* parse_img(char* filename);
void flush_img(char* filename, img_t* image);
float* get_filter(char* filter);

unsigned char* apply_filter_chunk(
unsigned char P, unsigned char maxvalue,
int rank, int nProcesses, char* filter_name,
int line_width, int buff_len,
unsigned char* img_data,
unsigned char* upper_line,
unsigned char* bottom_line);

void destroy_img(img_t* image);