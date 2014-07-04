/*
 * Copyright (C) 2008-2014 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup utils_i utils
 * @{ @ingroup utils
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <sys/types.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/time.h>
#include <string.h>

#ifdef WIN32
# include "windows.h"
#else
# define _GNU_SOURCE
# include <arpa/inet.h>
# include <sys/socket.h>
# include <netdb.h>
# include <netinet/in.h>
# include <sched.h>
#endif

/**
 * strongSwan program return codes
 */
#define SS_RC_LIBSTRONGSWAN_INTEGRITY	64
#define SS_RC_DAEMON_INTEGRITY			65
#define SS_RC_INITIALIZATION_FAILED		66

#define SS_RC_FIRST	SS_RC_LIBSTRONGSWAN_INTEGRITY
#define SS_RC_LAST	SS_RC_INITIALIZATION_FAILED

/**
 * Number of bits in a byte
 */
#define BITS_PER_BYTE 8

/**
 * Default length for various auxiliary text buffers
 */
#define BUF_LEN 512

/**
 * Build assertion macro for integer expressions, evaluates to 0
 */
#define BUILD_ASSERT(x) (sizeof(char[(x) ? 0 : -1]))

/**
 * Build time check to assert a is an array, evaluates to 0
 *
 * The address of an array element has a pointer type, which is not compatible
 * to the array type.
 */
#define BUILD_ASSERT_ARRAY(a) \
		BUILD_ASSERT(!__builtin_types_compatible_p(typeof(a), typeof(&(a)[0])))

#include "utils/types.h"
#include "enum.h"
#include "utils/strerror.h"
#include "utils/string.h"
#include "utils/memory.h"
#include "utils/byteorder.h"
#include "utils/time.h"

/**
 * Directory separator character in paths on this platform
 */
#ifdef WIN32
# define DIRECTORY_SEPARATOR "\\"
#else
# define DIRECTORY_SEPARATOR "/"
#endif

/**
 * Initialize utility functions
 */
void utils_init();

/**
 * Deinitialize utility functions
 */
void utils_deinit();

/**
 * Macro gives back larger of two values.
 */
#define max(x,y) ({ \
	typeof(x) _x = (x); \
	typeof(y) _y = (y); \
	_x > _y ? _x : _y; })

/**
 * Macro gives back smaller of two values.
 */
#define min(x,y) ({ \
	typeof(x) _x = (x); \
	typeof(y) _y = (y); \
	_x < _y ? _x : _y; })

/**
 * Call destructor of an object, if object != NULL
 */
#define DESTROY_IF(obj) if (obj) (obj)->destroy(obj)

/**
 * Call offset destructor of an object, if object != NULL
 */
#define DESTROY_OFFSET_IF(obj, offset) if (obj) obj->destroy_offset(obj, offset);

/**
 * Call function destructor of an object, if object != NULL
 */
#define DESTROY_FUNCTION_IF(obj, fn) if (obj) obj->destroy_function(obj, fn);

/**
 * Debug macro to follow control flow
 */
#define POS printf("%s, line %d\n", __FILE__, __LINE__)

/**
 * Object allocation/initialization macro, using designated initializer.
 */
#define INIT(this, ...) { (this) = malloc(sizeof(*(this))); \
						   *(this) = (typeof(*(this))){ __VA_ARGS__ }; }

/**
 * Method declaration/definition macro, providing private and public interface.
 *
 * Defines a method name with this as first parameter and a return value ret,
 * and an alias for this method with a _ prefix, having the this argument
 * safely casted to the public interface iface.
 * _name is provided a function pointer, but will get optimized out by GCC.
 */
#define METHOD(iface, name, ret, this, ...) \
	static ret name(union {iface *_public; this;} \
	__attribute__((transparent_union)), ##__VA_ARGS__); \
	static typeof(name) *_##name = (typeof(name)*)name; \
	static ret name(this, ##__VA_ARGS__)

/**
 * Same as METHOD(), but is defined for two public interfaces.
 */
#define METHOD2(iface1, iface2, name, ret, this, ...) \
	static ret name(union {iface1 *_public1; iface2 *_public2; this;} \
	__attribute__((transparent_union)), ##__VA_ARGS__); \
	static typeof(name) *_##name = (typeof(name)*)name; \
	static ret name(this, ##__VA_ARGS__)

/**
 * Callback declaration/definition macro, allowing casted first parameter.
 *
 * This is very similar to METHOD, but instead of casting the first parameter
 * to a public interface, it uses a void*. This allows type safe definition
 * of a callback function, while using the real type for the first parameter.
 */
#define CALLBACK(name, ret, param1, ...) \
	static ret _cb_##name(union {void *_generic; param1;} \
	__attribute__((transparent_union)), ##__VA_ARGS__); \
	static typeof(_cb_##name) *name = (typeof(_cb_##name)*)_cb_##name; \
	static ret _cb_##name(param1, ##__VA_ARGS__)

/**
 * This macro allows counting the number of arguments passed to a macro.
 * Combined with the VA_ARGS_DISPATCH() macro this can be used to implement
 * macro overloading based on the number of arguments.
 * 0 to 10 arguments are currently supported.
 */
#define VA_ARGS_NUM(...) _VA_ARGS_NUM(0,##__VA_ARGS__,10,9,8,7,6,5,4,3,2,1,0)
#define _VA_ARGS_NUM(_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,NUM,...) NUM

/**
 * This macro can be used to dispatch a macro call based on the number of given
 * arguments, for instance:
 *
 * @code
 * #define MY_MACRO(...) VA_ARGS_DISPATCH(MY_MACRO, __VA_ARGS__)(__VA_ARGS__)
 * #define MY_MACRO1(arg) one_arg(arg)
 * #define MY_MACRO2(arg1,arg2) two_args(arg1,arg2)
 * @endcode
 *
 * MY_MACRO() can now be called with either one or two arguments, which will
 * resolve to one_arg(arg) or two_args(arg1,arg2), respectively.
 */
#define VA_ARGS_DISPATCH(func, ...) _VA_ARGS_DISPATCH(func, VA_ARGS_NUM(__VA_ARGS__))
#define _VA_ARGS_DISPATCH(func, num) __VA_ARGS_DISPATCH(func, num)
#define __VA_ARGS_DISPATCH(func, num) func ## num

/**
 * Architecture independent bitfield definition helpers (at least with GCC).
 *
 * Defines a bitfield with a type t and a fixed size of bitfield members, e.g.:
 * BITFIELD2(u_int8_t,
 *     low: 4,
 *     high: 4,
 * ) flags;
 * The member defined first placed at bit 0.
 */
#if BYTE_ORDER == LITTLE_ENDIAN
#define BITFIELD2(t, a, b,...)			struct { t a; t b; __VA_ARGS__}
#define BITFIELD3(t, a, b, c,...)		struct { t a; t b; t c; __VA_ARGS__}
#define BITFIELD4(t, a, b, c, d,...)	struct { t a; t b; t c; t d; __VA_ARGS__}
#define BITFIELD5(t, a, b, c, d, e,...)	struct { t a; t b; t c; t d; t e; __VA_ARGS__}
#elif BYTE_ORDER == BIG_ENDIAN
#define BITFIELD2(t, a, b,...)			struct { t b; t a; __VA_ARGS__}
#define BITFIELD3(t, a, b, c,...)		struct { t c; t b; t a; __VA_ARGS__}
#define BITFIELD4(t, a, b, c, d,...)	struct { t d; t c; t b; t a; __VA_ARGS__}
#define BITFIELD5(t, a, b, c, d, e,...)	struct { t e; t d; t c; t b; t a; __VA_ARGS__}
#endif

/**
 * Macro to allocate a sized type.
 */
#define malloc_thing(thing) ((thing*)malloc(sizeof(thing)))

/**
 * Get the number of elements in an array
 */
#define countof(array) (sizeof(array)/sizeof((array)[0]) \
						+ BUILD_ASSERT_ARRAY(array))

/**
 * Ignore result of functions tagged with warn_unused_result attributes
 */
#define ignore_result(call) { if(call){}; }

/**
 * Assign a function as a class method
 */
#define ASSIGN(method, function) (method = (typeof(method))function)


typedef enum status_t status_t;

/**
 * Return values of function calls.
 */
enum status_t {
	/**
	 * Call succeeded.
	 */
	SUCCESS,

	/**
	 * Call failed.
	 */
	FAILED,

	/**
	 * Out of resources.
	 */
	OUT_OF_RES,

	/**
	 * The suggested operation is already done
	 */
	ALREADY_DONE,

	/**
	 * Not supported.
	 */
	NOT_SUPPORTED,

	/**
	 * One of the arguments is invalid.
	 */
	INVALID_ARG,

	/**
	 * Something could not be found.
	 */
	NOT_FOUND,

	/**
	 * Error while parsing.
	 */
	PARSE_ERROR,

	/**
	 * Error while verifying.
	 */
	VERIFY_ERROR,

	/**
	 * Object in invalid state.
	 */
	INVALID_STATE,

	/**
	 * Destroy object which called method belongs to.
	 */
	DESTROY_ME,

	/**
	 * Another call to the method is required.
	 */
	NEED_MORE,
};

/**
 * enum_names for type status_t.
 */
extern enum_name_t *status_names;

typedef enum tty_escape_t tty_escape_t;

/**
 * Excape codes for tty colors
 */
enum tty_escape_t {
	/** text properties */
	TTY_RESET,
	TTY_BOLD,
	TTY_UNDERLINE,
	TTY_BLINKING,

	/** foreground colors */
	TTY_FG_BLACK,
	TTY_FG_RED,
	TTY_FG_GREEN,
	TTY_FG_YELLOW,
	TTY_FG_BLUE,
	TTY_FG_MAGENTA,
	TTY_FG_CYAN,
	TTY_FG_WHITE,
	TTY_FG_DEF,

	/** background colors */
	TTY_BG_BLACK,
	TTY_BG_RED,
	TTY_BG_GREEN,
	TTY_BG_YELLOW,
	TTY_BG_BLUE,
	TTY_BG_MAGENTA,
	TTY_BG_CYAN,
	TTY_BG_WHITE,
	TTY_BG_DEF,
};

/**
 * Get the escape string for a given TTY color, empty string on non-tty fd
 */
char* tty_escape_get(int fd, tty_escape_t escape);

/**
 * deprecated pluto style return value:
 * error message, NULL for success
 */
typedef const char *err_t;

/**
 * Handle struct chunk_t like an own type.
 */
typedef struct sockaddr sockaddr_t;

/**
 * Portable function to wait for SIGINT/SIGTERM (or equivalent).
 */
void wait_sigint();

/**
 * Like dirname(3) returns the directory part of the given null-terminated
 * pathname, up to but not including the final '/' (or '.' if no '/' is found).
 * Trailing '/' are not counted as part of the pathname.
 *
 * The difference is that it does this in a thread-safe manner (i.e. it does not
 * use static buffers) and does not modify the original path.
 *
 * @param path		original pathname
 * @return			allocated directory component
 */
char *path_dirname(const char *path);

/**
 * Like basename(3) returns the filename part of the given null-terminated path,
 * i.e. the part following the final '/' (or '.' if path is empty or NULL).
 * Trailing '/' are not counted as part of the pathname.
 *
 * The difference is that it does this in a thread-safe manner (i.e. it does not
 * use static buffers) and does not modify the original path.
 *
 * @param path		original pathname
 * @return			allocated filename component
 */
char *path_basename(const char *path);

/**
 * Check if a given path is absolute.
 *
 * @param path		path to check
 * @return			TRUE if absolute, FALSE if relative
 */
bool path_absolute(const char *path);

/**
 * Creates a directory and all required parent directories.
 *
 * @param path		path to the new directory
 * @param mode		permissions of the new directory/directories
 * @return			TRUE on success
 */
bool mkdir_p(const char *path, mode_t mode);

#ifndef HAVE_CLOSEFROM
/**
 * Close open file descriptors greater than or equal to lowfd.
 *
 * @param lowfd		start closing file descriptors from here
 */
void closefrom(int lowfd);
#endif

/**
 * returns null
 */
void *return_null();

/**
 * No-Operation function
 */
void nop();

/**
 * returns TRUE
 */
bool return_true();

/**
 * returns FALSE
 */
bool return_false();

/**
 * returns FAILED
 */
status_t return_failed();

/**
 * returns SUCCESS
 */
status_t return_success();

/**
 * Get the padding required to make size a multiple of alignment
 */
static inline size_t pad_len(size_t size, size_t alignment)
{
	size_t remainder;

	remainder = size % alignment;
	return remainder ? alignment - remainder : 0;
}

/**
 * Round up size to be multiple of alignment
 */
static inline size_t round_up(size_t size, size_t alignment)
{
	return size + pad_len(size, alignment);
}

/**
 * Round down size to be a multiple of alignment
 */
static inline size_t round_down(size_t size, size_t alignment)
{
	return size - (size % alignment);
}

/**
 * Special type to count references
 */
typedef u_int refcount_t;

/* use __atomic* built-ins with GCC 4.7 and newer */
#ifdef __GNUC__
# if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 6))
#  define HAVE_GCC_ATOMIC_OPERATIONS
# endif
#endif

#ifdef HAVE_GCC_ATOMIC_OPERATIONS

#define ref_get(ref) __atomic_add_fetch(ref, 1, __ATOMIC_RELAXED)
/* The relaxed memory model works fine for increments as these (usually) don't
 * change the state of refcounted objects.  But here we have to ensure that we
 * free the right stuff if ref counted objects are mutable.  So we have to sync
 * with other threads that call ref_put().  It would be sufficient to use
 * __ATOMIC_RELEASE here and then call __atomic_thread_fence() with
 * __ATOMIC_ACQUIRE if we reach 0, but since we don't have control over the use
 * of ref_put() we have to make sure. */
#define ref_put(ref) (!__atomic_sub_fetch(ref, 1, __ATOMIC_ACQ_REL))
#define ref_cur(ref) __atomic_load_n(ref, __ATOMIC_RELAXED)

#define _cas_impl(ptr, oldval, newval) ({ typeof(oldval) _old = oldval; \
			__atomic_compare_exchange_n(ptr, &_old, newval, FALSE, \
										__ATOMIC_SEQ_CST, __ATOMIC_RELAXED); })
#define cas_bool(ptr, oldval, newval) _cas_impl(ptr, oldval, newval)
#define cas_ptr(ptr, oldval, newval) _cas_impl(ptr, oldval, newval)

#elif defined(HAVE_GCC_SYNC_OPERATIONS)

#define ref_get(ref) __sync_add_and_fetch(ref, 1)
#define ref_put(ref) (!__sync_sub_and_fetch(ref, 1))
#define ref_cur(ref) __sync_fetch_and_add(ref, 0)

#define cas_bool(ptr, oldval, newval) \
					(__sync_bool_compare_and_swap(ptr, oldval, newval))
#define cas_ptr(ptr, oldval, newval) \
					(__sync_bool_compare_and_swap(ptr, oldval, newval))

#else /* !HAVE_GCC_ATOMIC_OPERATIONS && !HAVE_GCC_SYNC_OPERATIONS */

/**
 * Get a new reference.
 *
 * Increments the reference counter atomically.
 *
 * @param ref	pointer to ref counter
 * @return		new value of ref
 */
refcount_t ref_get(refcount_t *ref);

/**
 * Put back a unused reference.
 *
 * Decrements the reference counter atomically and
 * says if more references available.
 *
 * @param ref	pointer to ref counter
 * @return		TRUE if no more references counted
 */
bool ref_put(refcount_t *ref);

/**
 * Get the current value of the reference counter.
 *
 * @param ref	pointer to ref counter
 * @return		current value of ref
 */
refcount_t ref_cur(refcount_t *ref);

/**
 * Atomically replace value of ptr with newval if it currently equals oldval.
 *
 * @param ptr		pointer to variable
 * @param oldval	old value of the variable
 * @param newval	new value set if possible
 * @return			TRUE if value equaled oldval and newval was written
 */
bool cas_bool(bool *ptr, bool oldval, bool newval);

/**
 * Atomically replace value of ptr with newval if it currently equals oldval.
 *
 * @param ptr		pointer to variable
 * @param oldval	old value of the variable
 * @param newval	new value set if possible
 * @return			TRUE if value equaled oldval and newval was written
 */
bool cas_ptr(void **ptr, void *oldval, void *newval);

#endif /* HAVE_GCC_ATOMIC_OPERATIONS */

#ifndef HAVE_FMEMOPEN
# ifdef HAVE_FUNOPEN
#  define HAVE_FMEMOPEN
#  define HAVE_FMEMOPEN_FALLBACK
#  include <stdio.h>
/**
 * fmemopen(3) fallback using BSD funopen.
 *
 * We could also provide one using fopencookie(), but should we have it we
 * most likely have fmemopen().
 *
 * fseek() is currently not supported.
 */
FILE *fmemopen(void *buf, size_t size, const char *mode);
# endif /* FUNOPEN */
#endif /* FMEMOPEN */

#endif /** UTILS_H_ @}*/
