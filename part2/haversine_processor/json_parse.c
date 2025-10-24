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
	/* TODO(josh): need to be able to have these be darrays. uh oh memory leaks? */
	jstring *keys; 
	json_value *values;
} json_object;

typedef struct {
	json_value *values;
} json_array;

typedef struct json_value {
	json_value_type type;	
	/* NOTE(josh): these should all be ignored except the one for the type it is */
	json_object *object;
	json_array *array;
	/* NOTE(josh): technically, in .json these are unicode, but that probably will never
	 * matter for our uses withing computer enhance 
	 */
	jstring string;
	f64 number;
	/* nothing needed for true, false, or null, cuz the type tells it all */
} json_value;

/* json memory for all the objects and things */
static void *json_memory = 0;
static u64 json_memory_size = 0;
static u64 json_memory_offset = 0;

b32 json_memory_initialize(u64 size)
{
	if(json_memory)
	{
		log_error("json_memory_initialize: json memory has already been initialized."
				  " re-initializing would likely result in lost data. returning 'false'");
		return(false);
	}
	json_memory = malloc(size);
	if(!json_memory)
	{
		_assert(0);
		log_error("json_memory_initialize: failed to initialize json memory. returning 'false'");
		return(false);
	}
	json_memory_size = size;
	log_trace("json_memory_initialize: initialized with size of %u bytes", json_memory_size);
	return(true);
}

void *json_memory_allocate(u64 size)
{
	_assert(json_memory);
	if((json_memory_offset + size) > json_memory_size)
	{
		while(json_memory_offset + size > json_memory_size)
		{
			json_memory = realloc(json_memory, json_memory_size * 2);
			json_memory_size = json_memory_size * 2;
			log_trace("json_memory_allocate: realloc'd to %u bytes", json_memory_size);
		}
	}
	void *return_address = ((char*)json_memory + json_memory_offset);
	json_memory_offset+=size;
	log_trace("json_memory_allocate: allocated %u bytes @%p", size, json_memory);
	return(return_address); 
}

/* NOTE(josh): only call this when you don't need the .json data anymore cuz you won't be able
 * to get it back, but you will still probably have a handle to it to some extent... idk if
 * that makes sense future self
 */
void json_memory_clear()
{
	json_memory = 0;
	json_memory_size = 0;
	json_memory_offset = 0;
	log_trace("json_memory_clear: json memory cleared.");
}


/* helper functions */
b32 json_parse_whitespace(char *json_txt, u64 json_txt_size, u64 *json_txt_offset);
json_object *json_parse_object(char *json_txt, u64 json_txt_size, u64 *json_txt_offset);
json_array *json_parse_array(char *json_txt, u64 json_txt_size, u64 *json_txt_offset);
u32 json_get_object_value_count(char *json_txt, u64 json_txt_size, u64 *json_txt_offset);
u32 json_get_array_value_count(char *json_txt, u64 json_txt_size, u64 *json_txt_offset);
json_value json_parse_value(char *json_txt, u64 json_txt_size, u64 *json_txt_offset);

/* NOTE(josh): json_value_out will be a json_value of type JSON_VALUE_OBJECT or type 
 * JSON_VALUE_ARRAY 
 * returns number of json values in the .json text
 */

/* XXX: NOTE TO SELF THROW log_traces's EVERYWHERE */
/* XXX: NOTE TO SELF there's a null terminator at the end of json_txt @index json_txt_size*/
u32 json_parse(
	char *json_txt, 
	u64 json_txt_size, 
	json_value *json_values_out, 
	u32 json_value_count_expected)
{
	_assert(jstring_temporary_memory_info.activated);
	_assert(jstring_temporary_memory_info.address);
	_assert(jstring_temporary_memory_info.offset == 0);
	_assert(jstring_temporary_memory_info.size >= ((json_txt_size+1)*2) );

	json_memory_initialize(json_txt_size);

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
	
	while(json_txt_offset < json_txt_size)
	{
		if(json_txt[json_txt_offset] == '{')
		{
			if(json_value_count + 1 > json_value_count_expected)
			{
				log_error("json_parse: caller expected %u json values, "
						  "but %u have been parsed so far",
						  json_value_count_expected, 
						  json_value_count_expected);
				log_error("json_parse: json parse failed.");
				return(0);
			}
			log_trace("json_parse: parsing an object...");
			json_object *tmp = 
				json_parse_object(json_txt, json_txt_size, &json_txt_offset);
			_assert(tmp);
			json_values_out[json_value_count].type = JSON_VALUE_OBJECT;
			json_values_out[json_value_count].object = tmp;
			json_value_count++;
		}
		else if(json_txt[json_txt_offset] == '[')
		{
			if(json_value_count + 1 > json_value_count_expected)
			{
				log_error("json_parse: caller expected %u json values, "
						  "but %u have been parsed so far",
						  json_value_count_expected, 
						  json_value_count_expected);
				log_error("json_parse: json parse failed.");
				return(0);
			}
			log_trace("json_parse: parsing an array...");
			/* TODO: parse array */
			json_array *tmp = 
				json_parse_array(json_txt, json_txt_size, &json_txt_offset);
			_assert(tmp);
			json_values_out[json_value_count].type = JSON_VALUE_ARRAY;
			json_values_out[json_value_count].array = tmp;
			json_value_count++;
		}
		else
		{
			log_error("json_parse: expected first non-whitespace token to be '{'"
				  " or '['.");
			log_error("json_parse: json parse failed.");
			return(0);
		}
	}

	return(json_value_count);
}

b32 json_parse_whitespace(char *json_txt, u64 json_txt_size, u64 *json_txt_offset)
{
	log_trace("parsing whitespace...");
	b32 test;
	while(1)
	{
		log_debug("json txt offset: %u", *json_txt_offset);
		if(*json_txt_offset >= json_txt_size)
		{
			log_error("json_parse_whitespace: cannot read from offset (%u), "
					  "json text size is %u bytes.", 
					  *json_txt_offset, json_txt_size);
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

json_object *json_parse_object(char *json_txt, u64 json_txt_size, u64 *json_txt_offset)
{
	_assert(json_txt[*json_txt_offset] == '{');
	json_object *result = (json_object *)json_memory_allocate(sizeof(json_object));
	_assert(result);

	u32 value_count = json_get_object_value_count(json_txt, json_txt_size, json_txt_offset);

	log_debug("sizeof(json_value): %u", sizeof(json_value));
	result->keys = (jstring *)json_memory_allocate(sizeof(jstring) * value_count);
	result->values = (json_value *)json_memory_allocate(sizeof(json_value) * value_count);
	_assert(result->values);

	(*json_txt_offset)++;
	u32 value_index;
	for(value_index = 0; value_index < value_count; value_index++)
	{
		log_trace("json_parse_object: parsing value %u of object...", value_index);
		if(!json_parse_whitespace(json_txt, json_txt_size, json_txt_offset))
		{
			_assert(0);
		}

		log_trace("json_parse_object: parsing key for value %u...", value_index);
		u32 key_string_length = 0;
		_assert(json_txt[*json_txt_offset] == '"');
		(*json_txt_offset)++;
		while(json_txt[*json_txt_offset + key_string_length] != '"')
		{
			key_string_length++;
		}

		const char *key_string = json_txt + (*json_txt_offset);
		result->keys[value_index] = 
			jstring_create_temporary(key_string, key_string_length);
		/* NOTE: + 1 to deal with the closing '"' */
		(*json_txt_offset) += key_string_length + 1; 
		log_debug("key for value %u -> %s", value_index, result->keys[value_index].data);

		if(!json_parse_whitespace(json_txt, json_txt_size, json_txt_offset))
		{
			_assert(0);
		}

		_assert(json_txt[*json_txt_offset] == ':');
		(*json_txt_offset)++;

		log_trace("json_parse_object: parsing actual value %u...", value_index);
		result->values[value_index] = 
			/* NOTE: the magic really happens in json_parse_value I guess */
			json_parse_value(json_txt, json_txt_size, json_txt_offset);
	}

	return(result);
}

json_array *json_parse_array(char *json_txt, u64 json_txt_size, u64 *json_txt_offset)
{
	_assert(json_txt[*json_txt_offset] == '[');
	json_array *result = (json_array *)json_memory_allocate(sizeof(json_array));
	_assert(result);

	u32 value_count = json_get_array_value_count(json_txt, json_txt_size, json_txt_offset);

	log_debug("sizeof(json_value): %u", sizeof(json_value));
	result->values = (json_value *)json_memory_allocate(sizeof(json_value) * value_count);
	_assert(result->values);

	/* TODO: now actually parse the array and stick the stuff in values. 
	 * but its very good that we prepassed bc now the array for all the 
	 * array's values has been allocated
	 */

	/* parse value for each one I guess */

	return(result);
}

json_value json_parse_value(char *json_txt, u64 json_txt_size, u64 *json_txt_offset)
{
	json_value result;
	/* NOTE: no whitespace to trim since caller would have done that already */
	log_trace("json_parse_value: first char in value: '%c'", json_txt[*json_txt_offset]);

	switch(json_txt[*json_txt_offset])
	{
		case '{':
		{
			/* object */
		} break;
		case '[':
		{
			/* array */
		} break;
		case '"':
		{
			/* string */
		} break;
		case 't':
		{
			/* 'true' (potentially) */
		} break;
		case 'f':
		{
			/* 'false' (potentially) */
		} break;
		case 'n':
		{
			/* 'null' (potentially) */
		} break;
		case '0':
		{
			/* the number '0' */
		} break;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		{
			/* number */
		} break;
		default:
		{
			log_error("unexpected first char for a value: '%c'", 
				json_txt[*json_txt_offset]);
			_assert(0);
		} break;
	}

	return(result);
}

u32 json_get_array_value_count(char *json_txt, u64 json_txt_size, u64 *json_txt_offset)
{
	/* TODO: parse values until end of array */

	/* TODO: this prepass code can I think be used for both array and object parsing, so put 
	 * it in a function that returns value count
	 */
	u32 bracket_depth = 1; 
	/* '[' .. since we got a bracket in the first place from it being 
	 * an array 
	 */
	u32 brace_depth = 0; /* '{' */
	u64 prepass_json_txt_offset = *json_txt_offset;
	b32 in_value = false;
	b32 in_string = false;

	/* NOTE: just set it to 1, cuz even in the case where it's just [ ], the parsing done 
	 * after the prepass will catch that
	 */
	u32 value_count = 1;

	/* once bracket_depth is 0 (i.e. we've found the closing bracket of the array, we know we 
	 * reached the end 
	 */
	log_trace("json_parse_array: doing prepass to determine number of values in the array...");
	prepass_json_txt_offset++; /* get past the '[' we started with */
	while(bracket_depth != 0)
	{
		/* should just be # commas that aren't within values ? */
		/* true, false, null won't contain commas */
		/* so if first char, after whitespace is '[', '{' or '"', we ignore commas until 
		 * we meet another one, or like
		 * the corresponding 'closing' one */
		/* NOTE: also need to ignore \" if encountered within a string. WAIT NVM, \" 
		 * is its own char, not 2 chars lol */
		switch(json_txt[prepass_json_txt_offset])
		{
			case ' ':
			case '\n':
			case '\r':
			case '\t':
			{
				/* do nothing */
			} break;
			case ',':
			{
				/* NOTE: in the case where there are 2 back to back commas, we'll 
				 * just get a parse error later
				 * when we actually parse the array, so can ignore that case here
				 */
				if((!in_value) && (!in_string))
				{
					value_count++;
				}
			} break;
			case '{':
			{
				if(!in_string)
				{
					brace_depth++;
					if(!in_value)
					{
						in_value = true;
					}
				}
			} break;
			case '}':
			{
				if(!in_string)
				{
					brace_depth--;
					if(brace_depth == 0 && bracket_depth == 1)
					{
						in_value = false;
					}
				}
			} break;
			case '[':
			{
				if(!in_string)
				{
					bracket_depth++;
					if(!in_value)
					{
						in_value = true;
					}
				}
			} break;
			case ']':
			{
				if(!in_string)
				{
					bracket_depth--;
					if(brace_depth == 0 && bracket_depth == 1)
					{
						in_value = false;
					}
				}
			} break;
			case '"':
			{
				if(in_string)
				{
					in_string = false;
				}
				else
				{
					in_string = true;
				}
			}  break;

			default:
			{
				/* do nothing */
			} break;
		}

		log_trace("\tprepass - char: '%c' | bracket depth: %u | brace depth: %u |"
			  " in value? %u | in string? %u | value count: %u",
			  json_txt[prepass_json_txt_offset], bracket_depth, brace_depth, in_value,
			  in_string, value_count);

		prepass_json_txt_offset++;
		_assert(prepass_json_txt_offset < json_txt_size);
	}

	return(value_count);
}

u32 json_get_object_value_count(char *json_txt, u64 json_txt_size, u64 *json_txt_offset)
{
	/* TODO: parse values until end of array */

	/* TODO: this prepass code can I think be used for both array and object parsing, so put 
	 * it in a function that returns value count
	 */
	u32 brace_depth = 1; 
	/* '[' .. since we got a bracket in the first place from it being 
	 * an array 
	 */
	u32 bracket_depth = 0; /* '{' */
	u64 prepass_json_txt_offset = *json_txt_offset;
	b32 in_value = false;
	b32 in_string = false;

	/* NOTE: just set it to 1, cuz even in the case where it's just [ ], the parsing done 
	 * after the prepass will catch that
	 */
	u32 value_count = 1;

	/* once bracket_depth is 0 (i.e. we've found the closing bracket of the array, we know we 
	 * reached the end 
	 */
	log_trace("json_parse_array: doing prepass to determine number of values in the array...");
	prepass_json_txt_offset++; /* get past the '[' we started with */
	while(brace_depth != 0)
	{
		/* should just be # commas that aren't within values ? */
		/* true, false, null won't contain commas */
		/* so if first char, after whitespace is '[', '{' or '"', we ignore commas until 
		 * we meet another one, or like
		 * the corresponding 'closing' one */
		/* NOTE: also need to ignore \" if encountered within a string. WAIT NVM, \" 
		 * is its own char, not 2 chars lol */
		switch(json_txt[prepass_json_txt_offset])
		{
			case ' ':
			case '\n':
			case '\r':
			case '\t':
			{
				/* do nothing */
			} break;
			case ',':
			{
				/* NOTE: in the case where there are 2 back to back commas, we'll 
				 * just get a parse error later
				 * when we actually parse the array, so can ignore that case here
				 */
				if((!in_value) && (!in_string))
				{
					value_count++;
				}
			} break;
			case '{':
			{
				if(!in_string)
				{
					brace_depth++;
					if(!in_value)
					{
						in_value = true;
					}
				}
			} break;
			case '}':
			{
				if(!in_string)
				{
					brace_depth--;
					if(brace_depth == 1 && bracket_depth == 0)
					{
						in_value = false;
					}
				}
			} break;
			case '[':
			{
				if(!in_string)
				{
					bracket_depth++;
					if(!in_value)
					{
						in_value = true;
					}
				}
			} break;
			case ']':
			{
				if(!in_string)
				{
					bracket_depth--;
					if(brace_depth == 1 && bracket_depth == 0)
					{
						in_value = false;
					}
				}
			} break;
			case '"':
			{
				if(in_string)
				{
					in_string = false;
				}
				else
				{
					in_string = true;
				}
			}  break;

			default:
			{
				/* do nothing */
			} break;
		}

		log_trace("\tprepass - char: '%c' | bracket depth: %u | brace depth: %u |"
			  " in value? %u | in string? %u | value count: %u",
			  json_txt[prepass_json_txt_offset], bracket_depth, brace_depth, in_value,
			  in_string, value_count);

		prepass_json_txt_offset++;
		_assert(prepass_json_txt_offset < json_txt_size);
	}

	return(value_count);
}

void debug_print_json_value(json_value *value)
{
	log_error("debug_print_json_value:\n\n\tTODO");
}
