typedef struct {
	b32 testing;

	u64 bytes_processed;

	u64 timer_start_tsc;
	
	u32 timer_start_count;
	u32 timer_stop_count;

	u64 test_max_tsc;
	u64 test_min_tsc;

	u32 test_count;
	u64 test_start_tsc;
	u64 test_length_tsc;
	u64 test_total_tsc;
} repetition_tester;

void repetition_tester_start_timing(repetition_tester *t)
{
	t->timer_start_tsc = read_cpu_timer();
	t->timer_start_count++;
}

void repetition_tester_stop_timing(repetition_tester *t)
{
	u64 elapsed = read_cpu_timer() - t->timer_start_tsc;
	if(t->test_min_tsc == 0)
	{
		t->test_min_tsc = elapsed;
		t->test_max_tsc = elapsed;
		printf("\rmin: %llu ", t->test_min_tsc);
		fflush(stdout);
	}
	if(elapsed < t->test_min_tsc)
	{
		t->test_min_tsc = elapsed;
		t->test_start_tsc = read_cpu_timer();
		printf("\rmin: %llu ", t->test_min_tsc);
		fflush(stdout);
	}
	else if(elapsed > t->test_max_tsc)
	{
		t->test_max_tsc = elapsed;
	}
	t->test_total_tsc += elapsed;
	t->timer_stop_count++;
}

void repetition_tester_count_bytes(repetition_tester *t, u64 bytes_read)
{
	t->bytes_processed = bytes_read;
}

b32 repetition_tester_is_testing(repetition_tester *t)
{
	u64 elapsed = read_cpu_timer() - (t->test_start_tsc);
	u64 cpu_frequency = read_cpu_frequency();
	_assert(t->timer_start_count == t->timer_stop_count);
	if(elapsed > t->test_length_tsc)
	{
		u64 bytes = t->bytes_processed;
		u64 average = t->test_total_tsc / t->test_count;
		f64 seconds = (f64)t->test_min_tsc / (f64)cpu_frequency;
		printf("\rmin: %llu, %.2lfms, %.3lfgb/s\n", t->test_min_tsc, seconds * 1000.0,
			(f64)(bytes / (1024 * 1024 * 1024)) / seconds);
		seconds = (f64)t->test_max_tsc / (f64)cpu_frequency;
		printf("\rmax: %llu, %.2lfms, %.3lfgb/s\n", t->test_max_tsc, seconds * 1000.0,
			(f64)(bytes / (1024 * 1024 * 1024)) / seconds);
		seconds = (f64)average / (f64)cpu_frequency;
		printf("\raverage: %llu, %.2lfms, %.3lfgb/s\n", average, seconds * 1000.0, 
			(f64)(bytes / (1024 * 1024 * 1024)) / seconds);
		return(false);
	}
	t->test_count++;
	return(true);
}
