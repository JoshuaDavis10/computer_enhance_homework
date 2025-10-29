#define INITIAL_ENTRIES_COUNT 2

typedef struct {
	char function_name[64];
	u64 cycles;
} timing_profiler_entry;

typedef struct {
	b32 initialized;
	b32 finished;
	u64 start;
	u64 end;
	u64 cpu_frequency;
	u64 total_cycles;
	u64 last_entry_index;
	u64 entries_capacity;
	timing_profiler_entry *entries;
} timing_profiler;

timing_profiler profiler = {false, 0, 0, 0, 0, 0};

void timing_profiler_setup()
{
	profiler.last_entry_index = 0;
	profiler.entries = malloc(sizeof(timing_profiler_entry) * INITIAL_ENTRIES_COUNT);
	profiler.entries_capacity = INITIAL_ENTRIES_COUNT;
	profiler.cpu_frequency = read_cpu_frequency();
	profiler.initialized = true;
	profiler.finished = false;
	profiler.start = read_cpu_timer();
}

void timing_profiler_finish()
{
	_assert(profiler.initialized);
	profiler.end = read_cpu_timer();
	profiler.total_cycles = profiler.end - profiler.start;
	profiler.finished = true;
}

void timing_profiler_add_entry(char *function_name, u64 cycles)
{
	_assert(profiler.initialized);
	_assert(profiler.last_entry_index <= profiler.entries_capacity);
	if(profiler.last_entry_index == profiler.entries_capacity)
	{
		profiler.entries = realloc(profiler.entries, 
			sizeof(timing_profiler_entry) * profiler.entries_capacity * 2);
		profiler.entries_capacity *= 2;
		_assert(profiler.entries);
	}

	log_trace("adding profiler entry (%u) for function: %s, with %u cycles...", 
		  profiler.last_entry_index, function_name, cycles);
	jstring_copy_chars(profiler.entries[profiler.last_entry_index].function_name, 
					function_name, 64);
	profiler.entries[profiler.last_entry_index].cycles = cycles;

	profiler.last_entry_index++;
}

void timing_profiler_print_info()
{
	_assert(profiler.initialized);
	_assert(profiler.finished);
	log_info("timing profile info (cpu frequency estimate -> %llu):", profiler.cpu_frequency);
	u32 index = 0;
	f64 fraction_cycles_used;
	timing_profiler_entry entry;

	log_info("\ttotal time: %.2lfms (%llu cycles)", 
		((f64)profiler.total_cycles) / ((f64)profiler.cpu_frequency) * 1000, 
		profiler.total_cycles);

	while(index < profiler.last_entry_index)
	{
		entry = profiler.entries[index];
		fraction_cycles_used = ((f64)entry.cycles) / ((f64) profiler.total_cycles);
		log_info("\t\t%s: %llu (%.2lf%%)", entry.function_name, entry.cycles, 
		   (fraction_cycles_used * 100));
		index++;
	}
}

#define TIME_FUNCTION_START(function) \
u64 timing_profiler_cpu_start = read_cpu_timer(); \
log_trace("started profiler timer for function: %s.", #function);

#define TIME_FUNCTION_END(function) \
u64 timing_profiler_cpu_end = read_cpu_timer(); \
u64 timing_profiler_cpu_diff = timing_profiler_cpu_end - timing_profiler_cpu_start; \
log_trace("finished profiler timer for function: %s.", #function); \
timing_profiler_add_entry(#function, timing_profiler_cpu_diff); 
