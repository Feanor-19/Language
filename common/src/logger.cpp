#include "logger.h"

#include <assert.h>
#include <time.h>
#include <stdarg.h>


FILE *STREAM_LOG    = NULL;


inline void print_local_time_into_stream( FILE *stream )
{
    time_t curr_time = time(NULL);
    tm curr_local_time = *localtime(&curr_time);

    fprintf( stream, "%d-%02d-%02d_%02d-%02d-%02d", curr_local_time.tm_year + 1900,
             curr_local_time.tm_mon + 1, curr_local_time.tm_mday, curr_local_time.tm_hour,
             curr_local_time.tm_min, curr_local_time.tm_sec);
}

inline void write_log_header( void )
{
    fprintf( STREAM_LOG, "[LOG START TIME: " );
    print_local_time_into_stream( STREAM_LOG );
    fprintf( STREAM_LOG, "]\n" );
}

int logger_init_log( const char *log_file_name )
{
    assert( log_file_name );

    if ( STREAM_LOG )
        return 0;

    STREAM_LOG = fopen( log_file_name, "w" );
    if (!STREAM_LOG)
        return 0;

    write_log_header( );

    return 1;
}

int logger_init_log( FILE *opened_stream )
{
    assert( opened_stream );

    if ( STREAM_LOG )
        return 0;

    STREAM_LOG = opened_stream;

    return 1;
}

void log_( const char *type,
           const char *FILE_,
           const int LINE_,
           const char *FUNC_,
           const char *format, ... )
{
    if (!STREAM_LOG)
        return;

    fprintf( STREAM_LOG, "[%s][", type );
    print_local_time_into_stream( STREAM_LOG );
    fprintf( STREAM_LOG, "][%s(%d), %s()] ", FILE_, LINE_, FUNC_ );

    va_list args;
    va_start( args, format );
    vfprintf( STREAM_LOG, format, args );
    va_end(args);

    putc( '\n', STREAM_LOG );
}

void log_end()
{

}
