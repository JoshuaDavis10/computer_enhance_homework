#define PROFILER_UNIT_COUNT 4096

typedef struct {
	u64 tsc_elapsed;

	/* NOTE(josh): I guess __func__ is program data so just need a pointer to it.
	 * i.e. don't have to create any strings
	 */
	const char *name;

	/* NOTE(josh): how many times has profiler unit with this name been used 
	 * e.g. if this unit times a function, how many times has that function been timed
	 */
	u32 hits;
} profiler_unit;

typedef struct {
	profiler_unit units[4096];	

	u64 tsc_start;
	u64 tsc_end;
} profiler;

typedef struct {
	u64 tsc_start;
	u32 unit_index;
} profiler_block;

static profiler global_profiler;

#define CONCAT(a, b) a##b

#define PROFILER_START_TIMING_BLOCK \
/* NOTE(josh): __COUNTER__ is not in C standard but seems to work with gcc -std=c89 fine */ \
profiler_block CONCAT(block_, __func__); \
CONCAT(block_, __func__).tsc_start = read_cpu_timer(); \
CONCAT(block_, __func__).unit_index = __COUNTER__; \

#define PROFILER_FINISH_TIMING_BLOCK \
{ \
	/* NOTE(josh): __func__ is C99 but seems to work with gcc -std=c89 fine */ \
	u64 temp_tsc_end = read_cpu_timer(); \
	u64 temp_unit_index = CONCAT(block_, __func__).unit_index; \
	u64 temp_tsc_start  = CONCAT(block_, __func__).tsc_start; \
	global_profiler.units[temp_unit_index].hits++; \
	global_profiler.units[temp_unit_index].tsc_elapsed += (temp_tsc_end - temp_tsc_start); \
	global_profiler.units[temp_unit_index].name = __func__; \
}

static void start_profile()
{
	global_profiler.tsc_start = read_cpu_timer();
}

/* NOTE(josh): pass whatever logging function you want to use for it */
static void finish_and_print_profile(void (*logger)(const char *, ...))
{
	global_profiler.tsc_end = read_cpu_timer();
	if(!(logger))
	{
		return;
	}

	u64 total_elapsed = global_profiler.tsc_end - global_profiler.tsc_start;

	u64 cpu_frequency = read_cpu_frequency();
	/* NOTE(josh): avoiding a division by 0 */
	_assert(cpu_frequency);
	logger("PROFILE: Total time: %0.4lfms (CPU freq %llu) -> %llu tsc's", 
		1000.0 * (f64)total_elapsed / (f64)cpu_frequency, cpu_frequency, total_elapsed);

	u32 unit_index = 0;
	while(unit_index < PROFILER_UNIT_COUNT)
	{
		if(global_profiler.units[unit_index].tsc_elapsed)
		{
			f64 percentage = 
				((f64)global_profiler.units[unit_index].tsc_elapsed / (f64)total_elapsed) * 100.0;
			logger("  %s[%llu]: %llu (%.2lf%%)", 
				global_profiler.units[unit_index].name,
				global_profiler.units[unit_index].hits,
				global_profiler.units[unit_index].tsc_elapsed,
				percentage);
		}
		unit_index++;
	}
}

/* NOTE(josh): put _static_assert(__COUNTER__ < PROFILER_UNIT_COUNT); at end of any program that uses profiler
 */
