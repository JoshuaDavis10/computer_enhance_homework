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

#define ARRAY_COUNT(array) (sizeof(array)/sizeof(array[0]))

#define GB (1024*1024*1024)

#define ADDRESS_MASK_4KB 0b111111111111
#define ADDRESS_MASK_16KB 0b11111111111111
#define ADDRESS_MASK_32KB 0b111111111111111
#define ADDRESS_MASK_64KB 0b1111111111111111
#define ADDRESS_MASK_256KB 0b111111111111111111
#define ADDRESS_MASK_512KB 0b1111111111111111111
#define ADDRESS_MASK_1MB 0b11111111111111111111
#define ADDRESS_MASK_4MB 0b1111111111111111111111
#define ADDRESS_MASK_16MB 0b111111111111111111111111
#define ADDRESS_MASK_64MB 0b11111111111111111111111111
#define ADDRESS_MASK_256MB 0b1111111111111111111111111111
#define ADDRESS_MASK_1GB 0b111111111111111111111111111111

typedef struct {
	char *data;
	u64 size;
} buffer;

#include "linux_util.c"
#include "repetition_tester.c"

typedef void func(char *data);

extern void READ_1GB_ASM(char *data, u64 bytes_to_read, u64 address_mask);

typedef struct test_func {
	const char *name;
	func *func;
} test_func;

static void test_read_1gb_from_4kb(char *data)
{
	READ_1GB_ASM(data, GB, ADDRESS_MASK_4KB);
}

static void test_read_1gb_from_16kb(char *data)
{
	READ_1GB_ASM(data, GB, ADDRESS_MASK_16KB);
}

static void test_read_1gb_from_32kb(char *data)
{
	READ_1GB_ASM(data, GB, ADDRESS_MASK_32KB);
}

static void test_read_1gb_from_64kb(char *data)
{
	READ_1GB_ASM(data, GB, ADDRESS_MASK_64KB);
}

static void test_read_1gb_from_256kb(char *data)
{
	READ_1GB_ASM(data, GB, ADDRESS_MASK_256KB);
}

static void test_read_1gb_from_512kb(char *data)
{
	READ_1GB_ASM(data, GB, ADDRESS_MASK_512KB);
}

static void test_read_1gb_from_1mb(char *data)
{
	READ_1GB_ASM(data, GB, ADDRESS_MASK_1MB);
}

static void test_read_1gb_from_4mb(char *data)
{
	READ_1GB_ASM(data, GB, ADDRESS_MASK_4MB);
}

static void test_read_1gb_from_16mb(char *data)
{
	READ_1GB_ASM(data, GB, ADDRESS_MASK_16MB);
}

static void test_read_1gb_from_64mb(char *data)
{
	READ_1GB_ASM(data, GB, ADDRESS_MASK_64MB);
}

static void test_read_1gb_from_256mb(char *data)
{
	READ_1GB_ASM(data, GB, ADDRESS_MASK_256MB);
}

static void test_read_1gb_from_1gb(char *data)
{
	READ_1GB_ASM(data, GB, ADDRESS_MASK_1GB);
}

static test_func test_functions[] = {
	{"Read 1GB from 4KB region", test_read_1gb_from_4kb},
	{"Read 1GB from 16KB region", test_read_1gb_from_16kb},
	{"Read 1GB from 32KB region", test_read_1gb_from_32kb},
	{"Read 1GB from 64KB region", test_read_1gb_from_64kb},
	{"Read 1GB from 256KB region", test_read_1gb_from_256kb},
	{"Read 1GB from 512KB region", test_read_1gb_from_512kb},
	{"Read 1GB from 1MB region", test_read_1gb_from_1mb},
	{"Read 1GB from 4MB region", test_read_1gb_from_4mb},
	{"Read 1GB from 16MB region", test_read_1gb_from_16mb},
	{"Read 1GB from 64MB region", test_read_1gb_from_64mb},
	{"Read 1GB from 256MB region", test_read_1gb_from_256mb},
	{"Read 1GB from 1GB region", test_read_1gb_from_1gb}
};

int main(int argc, char **argv)
{
	buffer read_buffer;
	read_buffer.size = GB;
	read_buffer.data = mmap(0, read_buffer.size, PROT_READ | PROT_WRITE, 
		MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);
	_assert(read_buffer.data);

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
				test_functions[index].func(read_buffer.data);
				repetition_tester_stop_timing(&testers[index]);

				repetition_tester_count_bytes(&testers[index], GB);
			}
		}
	}

	return(0);
}
