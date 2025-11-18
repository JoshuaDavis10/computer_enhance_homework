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

#define ARRAY_COUNT(array) (sizeof(array)/sizeof(array[0]))

typedef struct {
	char *data;
	u64 size;
} buffer;

#include "linux_util.c"
#include "repetition_tester.c"

typedef void asm_func(char *data, u64 repeat_count);

extern void read_x1(char *data, u64 repeat_count);
extern void read_x2(char *data, u64 repeat_count);
extern void read_x3(char *data, u64 repeat_count);
extern void read_x4(char *data, u64 repeat_count);

typedef struct test_func {
	const char *name;
	asm_func *func;
} test_func;

static test_func test_functions[] = {
	{"read_x1", read_x1},
	{"read_x2", read_x2},
	{"read_x3", read_x3},
	{"read_x4", read_x4}
};

int main(int argc, char **argv)
{
	buffer read_buffer;
	/* NOTE(josh): allocating a page even though we're only gonna read 8 bytes */
	read_buffer.data = malloc(4096); 
	read_buffer.size = 4096;

	u64 repeat_count = 1024 * 1024 * 1024; /* 1GB worth of loops essentially */
	repetition_tester testers[ARRAY_COUNT(test_functions)];
	while(1)
	{
		u32 index = 0;
		for( ; index < ARRAY_COUNT(test_functions); index++)
		{
			testers[index] = repetition_tester_create(10);
			printf("\n---------- %s ----------\n", test_functions[index].name);
			while(repetition_tester_is_testing(&testers[index]))
			{
				repetition_tester_start_timing(&testers[index]);
				test_functions[index].func(read_buffer.data, repeat_count);
				repetition_tester_stop_timing(&testers[index]);

				repetition_tester_count_bytes(&testers[index], repeat_count);
			}
		}
	}

	return(0);
}
