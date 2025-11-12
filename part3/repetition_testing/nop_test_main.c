#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

typedef double f64;
typedef float f32;

typedef signed char i8;
typedef unsigned char u8;
typedef short i16;
typedef unsigned short u16;
typedef int i32;
typedef unsigned int u32;
typedef long long i64;
typedef unsigned long long u64;

typedef unsigned int b32;

#define true 1
#define false 0

#include "linux_util.c"
#include "repetition_tester.c"

typedef enum {
	TEST_ALLOC_NONE,
	TEST_ALLOC_MALLOC,
	TEST_ALLOC_MMAP_POPULATE,
	TEST_ALLOC_TYPE_COUNT
} test_alloc_type;

typedef struct {
	char *data;
	u64 size;
} buffer;

extern void NOP3x1AllBytes(u64 size);
extern void NOP1x3AllBytes(u64 size);
extern void NOP1x9AllBytes(u64 size);

int main(int argc, char **argv)
{
	_assert(argc == 2);

	struct stat file_stat;
	i32 fd;
	u64 file_size;
	if((fd = open(argv[1], O_RDONLY)) == -1)
	{
		log_error("Failed to open file: %s (errno: %d). Terminating program.", 
			argv[1], errno);
		return(0);
	}

	if(stat(argv[1], &file_stat) == -1)
	{
		log_error("Failed to stat(2) file: %s (errno: %d). Terminating program.", 
			argv[1], errno);
		return(0);
	}

	_assert(file_stat.st_size >= 0);
	file_size = file_stat.st_size;

	close(fd);

	buffer read_buffer;
	read_buffer.data = malloc(file_size);
	read_buffer.size = file_size;

	repetition_tester testers[3];
	while(1)
	{
		testers[0] = repetition_tester_create(10);
		printf("--- NOP3x1AllBytes ---\n");
		while(repetition_tester_is_testing(testers))
		{
			repetition_tester_start_timing(testers);
			NOP3x1AllBytes(read_buffer.size);
			repetition_tester_stop_timing(testers);

			repetition_tester_count_bytes(testers, read_buffer.size);
		}

		testers[1] = repetition_tester_create(10);
		printf("--- NOP1X3AllBytes ---\n");
		while(repetition_tester_is_testing(testers+1))
		{
			repetition_tester_start_timing(testers+1);
			NOP1x3AllBytes(read_buffer.size);
			repetition_tester_stop_timing(testers+1);

			repetition_tester_count_bytes(testers+1, read_buffer.size);
		}

		testers[2] = repetition_tester_create(10);
		printf("--- NOP1X9AllBytes ---\n");
		while(repetition_tester_is_testing(testers+2))
		{
			repetition_tester_start_timing(testers+2);
			NOP1x9AllBytes(read_buffer.size);
			repetition_tester_stop_timing(testers+2);

			repetition_tester_count_bytes(testers+2, read_buffer.size);
		}
	}

	free(read_buffer.data);

	return(0);
}
