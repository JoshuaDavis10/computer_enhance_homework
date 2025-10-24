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
#include "util.c"
#include "haversine_formula.c"
#include "json_parse.c"

int main(int argc, char **argv)
{
	i32 input_json_fd;
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

	json_value json_parse_result;
	u32 json_parse_value_count = 
		json_parse(input_json_txt, input_json_file_size, &json_parse_result, 1);

	/* NOTE(josh): in our case, there's just gonna be one top-level .json object */
	_assert(json_parse_value_count == 1);

	debug_print_json_value(&json_parse_result);

	/* TODO: load the json value data into just like an array of floats
	 * you'll have to do this since your json parser is more general
	 */

	/* TODO: do the calculations */

	json_memory_clear();
	return(0);
}
