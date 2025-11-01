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

static u32 global_cunk_cannon = 0;
static u32 global_foo_cannon = 0;
static u32 global_bar_cannon = 0;

void cunk()
{
	PROFILER_START_TIMING_BLOCK; 
	global_cunk_cannon++;
	PROFILER_FINISH_TIMING_BLOCK;
}

void foo()
{
	PROFILER_START_TIMING_BLOCK;
	cunk();
	cunk();
	cunk();
	global_foo_cannon++;
	PROFILER_FINISH_TIMING_BLOCK;
}

void bar()
{
	PROFILER_START_TIMING_BLOCK;
	cunk();
	cunk();
	cunk();
	global_bar_cannon++;
	PROFILER_FINISH_TIMING_BLOCK;
}

int main()
{
	start_profile();
	PROFILER_START_TIMING_BLOCK;

	bar();
	foo();

	PROFILER_FINISH_TIMING_BLOCK;
	finish_and_print_profile(log_debug);

	return(0);
}
