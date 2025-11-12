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

void test_read(const char *filepath, buffer dest, repetition_tester *t, test_alloc_type alloc_type) 
{
	if(alloc_type == TEST_ALLOC_MALLOC)
	{
		printf("\n-----------------------------\nread test (mallocs):\n");
	}
	if(alloc_type == TEST_ALLOC_NONE)
	{
		printf("\n-----------------------------\nread test:\n");
	}
	if(alloc_type == TEST_ALLOC_MMAP_POPULATE)
	{
		printf("\n-----------------------------\nread test (mmap populate):\n");
	}
	while(repetition_tester_is_testing(t))
	{
		i32 fd;
		if((fd = open(filepath, O_RDONLY)) == -1)
		{
			repetition_tester_error(t, "Failed to open file."); 
		}

		if(alloc_type == TEST_ALLOC_MALLOC)
		{
			dest.data = malloc(dest.size);
			if(!dest.data)
			{
				repetition_tester_error(t, "Failed to malloc."); 
			}
		}
		if(alloc_type == TEST_ALLOC_MMAP_POPULATE)
		{
			dest.data = mmap(0, dest.size, PROT_READ | PROT_WRITE, 
				MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);
			if(dest.data == MAP_FAILED)
			{
				repetition_tester_error(t, "Failed to mmap."); 
			}
		}

		repetition_tester_start_timing(t);
		u64 bytes_read = read(fd, dest.data, dest.size);
		repetition_tester_stop_timing(t);

		if(bytes_read == 0 || bytes_read == -1)
		{
			repetition_tester_error(t, "Read 0 bytes."); 
		}
		repetition_tester_count_bytes(t, bytes_read);

		if(alloc_type == TEST_ALLOC_MALLOC)
		{
			free(dest.data);
		}
		if(alloc_type == TEST_ALLOC_MMAP_POPULATE)
		{
			if(munmap(dest.data, dest.size) == -1)
			{
				repetition_tester_error(t, "Failed to munmap.");
			}
		}

		if(close(fd) == -1)
		{
			repetition_tester_error(t, "Failed to close file."); 
		}
	}
}

void test_fread(const char *filepath, buffer dest, repetition_tester *t, test_alloc_type alloc_type) 
{

	if(alloc_type == TEST_ALLOC_MALLOC)
	{
		printf("\n-----------------------------\nfread test (mallocs):\n");
	}
	if(alloc_type == TEST_ALLOC_NONE)
	{
		printf("\n-----------------------------\nfread test:\n");
	}
	if(alloc_type == TEST_ALLOC_MMAP_POPULATE)
	{
		printf("\n-----------------------------\nfread test (mmap populate):\n");
	}
	while(repetition_tester_is_testing(t))
	{
		FILE *file = fopen(filepath, "r");
		if(!file)
		{
			repetition_tester_error(t, "Failed to open file."); 
		}

		if(alloc_type == TEST_ALLOC_MALLOC)
		{
			dest.data = malloc(dest.size);
			if(!dest.data)
			{
				repetition_tester_error(t, "Failed to malloc."); 
			}
		}
		if(alloc_type == TEST_ALLOC_MMAP_POPULATE)
		{
			dest.data = mmap(0, dest.size, PROT_READ | PROT_WRITE, 
				MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);
			if(dest.data == MAP_FAILED)
			{
				repetition_tester_error(t, "Failed to mmap."); 
			}
		}

		repetition_tester_start_timing(t);
		u64 bytes_read = fread(dest.data, 1, dest.size, file);
		repetition_tester_stop_timing(t);

		if(bytes_read == 0)
		{
			repetition_tester_error(t, "Read 0 bytes."); 
		}
		repetition_tester_count_bytes(t, bytes_read);

		if(alloc_type == TEST_ALLOC_MALLOC)
		{
			free(dest.data);
		}
		if(alloc_type == TEST_ALLOC_MMAP_POPULATE)
		{
			if(munmap(dest.data, dest.size) == -1)
			{
				repetition_tester_error(t, "Failed to munmap.");
			}
		}

		if(fclose(file) == EOF)
		{
			repetition_tester_error(t, "Failed to close file."); 
		}
	}
}

__attribute((noinline)) void test_write_loop(buffer dest)
{
	u64 index = 0;
	for( ; index < dest.size; ++index)
	{
		dest.data[index] = (u8)index;
	}
}

void test_write_to_all_bytes(repetition_tester *t, buffer dest, test_alloc_type alloc_type)
{
	if(alloc_type == TEST_ALLOC_MALLOC)
	{
		printf("\n-----------------------------\nwrite test (mallocs):\n");
	}
	if(alloc_type == TEST_ALLOC_NONE)
	{
		printf("\n-----------------------------\nwrite test:\n");
	}
	if(alloc_type == TEST_ALLOC_MMAP_POPULATE)
	{
		printf("\n-----------------------------\nwrite test (mmap populate):\n");
	}
	while(repetition_tester_is_testing(t))
	{
		/* TODO: put this in function */
		if(alloc_type == TEST_ALLOC_MALLOC)
		{
			dest.data = malloc(dest.size);
			if(!dest.data)
			{
				repetition_tester_error(t, "Failed to malloc."); 
			}
		}
		if(alloc_type == TEST_ALLOC_MMAP_POPULATE)
		{
			dest.data = mmap(0, dest.size, PROT_READ | PROT_WRITE, 
				MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);
			if(dest.data == MAP_FAILED)
			{
				repetition_tester_error(t, "Failed to mmap."); 
			}
		}

		repetition_tester_start_timing(t);
		test_write_loop(dest);
		repetition_tester_stop_timing(t);

		repetition_tester_count_bytes(t, dest.size);

		u32 index = 0;
		for( ; index < dest.size; index++)
		{
			if((u8)(index % 256) != ((u8)dest.data[index]))
			{
				repetition_tester_error(t, "incorrectly wrote bytes");
				log_debug("index: %u, value: %u", (u8)(index%256), (u8)dest.data[index]);
			}
		}

		/* TODO: put this in function */
		if(alloc_type == TEST_ALLOC_MALLOC)
		{
			free(dest.data);
		}
		if(alloc_type == TEST_ALLOC_MMAP_POPULATE)
		{
			if(munmap(dest.data, dest.size) == -1)
			{
				repetition_tester_error(t, "Failed to munmap.");
			}
		}
	}
}

extern void MOVAllBytesASM(u64 size, u8 *data);
extern void NOPAllBytesASM(u64 size);
extern void CMPAllBytesASM(u64 size);
extern void DECAllBytesASM(u64 size);

void test_mov_all_bytes_asm(repetition_tester *t, buffer dest)
{
	printf("\n-----------------------------\nTEST MOVAllBytesASM:\n");
	while(repetition_tester_is_testing(t))
	{
		repetition_tester_start_timing(t);
		MOVAllBytesASM(dest.size, dest.data);
		repetition_tester_stop_timing(t);
		repetition_tester_count_bytes(t, dest.size);

		u32 index = 0;
		for( ; index < dest.size; index++)
		{
			if((u8)(index % 256) != ((u8)dest.data[index]))
			{
				repetition_tester_error(t, "incorrectly wrote bytes");
				log_debug("index: %u, value: %u", (u8)(index%256), (u8)dest.data[index]);
			}
		}
	}
}

void test_nop_all_bytes_asm(repetition_tester *t, buffer dest)
{
	printf("\n-----------------------------\nTEST NOPAllBytesASM:\n");
	while(repetition_tester_is_testing(t))
	{
		repetition_tester_start_timing(t);
		NOPAllBytesASM(dest.size);
		repetition_tester_stop_timing(t);
		repetition_tester_count_bytes(t, dest.size);
	}
}

void test_cmp_all_bytes_asm(repetition_tester *t, buffer dest)
{
	printf("\n-----------------------------\nTEST CMPAllBytesASM:\n");
	while(repetition_tester_is_testing(t))
	{
		repetition_tester_start_timing(t);
		CMPAllBytesASM(dest.size);
		repetition_tester_stop_timing(t);
		repetition_tester_count_bytes(t, dest.size);
	}
}

void test_dec_all_bytes_asm(repetition_tester *t, buffer dest)
{
	printf("\n-----------------------------\nTEST DECAllBytesASM:\n");
	while(repetition_tester_is_testing(t))
	{
		repetition_tester_start_timing(t);
		DECAllBytesASM(dest.size);
		repetition_tester_stop_timing(t);
		repetition_tester_count_bytes(t, dest.size);
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

	repetition_tester testers[6];
	while(1)
	{
		/*
		testers[0] = repetition_tester_create(10);	
		test_read(argv[1], read_buffer, &(testers[0]), TEST_ALLOC_MMAP_POPULATE); 
		testers[1] = repetition_tester_create(10);	
		test_fread(argv[1], read_buffer, &(testers[1]), TEST_ALLOC_MMAP_POPULATE); 
		testers[2] = repetition_tester_create(10);	
		test_read(argv[1], read_buffer, &(testers[2]), TEST_ALLOC_MALLOC); 
		testers[3] = repetition_tester_create(10);	
		test_fread(argv[1], read_buffer, &(testers[3]), TEST_ALLOC_MALLOC); 
		testers[4] = repetition_tester_create(10);	
		test_read(argv[1], read_buffer, &(testers[4]), TEST_ALLOC_NONE); 
		testers[5] = repetition_tester_create(10);	
		test_fread(argv[1], read_buffer, &(testers[5]), TEST_ALLOC_NONE); 
		*/

		/*
		testers[0] = repetition_tester_create(10);
		test_write_to_all_bytes(testers, read_buffer, TEST_ALLOC_MALLOC);
		testers[1] = repetition_tester_create(10);
		test_write_to_all_bytes((testers+1), read_buffer, TEST_ALLOC_MMAP_POPULATE);
		testers[2] = repetition_tester_create(10);
		test_write_to_all_bytes((testers+2), read_buffer, TEST_ALLOC_NONE);
		*/

		testers[0] = repetition_tester_create(10);
		test_write_to_all_bytes(testers, read_buffer, TEST_ALLOC_NONE);

		testers[1] = repetition_tester_create(10);
		test_mov_all_bytes_asm(testers+1, read_buffer);

		testers[2] = repetition_tester_create(10);
		test_nop_all_bytes_asm(testers+2, read_buffer);

		testers[3] = repetition_tester_create(10);
		test_cmp_all_bytes_asm(testers+3, read_buffer);

		testers[4] = repetition_tester_create(10);
		test_dec_all_bytes_asm(testers+4, read_buffer);
	}

	free(read_buffer.data);

	return(0);
}
