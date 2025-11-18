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

typedef void asm_func(char *data, u64 bytes);

extern void read_4x2(char *data, u64 bytes);
extern void read_8x2(char *data, u64 bytes);
extern void read_16x2(char *data, u64 bytes);
extern void read_32x2(char *data, u64 bytes);

typedef struct test_func {
	const char *name;
	asm_func *func;
} test_func;

static test_func test_functions[] = {
	{"read_4x2", read_4x2},
	{"read_8x2", read_8x2},
	{"read_16x2", read_16x2},
	{"read_32x2", read_32x2}
};

int main(int argc, char **argv)
{
	buffer read_buffer;
	read_buffer.data = malloc(4096); 
	read_buffer.size = 4096;

	u64 bytes = 1024 * 1024 * 1024; /* 1GB worth of loops essentially */
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
				test_functions[index].func(read_buffer.data, bytes);
				repetition_tester_stop_timing(&testers[index]);

				repetition_tester_count_bytes(&testers[index], bytes);
			}
		}
	}

	return(0);
}
