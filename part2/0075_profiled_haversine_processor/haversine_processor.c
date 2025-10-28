#include <math.h> /* NOTE(josh): haversine_formula.c needs: sin, cos, asin, sqrt */
#include <fcntl.h> /* NOTE(josh): for open(2) */
#include <errno.h> /* NOTE(josh): error codes for some syscalls like open(2), read(2), etc. */
#include <sys/stat.h> /* NOTE(josh): stat(2) for figuring out filesizes */
#include <stdlib.h> /* NOTE(josh): for malloc(3)... I know, emberassing */

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

#include "jstring.h" 
#include "linux_util.c" /* NOTE(josh): cpu frequency stuff is in here */
#include "haversine_formula.c"
#include "json_parse.c"

typedef struct {
	u64 cpu_frequency;

	u64 startup_cycles;
	u64 file_read_cycles;
	u64 setup_cycles;
	u64 parse_cycles;
	u64 sum_cycles;
	u64 check_cycles;

	u64 total_cycles;
} timing_profile_data;

static timing_profile_data profile;

int main(int argc, char **argv)
{
	profile.cpu_frequency = read_cpu_frequency();

	u64 cpu_program_start = read_cpu_timer();
	u64 cpu_start = read_cpu_timer();

	i32 input_json_fd;
	i32 input_answers_fd;
	struct stat input_json_file_stat;
	char *input_json_txt;
	u64 input_json_file_size;
	void *jstring_memory;

	if(argc != 2 && argc != 3)
	{
		log_error("USAGE\n"
			"./haversine_processor [input haversine json]\n"
			"./haversine_processor [input haversine json] [input haversine answers]");
		return(-1);
	}

	if(argc == 3)
	{
		if((input_answers_fd = open(argv[2], O_RDONLY)) == -1)
		{
			log_error("Failed to open file: %s (errno: %d). Terminating program.", 
				argv[2], errno);
			return(-1);
		}
	}

	u64 cpu_end = read_cpu_timer();
	profile.startup_cycles = cpu_end - cpu_start;
	profile.total_cycles = profile.startup_cycles;
	
	cpu_start = read_cpu_timer();
	if((input_json_fd = open(argv[1], O_RDONLY)) == -1)
	{
		log_error("Failed to open file: %s (errno: %d). Terminating program.", 
			argv[1], errno);
		return(-1);
	}
	log_info("Input json file has been opened.");

	if(stat(argv[1], &input_json_file_stat) == -1)
	{
		log_error("Failed to stat(2) file: %s (errno: %d). Terminating program.", 
			argv[1], errno);
		return(-1);
	}
	log_info("Input json file has been stat'd.");

	/* XXX: st_size is an off_t, which I think is unsigned lol, keep in mind */
	_assert(input_json_file_stat.st_size >= 0);
	input_json_file_size = input_json_file_stat.st_size;
	log_debug("Input json file size: %u bytes", input_json_file_size);

	input_json_txt = malloc(input_json_file_size+1); /* +1 for null terminator */
	if(read(input_json_fd, input_json_txt, input_json_file_stat.st_size) == -1)
	{
		log_error("Failed to read(2) file: %s (errno: %d). Terminating program.", 
			argv[1], errno);
		return(-1);
	}
	input_json_txt[input_json_file_size] = '\0';
	log_info("Input json file contents have been read into a buffer.");

	cpu_end = read_cpu_timer();
	profile.file_read_cycles = cpu_end - cpu_start;
	profile.total_cycles += profile.file_read_cycles;

	cpu_start = read_cpu_timer();

	jstring_memory = malloc((input_json_file_size+1) * 2);
	if(!jstring_load_logging_function(log_trace))
	{
		log_error("Failed to load jstring logging function. Terminating program.");
		return(-1);
	}
	if(!jstring_memory_activate( ((input_json_file_size+1)*2), jstring_memory))
	{
		log_error("Failed to activate jstring memory. Terminating program.");
		return(-1);
	}

	json_value *json_parse_result = malloc(sizeof(json_value));

	cpu_end = read_cpu_timer();
	profile.setup_cycles = cpu_end - cpu_start;
	profile.total_cycles += profile.setup_cycles;

	cpu_start = read_cpu_timer();

	u32 json_parse_value_count = 
		json_parse(input_json_txt, input_json_file_size, json_parse_result, 1);

	cpu_end = read_cpu_timer();
	profile.parse_cycles = cpu_end - cpu_start;
	profile.total_cycles += profile.parse_cycles;

	cpu_start = read_cpu_timer();

	/* NOTE(josh): in our case, there's just gonna be one top-level .json object */
	_assert(json_parse_value_count == 1);

	/* debug_print_json_value(json_parse_result); */

	_assert(json_parse_result->type == JSON_VALUE_OBJECT);
	_assert(json_parse_result->object->values_count == 1);
	_assert(json_parse_result->object->values[0].type == JSON_VALUE_ARRAY);

	json_value *haversine_points_list = json_parse_result->object->values[0].array->values;
	u32 haversine_points_count = json_parse_result->object->values[0].array->values_count;
	u32 haversine_points_index = 0;
	f64 x0, y0, x1, y1;
	f64 haversine_accumulator = 0.0;
	for( ; haversine_points_index < haversine_points_count; haversine_points_index++)
	{
		_assert(haversine_points_list[haversine_points_index].type == JSON_VALUE_OBJECT);
		json_object *haversine_points_object = 
			haversine_points_list[haversine_points_index].object;
		
		_assert(haversine_points_object->values_count == 4);
		_assert(haversine_points_object->values[0].type == JSON_VALUE_NUMBER);
		_assert(haversine_points_object->values[1].type == JSON_VALUE_NUMBER);
		_assert(haversine_points_object->values[2].type == JSON_VALUE_NUMBER);
		_assert(haversine_points_object->values[3].type == JSON_VALUE_NUMBER);

		x0 = haversine_points_object->values[0].number;
		y0 = haversine_points_object->values[1].number;
		x1 = haversine_points_object->values[2].number;
		y1 = haversine_points_object->values[3].number;

		f64 reference_haversine_distance =
			reference_haversine(x0, y0, x1, y1, 6372.8);

		haversine_accumulator+=reference_haversine_distance;

		log_trace("reference_haversine (x0:%.16lf, y0:%.16lf, x1:%.16lf, y1:%.16lf):"
			 "\n\t%.16lf", x0, y0, x1, y1, reference_haversine_distance);

		if(argc == 3)
		{
			f64 haversine_answer; 
			if(read(input_answers_fd, &haversine_answer, sizeof(f64)) == -1)
			{
				log_error("Failed to read(2) file: %s (errno: %d). "
					  "Terminating program.", argv[2], errno);
				return(-1);
			}

			f64 diff = haversine_answer - reference_haversine_distance;
			if( (diff > 0.000000000001) || (diff < -0.000000000001) )
			{
				log_trace("check against answer:\n\t%.16lf == %.16lf", 
					reference_haversine_distance, haversine_answer);
			}
			else
			{
				log_trace("check against answer:\n\t%.16lf == %.16lf", 
					reference_haversine_distance, haversine_answer);
			}
		}
	}

	cpu_end = read_cpu_timer();
	profile.sum_cycles = cpu_end - cpu_start;
	profile.total_cycles += profile.sum_cycles;

	cpu_start = read_cpu_timer();

	if(argc == 3)
	{
		f64 haversine_average = haversine_accumulator / ((f64)haversine_points_count);
		f64 haversine_average_answer;
		if(read(input_answers_fd, &haversine_average_answer, sizeof(f64)) == -1)
		{
			log_error("Failed to read(2) file: %s (errno: %d). "
				  "Terminating program.", argv[2], errno);
			return(-1);
		}
		f64 diff = haversine_average_answer - haversine_average;
		if( (diff > 0.000000000001) || (diff < -0.000000000001) )
		{
			log_trace("\ncheck average against answer:\n\t%.16lf == %.16lf\n", 
				haversine_average, haversine_average_answer);
		}
		else
		{
			log_trace("\ncheck average against answer:\n\t%.16lf == %.16lf\n", 
				haversine_average, haversine_average_answer);
		}
	}

	cpu_end = read_cpu_timer();
	profile.check_cycles = cpu_end - cpu_start;
	profile.total_cycles += profile.check_cycles;

	u64 cpu_program_end = read_cpu_timer();

	u64 total_time_ms = ((cpu_program_end - cpu_program_start) / (f64)profile.cpu_frequency) * 1000;

	log_info("profile info:");
	log_info("\ttotal time: %llu ms | total cycles: %llu | estimate cpu frequency: %llu", 
		  total_time_ms,
		  profile.total_cycles, 
		  profile.cpu_frequency);
	log_info("\t\tstartup: %llu (%.2lf%%)", 
		  profile.startup_cycles, profile.startup_cycles / (f64)profile.total_cycles * 100); 
	log_info("\t\tread: %llu (%.2lf%%)", 
		  profile.file_read_cycles, profile.file_read_cycles / (f64)profile.total_cycles * 100); 
	log_info("\t\tmisc setup: %llu (%.2lf%%)", 
		  profile.setup_cycles, profile.setup_cycles / (f64)profile.total_cycles * 100); 
	log_info("\t\tparse: %llu (%.2lf%%)", 
		  profile.parse_cycles, profile.parse_cycles / (f64)profile.total_cycles * 100); 
	log_info("\t\tsum: %llu (%.2lf%%)", 
		  profile.sum_cycles, profile.sum_cycles / (f64)profile.total_cycles * 100); 
	log_info("\t\tcheck: %llu (%.2lf%%)", 
		  profile.check_cycles, profile.check_cycles / (f64)profile.total_cycles * 100); 

	json_memory_clear();
	log_trace("freeing stuff that was malloc'd in main() --"
	   " input_json_txt, jstring_memory, json_parse_result...");
	free(input_json_txt);
	free(jstring_memory);
	free(json_parse_result);

	log_info("\nall done.\n");

	return(0);
}
