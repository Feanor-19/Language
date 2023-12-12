#include "logger.h"

#include <assert.h>


FILE *STREAM_LOG    = NULL;
FILE *STREAM_WARN   = NULL;
FILE *STREAM_ERROR  = NULL;


int logger_init_log( const char *log_file_name )
{
    assert( log_file_name );

    if ( STREAM_LOG )
        return 0;

    STREAM_LOG = fopen( log_file_name, "w" );
    if (!STREAM_LOG)
        return 0;

    // TODO - write header

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

int logger_init_warn( const char *log_file_name )
{
    assert( log_file_name );

    if ( STREAM_WARN )
        return 0;

}

int logger_init_warn( FILE *opened_stream )
{
    assert( opened_stream );

    if ( STREAM_WARN )
        return 0;

    STREAM_WARN = opened_stream;

    return 1;
}

int logger_init_err( const char *log_file_name )
{
    assert( log_file_name );

    if ( STREAM_ERROR )
        return 0;


}

int logger_init_err( FILE *opened_stream )
{
    assert( opened_stream );

    if ( STREAM_ERROR )
        return 0;

    STREAM_ERROR = opened_stream;

    return 1;
}

void log_log_( const char *FILE,
               const int LINE,
               const char *FUNC,
               const char *format, ... )
{

}

void log_warn_( const char *FILE,
                const int LINE,
                const char *FUNC,
                const char *format, ... )
{

}

void log_err_( const char *FILE,
               const int LINE,
               const char *FUNC,
               const char *format, ... )
{

}

void log_end()
{

}
