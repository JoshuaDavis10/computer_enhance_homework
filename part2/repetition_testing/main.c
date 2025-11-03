#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

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

typedef struct {
	void *data;
	u64 size;
} buffer;

void test_read(const char *filepath, buffer dest, repetition_tester *t) 
{
	while(repetition_tester_is_testing(t))
	{
		struct stat file_stat;
		i32 fd;
		u64 file_size;
		if((fd = open(filepath, O_RDONLY)) == -1)
		{
			/* TODO: make these tester errors or smn */
			log_error("Failed to open file: %s (errno: %d). Terminating program.", 
				filepath, errno);
		}

		repetition_tester_start_timing(t);
		u64 bytes_read = read(fd, dest.data, dest.size);
		repetition_tester_stop_timing(t);

		repetition_tester_count_bytes(t, bytes_read);

		close(fd);
	}
}

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

	repetition_tester t = {0};	
	t.testing = true;	
	t.test_length_tsc = 10 * read_cpu_frequency();
	t.test_start_tsc = read_cpu_timer();

	test_read(argv[1], read_buffer, &t); 

	free(read_buffer.data);

	return(0);
}
