typedef enum {
	JSON_VALUE_OBJECT,
	JSON_VALUE_ARRAY,
	JSON_VALUE_STRING,
	JSON_VALUE_NUMBER,
	JSON_VALUE_TRUE,
	JSON_VALUE_FALSE,
	JSON_VALUE_NULL,
	JSON_VALUE_TYPES_COUNT
} json_value_type;

typedef struct json_value json_value;

typedef struct {
	jstring *keys; 
	json_value *values;
} json_object;

typedef struct {
	json_value *values;
} json_array;

typedef struct json_value {
	json_value_type type;	
	/* NOTE(josh): these should all be ignored except the one for the type it is */
	json_object object;
	json_array array;
	/* NOTE(josh): technically, in .json these are unicode, but that probably will never
	 * matter for our uses withing computer enhance 
	 */
	jstring string;
	f64 number;
	/* nothing needed for true, false, or null, cuz the type tells it all */
} json_value;


/* helper functions */
b32 json_parse_whitespace(char *json_txt, u64 json_txt_size, u64 *json_txt_offset);

/* NOTE(josh): json_value_out will be a json_value of type JSON_VALUE_OBJECT or type 
 * JSON_VALUE_ARRAY 
 * returns number of json values in the .json text
 */

/* XXX: NOTE TO SELF THROW log_traces's EVERYWHERE */
/* XXX: NOTE TO SELF there's a null terminator at the end of json_txt @index json_txt_size*/
u32 json_parse(char *json_txt, u64 json_txt_size, json_value *json_values_out)
{
	_assert(jstring_temporary_memory_info.activated);
	_assert(jstring_temporary_memory_info.address);
	_assert(jstring_temporary_memory_info.offset == 0);
	_assert(jstring_temporary_memory_info.size >= ((json_txt_size+1)*2) );
	u64 json_txt_offset = 0;
	/* NOTE(josh): these refer to the highest level json "values" i.e. a [] or {} with
	 * no associated name. in our case there should only ever be one which is just 
	 */
	u32 json_value_count = 0;
	log_trace("json_parse: trimming any inital whitespace...");
	if(!json_parse_whitespace(json_txt, json_txt_size, &json_txt_offset))
	{
		log_error("json_parse: json parse failed.");
		return(0);
	}
	log_trace("json_parse: first non-whitespace token: %c", json_txt[json_txt_offset]);
	return(json_value_count);
}

b32 json_parse_whitespace(char *json_txt, u64 json_txt_size, u64 *json_txt_offset)
{
	b32 test;
	while(1)
	{
		if(*json_txt_offset >= json_txt_size)
		{
			log_error("json_parse_whitespace: cannot read from offset (%u), "
					  "json text size is %u bytes.", *json_txt_offset, json_txt_size);
			return(false);
		}

		test = 
			(json_txt[*json_txt_offset] == ' ') ||
			(json_txt[*json_txt_offset] == '\n') ||
			(json_txt[*json_txt_offset] == '\r') ||
			(json_txt[*json_txt_offset] == '\t'); 

		if(!test)
		{
			break;
		}

		*json_txt_offset++;
	}
	return(true);
}

void debug_print_json_value(json_value *value)
{
	log_error("debug_print_json_value:\n\n\tTODO");
}
