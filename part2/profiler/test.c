typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

typedef signed long long i64;
typedef signed int i32;
typedef signed short i16;
typedef signed char i8;

typedef unsigned int b32;

typedef double f64;
typedef float f32;

#define false 0 
#define true  1

#include "linux_util.c"
#include "profiler.c"

static u64 global_cunk_cannon = 0;

void foo();
void bar();

void cunk()
{
	PROFILER_START_TIMING_BLOCK; 
	u32 i = 0;
	for( ; i < 100; i++)
	{
		foo();
	}
	PROFILER_FINISH_TIMING_BLOCK;
}

void foo()
{
	PROFILER_START_TIMING_BLOCK;
	u32 i = 0;
	for( ; i < 100; i++)
	{
		bar();
	}
	PROFILER_FINISH_TIMING_BLOCK;
}

void bar()
{
	PROFILER_START_TIMING_BLOCK;
	u32 i = 0;
	for( ; i < 100; i++)
	{
		global_cunk_cannon++;
	}
	PROFILER_FINISH_TIMING_BLOCK;
}

int main()
{
	start_profile();

	cunk();

	finish_and_print_profile(log_debug);

	return(0);
}
