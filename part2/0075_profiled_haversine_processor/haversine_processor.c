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
#include "profiler.c" /* NOTE(josh): this needs stuff from linux_util.c, and jstring */
#include "haversine_formula.c"
#include "json_parse.c"

u64 read_file(const char *filepath, char **output)
{	
	PROFILER_START_TIMING_BLOCK;
	struct stat file_stat;
	i32 fd;
	u64 file_size;
	if((fd = open(filepath, O_RDONLY)) == -1)
	{
		log_error("Failed to open file: %s (errno: %d). Terminating program.", 
			filepath, errno);
		return(0);
	}

	if(stat(filepath, &file_stat) == -1)
	{
		log_error("Failed to stat(2) file: %s (errno: %d). Terminating program.", 
			filepath, errno);
		return(0);
	}

	/* XXX: st_size is an off_t, which I think is unsigned lol, keep in mind */
	_assert(file_stat.st_size >= 0);
	file_size = file_stat.st_size;
	log_debug("Input json file size: %u bytes", file_size);

	(*output) = malloc(file_size+1); /* +1 for null terminator */
	if(read(fd, (*output), file_stat.st_size) == -1)
	{
		log_error("Failed to read(2) file: %s (errno: %d). Terminating program.", 
			filepath, errno);
		return(0);
	}
	(*output)[file_size] = '\0';

	PROFILER_FINISH_TIMING_BLOCK;
	return(file_size);
}

b32 compute_haversine_sums(json_value *json_parse_result, b32 check_answers, i32 answers_fd)
{
	PROFILER_START_TIMING_BLOCK;
	_assert(json_parse_result->type == JSON_VALUE_OBJECT);
	_assert(json_parse_result->object->values_count == 1);
	_assert(json_parse_result->object->values[0].type == JSON_VALUE_ARRAY);

	json_value *haversine_points_list = json_parse_result->object->values[0].array->values;
	u32 haversine_points_count = json_parse_result->object->values[0].array->values_count;
	u32 haversine_points_index = 0;
	f64 x0, y0, x1, y1;
	f64 haversine_accumulator = 0.0;
	u32 incorrect_haversine_distance_count = 0;
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

		if(check_answers)
		{
			f64 haversine_answer; 
			if(read(answers_fd, &haversine_answer, sizeof(f64)) == -1)
			{
				log_error("Failed to read(2) answers file. (errno: %d). "
					  "Terminating program.", errno);
				return(false);
			}

			f64 diff = haversine_answer - reference_haversine_distance;

			if( (diff > 0.000000000001) || (diff < -0.000000000001) )
			{
				incorrect_haversine_distance_count++;
			}
		}
	}
	if(incorrect_haversine_distance_count)
	{
		log_error("%u of %u haversine distances were incorrect",
			incorrect_haversine_distance_count, haversine_points_count);
	}

	f64 haversine_average = haversine_accumulator / ((f64)haversine_points_count);
	if(check_answers)
	{
		f64 haversine_average_answer;
		if(read(answers_fd, &haversine_average_answer, sizeof(f64)) == -1)
		{
			log_error("Failed to read(2) answers file. (errno: %d). "
				  "Terminating program.", errno);
			return(false);
		}
		f64 diff = haversine_average_answer - haversine_average;
		if( (diff > 0.000000000001) || (diff < -0.000000000001) )
		{
			log_error("check average against answer: %.16lf != %.16lf", 
				haversine_average, haversine_average_answer);
		}
		else
		{
			log_info("check average against answer: %.16lf == %.16lf", 
				haversine_average, haversine_average_answer);
		}
	}

	PROFILER_FINISH_TIMING_BLOCK;
	return(true);
}

int main(int argc, char **argv)
{
	start_profile();

	if(!jstring_load_logging_function(log_warn))
	{
		log_error("Failed to load jstring logging function. Terminating program.");
		return(-1);
	}

	u64 input_json_file_size;
	i32 input_answers_fd = 0;
	char *input_json_txt;
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

	input_json_file_size = read_file(argv[1], &input_json_txt);

	if(!input_json_file_size)
	{
		return(-1);
	}

	jstring_memory = malloc((input_json_file_size+1) * 2);
	if(!jstring_memory_activate( ((input_json_file_size+1)*2), jstring_memory))
	{
		log_error("Failed to activate jstring memory. Terminating program.");
		return(-1);
	}

	json_value *json_parse_result = malloc(sizeof(json_value));

	u32 json_parse_value_count = 
		json_parse(input_json_txt, input_json_file_size, json_parse_result, 1);

	/* NOTE(josh): in our case, there's just gonna be one top-level .json object */
	_assert(json_parse_value_count == 1);

	/* debug_print_json_value(json_parse_result); */

	b32 argc_3 = false;
	if(argc == 3)
	{
		argc_3 = true;
	}

	if(!compute_haversine_sums(json_parse_result, argc_3, input_answers_fd))
	{
		return(-1);
	}

	finish_and_print_profile(log_info);

	json_memory_clear();
	log_trace("freeing stuff that was malloc'd in main() --"
	   " input_json_txt, jstring_memory, json_parse_result...");
	free(input_json_txt);
	free(jstring_memory);
	free(json_parse_result);

	return(0);
}

_static_assert(__COUNTER__ < PROFILER_UNIT_COUNT, counter_went_past_profiler_unit_count);
