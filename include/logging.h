/*
 * Copyright (c) 2010, JetHead Development, Inc.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the JetHead Development nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file jhcommon/kernel/include/logging.h
 * @brief Ben, please provide a description here.
 *
 * @author Ben Payne
 * @date 08-17-04
 */

#ifndef _LOGGING_H_
#define _LOGGING_H_

//A little sanity checking. Can't be verbose and release at the same time
#if defined(JH_VERBOSE_LOGGING) && defined(JH_PRODUCTION_LOGGING)
#warning Undefining JH_VERBOSE_LOGGING when JH_PRODUCTION_LOGGING defined
#undef JH_VERBOSE_LOGGING
#endif

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>

#include "jh_types.h"

typedef int (command_parser_t)( char *name, int size );

__BEGIN_DECLS

extern int jh_log_indent;
extern int jh_log_indent_size;
extern const char* jh_log_level_names[];

// used to fixup the function name generated by __PRETTY_FUNCTION__ into a
//  format we like.  This function is completely thread safe.
int function_name_fixup( const char *name, char *buffer, int len );

// The print routine for logging.  This is a function because it will use stack
//  space to build the buffer for printing.
void jh_log_print( int level, const char *function, const char *file, int line, const char *fmt, ... )
		__attribute__ ((__format__ (__printf__, 5, 6)));

// DEPRECATED.  This was once used instead of removing logging in a release 
//  build.  It would allow logging statements that had effect (which are very
//  bad) to still run in a release build event though these were not printed.
void jh_null_print( int level, const char *fmt, ... )
		__attribute__ ((__format__ (__printf__, 2, 3)));
		
/**
 * print a buffer of data to the screen.  prints a hex 8 bytes per line.
 *
 * @param str This string will print at the start of each line followed by a ":"
 * @param buffer The buffer of data to print.
 * @param len The length of data to print.
 */
void print_buffer( const char *str, const void *data, int len );

/**
 * print a buffer of data to the screen.  prints 16 hex bytes per line with 
 *  ascii interprtation at the end of the line.
 *
 * @param str This string will print at the start of each line followed by a ":"
 * @param buffer The buffer of data to print.
 * @param len The length of data to print.
 */
void print_buffer2( const char *str, const void *data, int len );

/**
 * Add this to startup code in a file to register it with the logging system.
 *  This will allow dynamic control over logging level.  This is ONLY needed 
 *  for C.  C++ code will auto register at startup.
 */
#define INIT_LOGGING() \
	register_logging( __FILE__, &_file_log_cat, &_file_log_level )

/**
 * DON'T call this in your code. Instead add INIT_LOGGING to some startup code.
 */
void register_logging( const char *filename, uint32_t *cats,
					   int *level );

/**
 * Update the logging level on any file with the name @param filename.  If 
 *  multiple files have the same name all files with that name will be updated.
 *  If the filename is "all" then all files will be updated to this level.
 *
 * @param filename The filename to match on, or "all" will change all files.
 * @param level The new log level
 *
 * @return 0 if file not found, 1 otherwise
 */
int logging_set_level( const char *filename, int level );

/**
 * Update the logging categories on any file with the name @param filename.  
 *  If multiple files have the same name all files with that name will be 
 *  updated.  If the filename is "all" then all files will be updated to this 
 *  level.
 *
 * @param filename The filename to match on, or "all" will change all files.
 * @param cats The new logging category
 *
 * @return 0 if file not found, 1 otherwise
 */
int logging_set_cats( const char *filename, uint32_t cats );

/**
 * This will find the first file with this name and return it's caregories.
 *
 * @param filename The filename to match on.
 *
 * @return The categories for the file, or 0 if file not found.
 */
uint32_t logging_get_cats( const char *filename );

/**
 * This will find the first file with this name and return it's log level.
 *
 * @param filename The filename to match on.
 *
 * @return The log level for the file, or -1 if file not found.
 */
int logging_get_level( const char *filename );

/**
 * Set the sync mode for logging.  IF this is true after every log we will 
 *  flush the file.  This ensures that your file will contain the last line of 
 *  logging after a crash.  The drawback is that your code will be blocked 
 *  waiting for the IO operation to complete, this means the logging is more
 *  intrusive of the execution of your code.
 *
 * @param mode The mode, true enables sync logging.
 */
void logging_set_sync_mode( bool mode );

/**
 * This can be called to force the logging to flush all it's buffer.  This is 
 *  only usefull of sync mode has been set to false.
 */
void logging_sync( void );

/**
 * Return a whitespace separated list of the filenames in the list
 * being monitored, or NULL if no files are being logged.
 */
const char* logging_get_names( void );

/**
 * Convert a string into a logging level.  This will look for a numeric value
 *  or a string.  If this string doesn't parse properly then a -1 is returned.
 */
int logging_lookup_level( const char *str );

/**
 * Convert a string into a logging cat.  If the string fails to parse a 0 
 *  is returned.
 */
uint32_t logging_lookup_cat( const char *str );

/**
 * Init the logging system.  This is called internally.  The user should never 
 *  calls this directly.
 */
void logging_init( void );

/**
 * Cleanup any logging data.  Currently never called by userspace apps.   
 */
void logging_cleanup( void );

/**
 *	@brief Set logging output file
 *
 * This method sets the logging output file.  The default file is to print
 *  to "stdout".  It is up to the caller to eventually close this file.
 *
 * @param new_file the file that you want logging to print to.  The logging 
 *  system will not close this file for you.
 */
void logging_set_file( FILE *new_file );

/**
 *	@brief Set logging copy file
 *
 * This method sets the logging copy file.  It can be used to copy
 *  all logging to a separate filename (with different append overwrite 
 *  directives, for example).  If not set, the logging output is not copied.
 *
 * @param copy_file the file that you want logging to print to.  The logging 
 *  system will not close this file for you.
 */
void logging_set_copy_file( FILE* copy_file );

/**
 * Add a mark to the log file.  This will be a obvious section of lines in the 
 *  log file.  Usefull for marking a start point to potential interesting stuff
 *  in the file.  If num is >=0 it will print the number as part of the mark, 
 *  otherwise it will just print a mark.
 */
void logging_mark( int num );

void logging_show_files( FILE *file );

static inline const char *getVersionString(void)
{
	return JH_VERSION_STRING;
}

// Macro for adding a DTV compliance version string to a program or library.
#define JH_DECLARE_VERSION_STRING( sysname ) \
  static char version_number[] __attribute__ ((unused)) = "00aa " sysname ":" JH_VERSION_STRING " aa00"; \
  static char jh_revision_string[] __attribute__ ((unused)) = JH_REVISION_STRING ""

__END_DECLS


#define LOG_BIT_VALUE( x )	( 1 << x )

/**
 * @brief Logging Categories -
 * these are the system defined categories, if you would like to log
 * with additional categories in your code then define them in your
 * module and use them appropriately, start with bit 0 and go up.
 * global one if added will start with the most significant bit and
 * work down.
 */
#define LOG_CAT_DEFAULT	LOG_BIT_VALUE( 31 )
#define LOG_CAT_TRACE	LOG_BIT_VALUE( 30 )

#define LOG_CAT_ALL		0xffffffff

/**
 * @brief Logging Levels - these are a subset of the kernel levels,
 * they do map to the kernel levels if you need to log at a level
 * greater than err you should consider a panic.
 */

/**
 * @brief LOG_LVL_ERR - logging at this level will always be
 * enabled. Use this to print something that is an error that should
 * never happen.
 */
#define LOG_LVL_ERR				0

/**
 * @brief LOG_LVL_ERR_PERROR - logging at this level will always be
 * enabled.  This is the same as LOG_LVL_ERR but it will also do a
 * perror to show the errno.
 */
#define LOG_LVL_ERR_PERROR		1

/**
 * @brief LOG_LVL_WARN - logging at this level will always be
 * enabled. Use this to print something that is an error but is
 * recoverable.
 */
#define LOG_LVL_WARN			2

/**
 * @brief LOG_LVL_WARN_PERROR - logging at this level will always be
 * enabled.  This is the same as LOG_LVL_WARN but it will also do a
 * perror to show the errno.
 */
#define LOG_LVL_WARN_PERROR		3

/**
 * @brief LOG_LVL_NOTICE - Startup and shutdown code may log at this
 * level (as long as it is not excessive). This can assist us in
 * diagnosing startup and shutdown issues from customer log files.
 */
#define LOG_LVL_NOTICE	4

/**
 * @brief LOG_LVL_INFO - This level is for more detailed startup and
 * shutdown logging, for other functions (ioctl, etc...) as long as
 * their frequency of calling is not typically greater than once per 5
 * seconds.
 */
#define LOG_LVL_INFO	5

/**
 * @brief LOG_LVL_NOISE - Code that runs frequently such as code that
 * processes streaming data.
 */
#define LOG_LVL_NOISE	6


#define JH_FUNCTION_NAME  __PRETTY_FUNCTION__

/*
 * TRACING SECTION
 * 
 * This is the tracing support.  It work by adding a TRACE_BEGIN macro to the
 *  start of a function.  TRACE_BEGIN takes a logging level and will only log
 *  if the level of the file is higher than or equal to this level.  In C++ 
 *  code TRACE_BEGIN is all you need since end will print whenever the function
 *  exits.  In C++ TRACE_END does nothing, however in C code it is needed 
 *  other end will not print.  TRACE_END_ERR can be used to print an error
 *  condition that cause the exit.  This will always print the message at
 *  LOG_LVL_ERROR.  TRACE_END_ERR work in both C and C++.  In a release build
 *  all tracing is disabled except TRACE_END_ERR will still print an error.
 */
#ifndef JH_VERBOSE_LOGGING

#define TRACE_BEGIN(x)
#define TRACE_END()
#define TRACE_END_ERR( fmt, args... )									\
	do {																\
		JH_LOG_ALWAYS( LOG_LVL_ERR, LOG_CAT_TRACE, "end, " fmt, ## args ); \
	} while (0)

#else // JH_VERBOSE_LOGGING

#ifdef  __cplusplus

class Tracer {
public:
	Tracer( int level, const char *name, const char *file, int line, 
			volatile uint32_t& file_cats, volatile int& file_level ) :
		mLevel( level ), mName( name ), mFile( file ), mLineNum( line ),
		mFileCats( file_cats ), mFileLevel( file_level ), mPrintExit( true )
	{
		if ( (LOG_CAT_TRACE & file_cats) && (mLevel <= file_level) )
		{
			jh_log_indent += jh_log_indent_size;
			jh_log_print( mLevel, mName, mFile, mLineNum, "begin" );
		}
	}

	~Tracer()
	{
		if ( mPrintExit == true && (LOG_CAT_TRACE & mFileCats) && (mLevel <= mFileLevel) )
		{
			jh_log_indent = jh_log_indent <=0  ? 0 : jh_log_indent - jh_log_indent_size;
			jh_log_print( mLevel, mName, mFile, mLineNum, "end" );
		}
	}

	void earlyExit() { mPrintExit = false; }

	int getLevel() { return mLevel; }

private:
	int mLevel;
	const char *mName;
	const char *mFile;
	int mLineNum;
	volatile uint32_t& mFileCats;
	volatile int& mFileLevel;
	bool mPrintExit;
};

#define TRACE_BEGIN(level)		\
	Tracer _tracer( level, JH_FUNCTION_NAME, __FILE__, __LINE__, _file_log_cat, _file_log_level )

#define TRACE_END()

#define TRACE_END_ERR( fmt, args... )		\
	_tracer.earlyExit(); \
	JH_LOG_ALWAYS( LOG_LVL_ERR, LOG_CAT_TRACE, "end, " fmt, ## args )

#else // !__cplusplus

#define TRACE_BEGIN(level)													 \
	int _function_log_level;											\
	_function_log_level = level;										\
	JH_LOG( level, LOG_CAT_TRACE, "begin" );							 \
	if ( (LOG_CAT_TRACE & _file_log_cat) && (level <= _file_log_level) ) \
		jh_log_indent += jh_log_indent_size

#define TRACE_END()		\
	do {																					\
		if ( (LOG_CAT_TRACE & _file_log_cat) && (_function_log_level <= _file_log_level) )	\
			jh_log_indent = jh_log_indent <=0  ? 0 : jh_log_indent - jh_log_indent_size;	\
		JH_LOG( _function_log_level, LOG_CAT_TRACE, "end" );								\
	} while (0)

#define TRACE_END_ERR( fmt, args... )		\
	do {																					\
		if ( (LOG_CAT_TRACE & _file_log_cat) && (_function_log_level <= _file_log_level) )	\
			jh_log_indent = jh_log_indent <=0  ? 0 : jh_log_indent - jh_log_indent_size;	\
		JH_LOG_ALWAYS( LOG_LVL_ERR, LOG_CAT_TRACE, "end, " fmt, ## args );					\
	} while (0)

#endif // __cplusplus

#endif // !JH_VERBOSE_LOGGING

/*
 * SETTING LOGGING LEVELS
 *
 * This section defines macros that go at the top of every file that uses 
 *  logging.  It set the logging level and categories for a file.  In a release
 *  system this evals to nothing.  In C++ this code will auto register the
 *  file with the logging system.  With C code the file will need to register
 *  itself with "register_logging".
 * 
 */

#ifdef  __cplusplus
class AutoRegister {
public:
	AutoRegister( const char *file, uint32_t *cats,
				  int *level )
	{
		register_logging( file, cats, level );
	}
};

#define SET_LOG_CAT(x)		static uint32_t _file_log_cat __attribute__ ((unused)) = x
#define SET_LOG_LEVEL(x)	static int _file_log_level __attribute__ ((unused)) = x; \
							static AutoRegister _file_log_reg( __FILE__, &_file_log_cat, &_file_log_level )

#define LOG( fmt, args... )		JH_LOG( _tracer.getLevel(), LOG_CAT_DEFAULT, fmt, ## args )

#else // __cplusplus

#ifdef JH_VERBOSE_LOGGING

#define SET_LOG_CAT(x)		static uint32_t _file_log_cat __attribute__ ((unused)) = x
#define SET_LOG_LEVEL(x)	static int _file_log_level __attribute__ ((unused)) = x

#define LOG( fmt, args... )		JH_LOG( _function_log_level, LOG_CAT_DEFAULT, fmt, ## args )

#else // !JH_VERBOSE_LOGGING

#define SET_LOG_CAT(x)		
#define SET_LOG_LEVEL(x)	

#define LOG( fmt, args... )		

#endif // !JH_VERBOSE_LOGGING

#endif // __cplusplus


/*
 * FATAL ERROR HANDLERS
 *
 * USERSPACE ONLY
 *
 * These macros can be used to deal with fatal errors.  There is a log type 
 *  macro and a "assert" style macro.  Our assert takes logging string tool.
 *  You can always pass "" (empty string) if you don't want to log anthing 
 *  on the assert.  You still get a print with file and line number.
 *
 * In a release build these will still log the message however they will not
 *  abort.
 */

#ifndef JH_VERBOSE_LOGGING

#define LOG_ERR_FATAL( fmt, args... )		\
do { \
	LOG_ERR_PERROR( fmt, ## args ); \
} while ( 0 )

#define ASSERT_ERR( eval, fmt, args... )	\
do { \
	if( !(eval) ) { \
		JH_LOG_ALWAYS( LOG_LVL_ERR_PERROR, LOG_CAT_DEFAULT, fmt, ## args ); \
	} \
} while( 0 )

#else // JH_VERBOSE_LOGGING

#define LOG_ERR_FATAL( fmt, args... )		\
do { \
	LOG_ERR_PERROR( fmt, ## args ); \
	abort(); \
} while( 0 )

#define ASSERT_ERR( eval, fmt, args... )	\
do { \
	if( !(eval) ) { \
		JH_LOG_ALWAYS( LOG_LVL_ERR_PERROR, LOG_CAT_DEFAULT, fmt, ## args ); \
		abort(); \
	} \
} while( 0 )

#endif // JH_VERBOSE_LOGGING

/*
 * NON-FATAL ASSERT
 *
 * USERSPACE and KERNEL
 *
 * This macro give assert type function but without aborting the system.  
 */

#define ASSERT_WARN( eval, fmt, args... )	\
do { \
	if( !(eval) ) { \
		JH_LOG_ALWAYS( LOG_LVL_WARN, LOG_CAT_DEFAULT, fmt, ## args ); \
	} \
} while( 0 )

/*
 * LOG MACROS
 * 
 * USERSPACE and KERNEL
 *
 * These macros will cause logging at various specific levels.  These are what 
 *  you want to use in your code.
 */

#define LOG_ERR( fmt, args... )				JH_LOG_ALWAYS( LOG_LVL_ERR, LOG_CAT_DEFAULT, fmt, ## args )
#define LOG_WARN( fmt, args... )			JH_LOG_ALWAYS( LOG_LVL_WARN, LOG_CAT_DEFAULT, fmt, ## args )
#define LOG_ERR_PERROR( fmt, args... )		JH_LOG_ALWAYS( LOG_LVL_ERR_PERROR, LOG_CAT_DEFAULT, fmt, ## args )
#define LOG_WARN_PERROR( fmt, args... )		JH_LOG_ALWAYS( LOG_LVL_WARN_PERROR, LOG_CAT_DEFAULT, fmt, ## args )
#define LOG_NOTICE( fmt, args... )			JH_LOG( LOG_LVL_NOTICE, LOG_CAT_DEFAULT, fmt, ## args )
#define LOG_INFO( fmt, args... )			JH_LOG( LOG_LVL_INFO, LOG_CAT_DEFAULT, fmt, ## args )
#define LOG_NOISE( fmt, args... )			JH_LOG( LOG_LVL_NOISE, LOG_CAT_DEFAULT, fmt, ## args )
#define LOG_CAT_NOTICE( cat, fmt, args... )	JH_LOG( LOG_LVL_NOTICE, cat, fmt, ## args )
#define LOG_CAT_INFO( cat, fmt, args... )	JH_LOG( LOG_LVL_INFO, cat, fmt, ## args )
#define LOG_CAT_NOISE( cat, fmt, args... )	JH_LOG( LOG_LVL_NOISE, cat, fmt, ## args )

#define PRINT_BUFFER_ERR( str, buf, len )		JH_PRINT_BUFFER( LOG_LVL_ERR, str, buf, len )
#define PRINT_BUFFER_WARN( str, buf, len )		JH_PRINT_BUFFER( LOG_LVL_WARN, str, buf, len )
#define PRINT_BUFFER_NOTICE( str, buf, len )	JH_PRINT_BUFFER( LOG_LVL_NOTICE, str, buf, len )
#define PRINT_BUFFER_INFO( str, buf, len )		JH_PRINT_BUFFER( LOG_LVL_INFO, str, buf, len )
#define PRINT_BUFFER_NOISE( str, buf, len )		JH_PRINT_BUFFER( LOG_LVL_NOISE, str, buf, len )

/*
 * LOGGING INTERNALS
 *
 */
 
#define JH_LOG_OUTPUT_BUF_SIZE           512

#ifndef JH_VERBOSE_LOGGING

#define JH_LOG( level, cat, fmt, args... )
#define JH_PRINT_BUFFER( level, str, buf, len )

#else //JH_VERBOSE_LOGGING

#define JH_LOG( level, cat, fmt, args... ) 					\
do { \
	if ( (cat & _file_log_cat) && (level <= _file_log_level) )			\
		jh_log_print( level, JH_FUNCTION_NAME, __FILE__, __LINE__, fmt, ## args );\
} while (0)

#define JH_PRINT_BUFFER( level, str, buf, len ) \
do { \
	if ( (level <= _file_log_level) )	\
		print_buffer2( str, buf, len );	\
} while (0)

#endif //JH_VERBOSE_LOGGING


#ifndef JH_PRODUCTION_LOGGING

#define JH_LOG_ALWAYS( level, cat, fmt, args... ) 					\
	jh_log_print( level, JH_FUNCTION_NAME, __FILE__, __LINE__, fmt, ## args )

#else //JH_PRODUCTION_LOGGING

#define JH_LOG_ALWAYS( level, cat, fmt, args... )

#endif //JH_PRODUCTION_LOGGING

#else // _LOGGING_H_
#ifndef _INCLUDING_LOGGING_IN_HEADER_
#warning Do not include logging.h in header files.
#endif
#endif
