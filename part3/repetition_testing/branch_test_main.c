#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

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

enum {
	BRANCH_TEST_TYPE_ALWAYS_TAKE,
	BRANCH_TEST_TYPE_NEVER_TAKE,
	BRANCH_TEST_TYPE_TAKE_EVERY_2,
	BRANCH_TEST_TYPE_TAKE_EVERY_3,
	BRANCH_TEST_TYPE_TAKE_EVERY_4,
	BRANCH_TEST_TYPE_TAKE_CRT_RANDOM,
	BRANCH_TEST_TYPE_TAKE_OS_RANDOM
};

typedef struct {
	char *data;
	u64 size;
} buffer;

extern void ConditionalNOP(char *data, u64 size);

void branch_test(repetition_tester *t, buffer src, i32 branch_test_type)
{
	const char *pattern_name = "UNKNOWN";

	/* fill buffer accordingly */
	u32 index = 0;
	log_trace("filling source buffer...");
	switch(branch_test_type)
	{
		case BRANCH_TEST_TYPE_ALWAYS_TAKE:
		{
			for( ; index < src.size; index++)
			{
				src.data[index] = 1;
			}
			pattern_name = "Always take";
		} break;
		case BRANCH_TEST_TYPE_NEVER_TAKE:
		{
			for( ; index < src.size; index++)
			{
				src.data[index] = 0;
			}
			pattern_name = "Never take";
		} break;
		case BRANCH_TEST_TYPE_TAKE_EVERY_2:
		{
			for( ; index < src.size; index++)
			{
				src.data[index] = (index % 2 == 0);
			}
			pattern_name = "Take every 2";
		} break;
		case BRANCH_TEST_TYPE_TAKE_EVERY_3:
		{
			for( ; index < src.size; index++)
			{
				src.data[index] = (index % 3 == 0);
			}
			pattern_name = "Take every 3";
		} break;
		case BRANCH_TEST_TYPE_TAKE_EVERY_4:
		{
			for( ; index < src.size; index++)
			{
				src.data[index] = (index % 4 == 0);
			}
			pattern_name = "Take every 4";
		} break;
		case BRANCH_TEST_TYPE_TAKE_CRT_RANDOM:
		{
			for( ; index < src.size; index++)
			{
				src.data[index] = rand() % 2;
			}
			pattern_name = "CRT Random";
		} break;
		case BRANCH_TEST_TYPE_TAKE_OS_RANDOM:
		{
			i32 urandom_fd = open("/dev/urandom", O_RDONLY);
			if(urandom_fd == -1)
			{
				repetition_tester_error(t, "failed to open /dev/urandom");
				log_error("errno: %d", errno);
			}
			log_trace("opened /dev/urandom");
			u64 bytes = read(urandom_fd, (src.data), src.size);
			if(bytes != src.size)
			{
				repetition_tester_error(t, "did not read expected number of bytes");
				log_error("expected number of bytes: %u, but read: %u", src.size, bytes);
				log_error("errno: %d", errno);
			}
			log_trace("read %u bytes from /dev/urandom", bytes);
			pattern_name = "OS Random";
		} break;
		default:
		{
		} break;
	}


	log_info("BRANCH TEST: %s", pattern_name);
	while(repetition_tester_is_testing(t))
	{
		if(src.size == 0)
		{
			repetition_tester_error(t, "source buffer size is 0");
		}
		if(src.data == 0)
		{
			repetition_tester_error(t, "source buffer is a NULL pointer");
		}

		repetition_tester_start_timing(t);
		ConditionalNOP(src.data, src.size);
		repetition_tester_stop_timing(t);

		repetition_tester_count_bytes(t, src.size);
	}
	printf("\n");
}

int main(int argc, char **argv)
{
	_assert(argc == 2);

	struct stat file_stat;
	i32 fd;
	u64 file_size;

	if(stat(argv[1], &file_stat) == -1)
	{
		log_error("Failed to stat(2) file: %s (errno: %d). Terminating program.", 
			argv[1], errno);
		return(0);
	}

	_assert(file_stat.st_size >= 0);
	file_size = file_stat.st_size;

	buffer read_buffer;
	read_buffer.data = malloc(file_size);
	read_buffer.size = file_size;


	repetition_tester testers[7];
	while(1)
	{
		testers[6] = repetition_tester_create(10);
		branch_test(testers+6, read_buffer, BRANCH_TEST_TYPE_TAKE_OS_RANDOM);
		testers[0] = repetition_tester_create(10);
		branch_test(testers, read_buffer, BRANCH_TEST_TYPE_ALWAYS_TAKE);
		testers[1] = repetition_tester_create(10);
		branch_test(testers+1, read_buffer, BRANCH_TEST_TYPE_NEVER_TAKE);
		testers[2] = repetition_tester_create(10);
		branch_test(testers+2, read_buffer, BRANCH_TEST_TYPE_TAKE_EVERY_2);
		testers[3] = repetition_tester_create(10);
		branch_test(testers+3, read_buffer, BRANCH_TEST_TYPE_TAKE_EVERY_3);
		testers[4] = repetition_tester_create(10);
		branch_test(testers+4, read_buffer, BRANCH_TEST_TYPE_TAKE_EVERY_4);
		testers[5] = repetition_tester_create(10);
		branch_test(testers+5, read_buffer, BRANCH_TEST_TYPE_TAKE_CRT_RANDOM);
	}

	return(0);
}
