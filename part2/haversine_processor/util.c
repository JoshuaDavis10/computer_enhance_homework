/*logging*/
#include <stdio.h>
#include <stdarg.h>

#define MAX_LOGGER_MESSAGE_SIZE 16384

#define LOGGER_ERROR_ENABLED 1
#define LOGGER_WARN_ENABLED 1
#define LOGGER_INFO_ENABLED 1
#define LOGGER_DEBUG_ENABLED 1
#define LOGGER_TRACE_ENABLED 1

#if LOGGER_ERROR_ENABLED
void log_error(const char *message, ...) {
	char output[MAX_LOGGER_MESSAGE_SIZE];
	va_list arg_ptr;
	va_start(arg_ptr, message);
	/* NOTE: vsnprintf is not in C89 standard? */
    vsnprintf(output, MAX_LOGGER_MESSAGE_SIZE, message, arg_ptr);
	va_end(arg_ptr);
	printf("\e[1;31m[ERROR]:\e[0;31m %s\e[0;37m\n", output);
}
#else
void log_error(const char *message, ...) {
}
#endif

#if LOGGER_WARN_ENABLED
void log_warn(const char *message, ...) {
	char output[MAX_LOGGER_MESSAGE_SIZE];
	va_list arg_ptr;
	va_start(arg_ptr, message);
    vsnprintf(output, MAX_LOGGER_MESSAGE_SIZE, message, arg_ptr);
	va_end(arg_ptr);
	printf("\e[1;33m[WARN]:\e[0;33m %s\e[0;37m\n", output);
}
#else
void log_warn(const char *message, ...) {
}
#endif

#if LOGGER_INFO_ENABLED
void log_info(const char *message, ...) {
	char output[MAX_LOGGER_MESSAGE_SIZE];
	va_list arg_ptr;
	va_start(arg_ptr, message);
    vsnprintf(output, MAX_LOGGER_MESSAGE_SIZE, message, arg_ptr);
	va_end(arg_ptr);
	printf("\e[1;32m[INFO]:\e[0;32m %s\e[0;37m\n", output);
}
#else
void log_info(const char *message, ...) {
}
#endif

#if LOGGER_DEBUG_ENABLED
void log_debug(const char *message, ...) {
	char output[MAX_LOGGER_MESSAGE_SIZE];
	va_list arg_ptr;
	va_start(arg_ptr, message);
    vsnprintf(output, MAX_LOGGER_MESSAGE_SIZE, message, arg_ptr);
	va_end(arg_ptr);
	printf("\e[1;34m[DEBUG]:\e[0;34m %s\e[0;37m\n", output);
}
#else
void log_debug(const char *message, ...) {
}
#endif

#if LOGGER_TRACE_ENABLED
void log_trace(const char *message, ...) {
	char output[MAX_LOGGER_MESSAGE_SIZE];
	va_list arg_ptr;
	va_start(arg_ptr, message);
    vsnprintf(output, MAX_LOGGER_MESSAGE_SIZE, message, arg_ptr);
	va_end(arg_ptr);
	printf("\e[1;37m[TRACE]:\e[0;37m %s\n", output);
}
#else
void log_trace(const char *message, ...) {
}
#endif

/* assertions */
#define _assert(expression)\
{							\
	if(!(expression))			\
	{						\
		printf("\e[1;31m[ASSERT]:\e[0;31mEXPRESSION: %s, FILE: %s, LINE: " \
				"%d\e[0;37m\n", \
				#expression, __FILE__, __LINE__); \
		__builtin_trap(); \
	} \
}
