#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>


//! @brief Opens file with name 'log_file_name' and
//! enables logging.
//! @return 1 if no error happens, 0 otherwise.
//! @note If the log is already initialized and not ended yet,
//! error is returned and nothing happens.
int logger_init_log( const char *log_file_name );

//! @brief Sets already opened 'opened_stream' as the
//! stream for logs to be wrritten into.
//! @return 1 if no error happens, 0 otherwise.
//! @note If the log is already initialized and not ended yet,
//! error is returned and nothing happens.
int logger_init_log( FILE *opened_stream );

//! @brief Internal function, use macros 'LOG', 'WARNING' and 'ERROR' instead.
void log_( const char *type,
           const char *FILE,
           const int LINE,
           const char *FUNC,
           const char *format, ... );

#define LOG(format,...) log_( "LOG", __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#define WARNING(format,...) log_( "WRN", __FILE__, __LINE__, __func__, format, ##__VA_ARGS__ )

#define ERROR(format,...) log_( "ERR", __FILE__, __LINE__, __func__, format, ##__VA_ARGS__ )


//! @brief Closes all opened logging streams.
void log_end( void );

//! @brief Returns current log stream.
FILE *log_get_stream( void );

#endif /* LOGGER_H */
