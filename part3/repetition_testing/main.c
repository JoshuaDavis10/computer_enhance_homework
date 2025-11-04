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

void test_read(const char *filepath, buffer dest, repetition_tester *t, b32 with_mallocs) 
{
	if(with_mallocs)
	{
		printf("\n-----------------------------\nread test (mallocs):\n");
	}
	else
	{
		printf("\n-----------------------------\nread test:\n");
	}
	while(repetition_tester_is_testing(t))
	{
		i32 fd;
		if((fd = open(filepath, O_RDONLY)) == -1)
		{
			repetition_tester_error(t, "Failed to open file."); 
		}

		if(with_mallocs)
		{
			dest.data = malloc(dest.size);
		}

		repetition_tester_start_timing(t);
		u64 bytes_read = read(fd, dest.data, dest.size);
		repetition_tester_stop_timing(t);

		if(bytes_read == 0)
		{
			repetition_tester_error(t, "Read 0 bytes."); 
		}
		repetition_tester_count_bytes(t, bytes_read);

		if(with_mallocs)
		{
			free(dest.data);
		}

		if(close(fd) == -1)
		{
			repetition_tester_error(t, "Failed to close file."); 
		}
	}
}

void test_fread(const char *filepath, buffer dest, repetition_tester *t, b32 with_mallocs) 
{
	if(with_mallocs)
	{
		printf("\n-----------------------------\nfread test (mallocs):\n");
	}
	else
	{
		printf("\n-----------------------------\nfread test:\n");
	}
	while(repetition_tester_is_testing(t))
	{
		FILE *file = fopen(filepath, "r");

		if(!file)
		{
			repetition_tester_error(t, "Failed to open file."); 
		}

		if(with_mallocs)
		{
			dest.data = malloc(dest.size);
		}

		repetition_tester_start_timing(t);
		u64 bytes_read = fread(dest.data, 1, dest.size, file);
		repetition_tester_stop_timing(t);

		if(bytes_read == 0)
		{
			repetition_tester_error(t, "Read 0 bytes."); 
		}

		repetition_tester_count_bytes(t, bytes_read);

		if(with_mallocs)
		{
			free(dest.data);
		}

		if(fclose(file) == EOF)
		{
			repetition_tester_error(t, "Failed to close file."); 
		}
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

	while(1)
	{
		repetition_tester t1 = repetition_tester_create(10);	
		test_read(argv[1], read_buffer, &t1, false); 
		repetition_tester t2 = repetition_tester_create(10);	
		test_fread(argv[1], read_buffer, &t2, false); 
		repetition_tester t3 = repetition_tester_create(10);	
		test_read(argv[1], read_buffer, &t3, true); 
		repetition_tester t4 = repetition_tester_create(10);	
		test_fread(argv[1], read_buffer, &t4, true); 
	}

	free(read_buffer.data);

	return(0);
}
