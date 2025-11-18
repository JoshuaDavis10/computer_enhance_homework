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

extern void write_x1(char *data, u64 repeat_count);
extern void write_x2(char *data, u64 repeat_count);
extern void write_x3(char *data, u64 repeat_count);
extern void write_x4(char *data, u64 repeat_count);

typedef struct test_func {
	const char *name;
	asm_func *func;
} test_func;

static test_func test_functions[] = {
	{"write_x1", write_x1},
	{"write_x2", write_x2},
	{"write_x3", write_x3},
	{"write_x4", write_x4}
};

int main(int argc, char **argv)
{
	buffer write_buffer;
	/* NOTE(josh): allocating a page even though we're only gonna write 8 bytes */
	write_buffer.data = malloc(4096); 
	write_buffer.size = 4096;

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
				test_functions[index].func(write_buffer.data, repeat_count);
				repetition_tester_stop_timing(&testers[index]);

				repetition_tester_count_bytes(&testers[index], repeat_count);

				u64 *ptr_u64 = (u64*)write_buffer.data;
				if(*ptr_u64 != 0xDEADBEEF)
				{
					repetition_tester_error(&testers[index], "did not write expected value");
				}
			}
		}
	}

	return(0);
}
