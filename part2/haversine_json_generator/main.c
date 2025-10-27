#include <math.h> /* NOTE(josh): haversine_formula.c needs: sin, cos, asin, sqrt */
#include <stdlib.h> /* NOTE(josh): rand/srand for generating random points */
#include <fcntl.h> /* NOTE(josh): for open(2) */
#include <unistd.h> /* NOTE(josh): for write(2), close(2) */
#include <errno.h> /* NOTE(josh): I used this for debugging write(2) failing */

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

#define JSTRING_MEMORY_SIZE 2048
#include "jstring.h" 
#include "util.c"
#include "haversine_formula.c"

static f64 haversine_distance_accumulator = 0.0;

/* NOTE(josh): will return a float between -range and range */
f64 get_random_double(f64 range)
{
	log_debug("getting random double...");
	f64 result = ( ( ((f64)rand())/RAND_MAX ) * (2*range) ) - range;
	log_debug("got random double.");
	return result;
}

int main(int argc, char **argv)
{
	/* jstring setup */
	jstring_load_logging_function(log_trace);
		/* NOTE(josh): this should be plenty for this program 
		we'll reset after each json line generated, and there's
		no way a given line will be longer than 2048 bytes right?
		*/
	void *jstring_memory = malloc(JSTRING_MEMORY_SIZE); 
	jstring_memory_activate(JSTRING_MEMORY_SIZE, jstring_memory);

	/* get seed and num point pairs */
	if(argc != 3)
	{
		log_error("usage - "
				"haversine_generator [seed] [number of pairs of points]");
		return(-1);
	}

	u32 seed = jstring_chars_to_int(argv[1]);
	u32 point_pairs_count = jstring_chars_to_int(argv[2]);

	/* rand stuff */
	srand(seed);

	/* NOTE(josh): holy cumbersome, batman. jstring really needs a format string function */
	jstring haversine_input_json_file_name = 
		jstring_create_temporary("haversine_input_", jstring_length("haversine_input_")); 
	jstring haversine_answers_file_name = 
		jstring_create_temporary("haversine_answers_", jstring_length("haversine_answers_")); 
	jstring_concatenate_raw(&haversine_input_json_file_name, argv[2]);
	jstring_concatenate_raw(&haversine_answers_file_name, argv[2]);
	jstring_concatenate_raw(&haversine_input_json_file_name, ".json");
	jstring_concatenate_raw(&haversine_answers_file_name, ".f64");

	jstring_memory_reset(JSTRING_MEMORY_SIZE, jstring_memory);

	/* NOTE(josh): O_TRUNC | O_CREAT -> create the file if it doesn't exist, and if it already
		exists, truncate it to length 0 (see man page for open(2))
	*/
	i32 haversine_input_json_fd = 
		open(haversine_input_json_file_name.data, O_RDWR | O_CREAT | O_TRUNC, 00700);
	i32 haversine_answers_fd = 
		open(haversine_answers_file_name.data, O_RDWR | O_CREAT | O_TRUNC, 00700);

	if(haversine_input_json_fd == -1)
	{
		_assert(0);
	}

	if(haversine_answers_fd == -1)
	{
		_assert(0);
	}
		
	u32 point_pairs_index = 0;
	jstring json_start_string = 
		jstring_create_temporary("{\"pairs\":[\n", jstring_length("{\"pairs\":[\n"));
	write(haversine_input_json_fd, json_start_string.data, json_start_string.length);

	while(point_pairs_index < point_pairs_count)
	{
		/* get random pair of points */
		f64 x0 = get_random_double(180.0);
		f64 y0 = get_random_double(90.0);
		f64 x1 = get_random_double(180.0);
		f64 y1 = get_random_double(90.0);

		log_debug("\nx0: %.16lf\ny0: %.16lf\nx1: %.16lf\ny1: %.16lf", x0, y0, x1, y1);

		/* calculate distance and store in answers file/accumulator */
		/* NOTE(josh): 6372.8 is the earth radius value casey uses */
		f64 reference_calculation = reference_haversine(x0, y0, x1, y1, 6372.8);
		log_debug("reference haversine -> %.16lf", reference_calculation);
		log_debug("reference haversine -> %.16x", reference_calculation);
		haversine_distance_accumulator += reference_calculation;
		write(haversine_answers_fd, &reference_calculation, 8);

		/* write points to input .json */
		jstring x0_string = jstring_create_double(x0, 16);
		jstring y0_string = jstring_create_double(y0, 16);
		jstring x1_string = jstring_create_double(x1, 16);
		jstring y1_string = jstring_create_double(y1, 16);

			/* x0 */
		jstring json_point_pair_string = 
			jstring_create_temporary("\t{\"x0\":", jstring_length("\t{\"x0\":"));
		if(!jstring_concatenate_jstring(&json_point_pair_string, x0_string))
		{
			_assert(0);	
		}

			/* y0 */
		if(!jstring_concatenate_raw(&json_point_pair_string, ", \"y0\":"))
		{
			_assert(0);	
		}
		if(!jstring_concatenate_jstring(&json_point_pair_string, y0_string))
		{
			_assert(0);	
		}

			/* x1 */
		if(!jstring_concatenate_raw(&json_point_pair_string, ", \"x1\":"))
		{
			_assert(0);	
		}
		if(!jstring_concatenate_jstring(&json_point_pair_string, x1_string))
		{
			_assert(0);	
		}

			/* y1 */
		if(!jstring_concatenate_raw(&json_point_pair_string, ", \"y1\":"))
		{
			_assert(0);	
		}
		if(!jstring_concatenate_jstring(&json_point_pair_string, y1_string))
		{
			_assert(0);	
		}

		if(point_pairs_index == point_pairs_count - 1)
		{
			if(!jstring_concatenate_raw(&json_point_pair_string, "}\n"))
			{
				_assert(0);	
			}
		}
		else
		{
			if(!jstring_concatenate_raw(&json_point_pair_string, "},\n"))
			{
				_assert(0);	
			}
		}

		if( write(
			haversine_input_json_fd, 
			json_point_pair_string.data, 
			json_point_pair_string.length) == -1)
		{
			log_error("errno = %d", errno);
			_assert(0);
		}

		jstring_memory_reset(JSTRING_MEMORY_SIZE, jstring_memory);

		point_pairs_index++;
	}

	jstring json_end_string = jstring_create_temporary("]}\n", jstring_length("]}\n"));
	write(haversine_input_json_fd, json_end_string.data, json_end_string.length);
	f64 haversine_average = haversine_distance_accumulator / ((f64)point_pairs_count);
	write(haversine_answers_fd, &haversine_average, 8);
	log_debug("haversine average: %.16lf", haversine_average);
	log_debug("haversine average: %.16x", haversine_average);


	if(close(haversine_input_json_fd) == -1)
	{
		_assert(0);
	}
	if(close(haversine_answers_fd) == -1)
	{
		_assert(0);
	}

	jstring_memory_reset(JSTRING_MEMORY_SIZE, jstring_memory);
	free(jstring_memory);

	f64 test_float = 0x40ad7aeef7ea0e65;
	log_debug("0e65 f7ea 7aee 40ad as float -> %.16lf", test_float); 

	return(0);
}
