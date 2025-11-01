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
	u32 values_count;
} json_object;

typedef struct {
	json_value *values;
	u32 values_count;
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
	/* NOTE(josh): ima just do floating point numbers for now but technically we should
	 * do a union type here for doubles or integers 
	 */
	f64 number;
	/* NOTE: nothing needed for true, false, or null, cuz the type tells it all in those cases */
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
		log_error("json_memory_initialize: failed to initialize json memory. "
			  "returning 'false'");
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
		/* TODO: I realized that realloc'ing screws up all the pointers,
		 * so I've setup asserts to stop that from happening. that way you'll know
		 * when you've used more memory than used at initialization... until I figure
		 * out a better memory solution, you'll just have to manually decide how much
		 * to allocate up front. It was a worthy pursuit but it's barely functional
		 */
		while(json_memory_offset + size > json_memory_size)
		{
			log_trace("json_memory_allocate: realloc'ing to %u bytes", 
				json_memory_size*2);
			void *tmp = json_memory;
			json_memory = realloc(json_memory, json_memory_size * 2);
			_assert(json_memory == tmp);
			json_memory_size = json_memory_size * 2;
		}
	}
	void *return_address = ((char*)json_memory + json_memory_offset);
	json_memory_offset+=size;
	log_trace("json_memory_allocate: allocated %u bytes @%p", size, return_address); 
	log_trace("json_memory_allocate: total allocated: %u bytes", json_memory_size);
	log_trace("json_memory_allocate: total used     : %u bytes", json_memory_offset);
	return(return_address); 
}

/* NOTE(josh): only call this when you don't need the .json data anymore cuz you won't be able
 * to get it back, but you will still probably have a handle to it to some extent... idk if
 * that makes sense future self
 */
void json_memory_clear()
{
	free(json_memory);
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

/* NOTE: TO SELF THROW log_traces's EVERYWHERE */
/* NOTE: TO SELF there's a null terminator at the end of json_txt @index json_txt_size*/
u32 json_parse(
	char *json_txt, 
	u64 json_txt_size, 
	json_value *json_values_out, 
	u32 json_value_count_expected)
{
	PROFILER_START_TIMING_BLOCK;
	_assert(jstring_temporary_memory_info.activated);
	_assert(jstring_temporary_memory_info.address);
	_assert(jstring_temporary_memory_info.offset == 0);
	_assert(jstring_temporary_memory_info.size >= ((json_txt_size+1)*2) );

	/* TODO: I realized that realloc'ing screws up all the pointers,
	 * so I've setup asserts to stop that from happening. that way you'll know
	 * when you've used more memory than used at initialization... until I figure
	 * out a better memory solution, you'll just have to manually decide how much
	 * to allocate up front. It was a worthy pursuit but it's barely functional
	 */
	json_memory_initialize(json_txt_size * 4);

	u64 json_txt_offset = 0;
	/* NOTE(josh): these refer to the highest level json "values" i.e. a [] or {} with
	 * no associated name. in our case there should only ever be one which is just 
	 */
	u32 json_value_count = 0;
	if(!json_parse_whitespace(json_txt, json_txt_size, &json_txt_offset))
	{
		log_error("json_parse: json parse failed.");
		return(0);
	}
	
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
				  " or '[', but got: %c", json_txt[json_txt_offset]);
			log_error("json_parse: json parse failed.");
			return(0);
		}

		if(!json_parse_whitespace(json_txt, json_txt_size, &json_txt_offset))
		{
			_assert(0);
		}
	}

	PROFILER_FINISH_TIMING_BLOCK;
	return(json_value_count);
}

b32 json_parse_whitespace(char *json_txt, u64 json_txt_size, u64 *json_txt_offset)
{
	b32 test;
	while(1)
	{
		if(*json_txt_offset >= json_txt_size)
		{
			return(true);
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

		(*json_txt_offset)++;
	}
	return(true);
}

json_object *json_parse_object(char *json_txt, u64 json_txt_size, u64 *json_txt_offset)
{
	_assert(json_txt[*json_txt_offset] == '{');
	json_object *result = (json_object *)json_memory_allocate(sizeof(json_object));
	_assert(result);

	u32 value_count = json_get_object_value_count(json_txt, json_txt_size, json_txt_offset);

	result->keys = (jstring *)json_memory_allocate(sizeof(jstring) * value_count);
	result->values = (json_value *)json_memory_allocate(sizeof(json_value) * value_count);
	result->values_count = value_count;
	_assert(result->values);

	(*json_txt_offset)++;
	_assert(*json_txt_offset < json_txt_size);
	u32 value_index;
	for(value_index = 0; value_index < value_count; value_index++)
	{
		if(!json_parse_whitespace(json_txt, json_txt_size, json_txt_offset))
		{
			_assert(0);
		}

		u32 key_string_length = 0;
		_assert(json_txt[*json_txt_offset] == '"');
		(*json_txt_offset)++;
		_assert(*json_txt_offset < json_txt_size);
		while(json_txt[*json_txt_offset + key_string_length] != '"')
		{
			key_string_length++;
		}

		const char *key_string = json_txt + (*json_txt_offset);
		result->keys[value_index] = 
			jstring_create_temporary(key_string, key_string_length);
		/* NOTE: + 1 to deal with the closing '"' */
		(*json_txt_offset) += key_string_length + 1; 
		_assert(*json_txt_offset < json_txt_size);

		if(!json_parse_whitespace(json_txt, json_txt_size, json_txt_offset))
		{
			_assert(0);
		}

		_assert(json_txt[*json_txt_offset] == ':');
		(*json_txt_offset)++;
		_assert(*json_txt_offset < json_txt_size);

		result->values[value_index] = 
			/* NOTE: the magic really happens in json_parse_value I guess */
			json_parse_value(json_txt, json_txt_size, json_txt_offset);

		/* NOTE: get past comma/whitespace at end of value */
		if(!json_parse_whitespace(json_txt, json_txt_size, json_txt_offset))
		{
			_assert(0);
		}

		if(value_index == value_count - 1) { }
		else 
		{
			_assert(json_txt[*json_txt_offset] == ',');
			(*json_txt_offset)++;
			_assert(*json_txt_offset < json_txt_size);
		}
	}

	if(!json_parse_whitespace(json_txt, json_txt_size, json_txt_offset))
	{
		_assert(0);
	}

	_assert(json_txt[*json_txt_offset] == '}');
	(*json_txt_offset)++;
	_assert(*json_txt_offset < json_txt_size);

	return(result);
}

json_array *json_parse_array(char *json_txt, u64 json_txt_size, u64 *json_txt_offset)
{
	_assert(json_txt[*json_txt_offset] == '[');
	json_array *result = (json_array *)json_memory_allocate(sizeof(json_array));
	_assert(result);

	u32 value_count = json_get_array_value_count(json_txt, json_txt_size, json_txt_offset);

	result->values = (json_value *)json_memory_allocate(sizeof(json_value) * value_count);
	result->values_count = value_count;
	_assert(result->values);

	(*json_txt_offset)++;
	_assert(*json_txt_offset < json_txt_size);
	u32 value_index;
	for(value_index = 0; value_index < value_count; value_index++)
	{
		if(!json_parse_whitespace(json_txt, json_txt_size, json_txt_offset))
		{
			_assert(0);
		}

		result->values[value_index] = 
			/* NOTE: the magic really happens in json_parse_value I guess */
			json_parse_value(json_txt, json_txt_size, json_txt_offset);

		/* NOTE: get past comma/whitespace at end of value */
		if(!json_parse_whitespace(json_txt, json_txt_size, json_txt_offset))
		{
			_assert(0);
		}

		if(value_index == value_count - 1) { }
		else 
		{
			_assert(json_txt[*json_txt_offset] == ',');
			(*json_txt_offset)++;
			_assert(*json_txt_offset < json_txt_size);
		}
	}

	if(!json_parse_whitespace(json_txt, json_txt_size, json_txt_offset))
	{
		_assert(0);
	}

	_assert(json_txt[*json_txt_offset] == ']');
	(*json_txt_offset)++;
	_assert(*json_txt_offset < json_txt_size);

	return(result);
}

json_value json_parse_value(char *json_txt, u64 json_txt_size, u64 *json_txt_offset)
{
	json_value result;
	/* NOTE: no whitespace to trim since caller would have done that already */

	switch(json_txt[*json_txt_offset])
	{
		case '{':
		{
			/* object */
			log_trace("json_parse_value: parsing an object...");
			result.type = JSON_VALUE_OBJECT;
			result.object = 
				json_parse_object(json_txt, json_txt_size, json_txt_offset);
		} break;
		case '[':
		{
			/* array */
			log_trace("json_parse_value: parsing an array...");
			result.type = JSON_VALUE_ARRAY;
			result.array = 
				json_parse_array(json_txt, json_txt_size, json_txt_offset);
		} break;
		case '"':
		{
			/* TODO: this case will never be tested by the haversine input json */
			/* string */
			log_trace("json_parse_value: parsing a string...");
			result.type = JSON_VALUE_STRING;
			u32 string_length = 0;
			while(json_txt[*json_txt_offset + string_length] != '"')
			{
				string_length++;
				_assert( ((*json_txt_offset) + string_length) < json_txt_size);
			}
			const char *string_start = json_txt + (*json_txt_offset);
			result.string = 
				jstring_create_temporary(string_start, string_length);
			/* NOTE: + 1 to deal with the closing '"' */
			(*json_txt_offset) += string_length + 1; 
			_assert(*json_txt_offset < json_txt_size);
		} break;
		case 't':
		{
			/* TODO: this case will never be tested by the haversine input json */
			/* NOTE: we parse until we hit whitespace or a comma */
			/* 'true' (potentially) */
			log_error("TODO: true");
			_assert(0);
		} break;
		case 'f':
		{
			/* TODO: this case will never be tested by the haversine input json */
			/* 'false' (potentially) */
			log_error("TODO: false");
			_assert(0);
		} break;
		case 'n':
		{
			/* TODO: this case will never be tested by the haversine input json */
			/* 'null' (potentially) */
			log_error("TODO: null");
			_assert(0);
		} break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '-':
		{
			/* number */
			log_trace("json_parse_value: parsing a number...");
			result.type = JSON_VALUE_NUMBER;

			u32 string_length = 0;
			while(  (json_txt[*json_txt_offset + string_length] != ',') && 
				(json_txt[*json_txt_offset + string_length] != '\n') &&
				(json_txt[*json_txt_offset + string_length] != '\t') &&
				(json_txt[*json_txt_offset + string_length] != '\r') &&
				(json_txt[*json_txt_offset + string_length] != '}')  &&
				(json_txt[*json_txt_offset + string_length] != ']')  &&
				(json_txt[*json_txt_offset + string_length] != ' ')  )
			{
				string_length++;
				_assert( ((*json_txt_offset) + string_length) < json_txt_size);
			}

			const char *string_start = json_txt + (*json_txt_offset);
			result.number = jstring_chars_to_double(string_start, string_length);
			(*json_txt_offset) += string_length; 
			_assert(*json_txt_offset < json_txt_size);
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
	prepass_json_txt_offset++; /* get past the '[' we started with */
	_assert(prepass_json_txt_offset < json_txt_size);
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

		prepass_json_txt_offset++;
		_assert(prepass_json_txt_offset < json_txt_size);
	}

	return(value_count);
}

u32 json_get_object_value_count(char *json_txt, u64 json_txt_size, u64 *json_txt_offset)
{
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

		prepass_json_txt_offset++;
		_assert(prepass_json_txt_offset < json_txt_size);
	}

	return(value_count);
}

void debug_print_json_value(json_value *value)
{
	switch(value->type)
	{
		case JSON_VALUE_OBJECT:
		{
			json_object obj = *(value->object);
			printf("{\n");
			u32 value_index = 0;
			for( ; value_index < obj.values_count; value_index++)
			{
				printf("\"%s\":", obj.keys[value_index].data);
				debug_print_json_value(&(obj.values[value_index]));
				if(value_index != obj.values_count - 1)
				{
					printf(", ");
				}
			}
			printf("}\n");
		} break;
		case JSON_VALUE_ARRAY:
		{
			json_array array = *(value->array);
			printf("[\n");
			u32 value_index = 0;
			for( ; value_index < array.values_count; value_index++)
			{
				debug_print_json_value(&(array.values[value_index]));
				if(value_index != array.values_count - 1)
				{
					printf(", ");
				}
			}
			printf("]\n");
		} break;
		case JSON_VALUE_STRING:
		{
			printf("\"%s\"", value->string.data);
		} break;
		case JSON_VALUE_NUMBER:
		{
			printf("%.16lf", value->number);
		} break;
		case JSON_VALUE_TRUE:
		{
			printf("true");
		} break;
		case JSON_VALUE_FALSE:
		{
			printf("false");
		} break;
		case JSON_VALUE_NULL:
		{
			printf("null");
		} break;
		default:
		{
			_assert(0);
		} break;
	}
}
