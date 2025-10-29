#include <x86intrin.h>
#include <sys/time.h>

#define MICROSECS_PER_SEC 1000000
#define MILLISECS_PER_SEC 1000
#define MILLISECONDS_FOR_CALIBRATION 100

static u64 read_os_timer()
{
	struct timeval time;
	gettimeofday(&time, 0);

	/* NOTE(josh): returns microseconds */
	return(MICROSECS_PER_SEC * time.tv_sec + time.tv_usec);
}

/* NOTE(josh): apparently this gets inlined automatically by most compilers 
 * but compiler might complain about the function not being used? 
 * TODO: update this comment if that occurs ^, then u can inline. just wanted to
 * see it happen for myself rather than just inlining like Casey did
 * NOTE(josh): C89 can't inline anyways apparently
 */
static u64 read_cpu_timer()
{
	return(__rdtsc());
}

/* TODO: put this in your linux util.c file */
static u64 read_cpu_frequency()
{
	u64 cpu_start = read_cpu_timer();
	u64 cpu_end = 0;
	u64 cpu_frequency = 0;
	u64 cpu_elapsed = 0;

	u64 os_start  = read_os_timer();
	u64 os_end = 0;
	u64 os_elapsed = 0;

	/* NOTE(josh): this will be #us in 100 ms */
	u64 os_wait_time = MICROSECS_PER_SEC * MILLISECONDS_FOR_CALIBRATION / MILLISECS_PER_SEC; 

	while(os_elapsed < os_wait_time)
	{
		os_end = read_os_timer();
		os_elapsed = os_end - os_start;
	}

	cpu_end = read_cpu_timer();
	cpu_elapsed = cpu_end - cpu_start;

	cpu_frequency = MICROSECS_PER_SEC * cpu_elapsed / os_elapsed;

	printf("os clock: %llu -> %llu = %llu elapsed\n", os_start, os_end, os_elapsed);
	printf("os seconds: %llu\n", os_elapsed / MICROSECS_PER_SEC);
	printf("cpu clock: %llu -> %llu = %llu elapsed\n", cpu_start, cpu_end, cpu_elapsed);
	printf("cpu frequency: %llu (guesstimate)\n", cpu_frequency);

	return(cpu_frequency);
}
