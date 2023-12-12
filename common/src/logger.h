#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>


//! @brief Opens file with name 'log_file_name' and
//! enables logging as simple log using 'log()'.
//! @return 1 if no error happens, 0 otherwise.
//! @note If the log is already initialized and not ended yet,
//! error is returned and nothing happens.
int logger_init_log( const char *log_file_name );

//! @brief Sets already opened 'opened_stream' as the
//! stream for the simple log to be wrritten into.
//! @return 1 if no error happens, 0 otherwise.
//! @note If the log is already initialized and not ended yet,
//! error is returned and nothing happens.
int logger_init_log( FILE *opened_stream );


//! @brief Opens file with name 'log_file_name' and
//! enables logging as warning log using 'log()'.
//! @return 1 if no error happens, 0 otherwise.
//! @note If the log is already initialized and not ended yet,
//! error is returned and nothing happens.
int logger_init_warn( const char *log_file_name );

//! @brief Sets already opened 'opened_stream' as the
//! stream for the warning log to be wrritten into.
//! @return 1 if no error happens, 0 otherwise.
//! @note If the log is already initialized and not ended yet,
//! error is returned and nothing happens.
int logger_init_warn( FILE *opened_stream );


//! @brief Opens file with name 'log_file_name' and
//! enables logging as error log using 'log()'.
//! @return 1 if no error happens, 0 otherwise.
//! @note If the log is already initialized and not ended yet,
//! error is returned and nothing happens.
int logger_init_err( const char *log_file_name );

//! @brief Sets already opened 'opened_stream' as the
//! stream for the error log to be wrritten into.
//! @return 1 if no error happens, 0 otherwise.
//! @note If the log is already initialized and not ended yet,
//! error is returned and nothing happens.
int logger_init_err( FILE *opened_stream );


// TODO - ф-я которая получает строчку (типа имя файла, например ".\logs\log"), а возвращает новую строку,
// в которой приписано дата и время (например ".\logs\log13_12_2023_00_11_19"), а эту новую строчку предполагается
// отправлять в качестве имени в logger_init


//! @brief Internal function, use macro 'LOG' instead.
void log_log_( const char *FILE,
               const int LINE,
               const char *FUNC,
               const char *format, ... );

#define LOG( format, ... ) log_log_( __FILE__, __LINE__, __func__, format, ... )

//! @brief Internal function, use macro 'WARNING' instead.
void log_warn_( const char *FILE,
                const int LINE,
                const char *FUNC,
                const char *format, ... );

#define WARNING( format, ... ) log_warn_( __FILE__, __LINE__, __func__, format, ... )

//! @brief Internal function, use macro 'ERROR' instead.
void log_err_( const char *FILE,
               const int LINE,
               const char *FUNC,
               const char *format, ... );

#define ERROR( format, ... ) log_err_( __FILE__, __LINE__, __func__, format, ... )


//! @brief Closes all opened logging streams.
void log_end();


#endif /* LOGGER_H */
