#ifndef COMMON_H
#define COMMON_H

#include "..\..\..\..\mylibheaders\tree.h"

#include "logger.h"


#define FREE(ptr) do{ if (ptr) free(ptr); ptr = NULL; }while(0)
#define ASSERT_UNREACHEABLE() assert( 0 && "Unreacheable line!" )

//! @brief Reads file with 'file_name' to dynamically allocated string
//! and returns it. If some error happens, returns NULL.
char *read_file_to_str( const char *file_name );

#endif /* COMMON_H */
