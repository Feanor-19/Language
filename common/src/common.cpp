#include "common.h"

#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>

inline off_t get_file_size(const char *file_name)
{
    assert(file_name);

    struct stat st_buf = {};

    if ( stat(file_name, &st_buf) == -1) return -1;

    return st_buf.st_size;
}

char *read_file_to_str( const char *file_name )
{
    assert(file_name);

    off_t file_size = get_file_size(file_name);
    if (file_size == -1)
        return NULL;

    FILE *file_p = fopen(file_name, "r");
    if (file_p == NULL)
        return NULL;

    char *str = (char *) calloc( file_size + 1, sizeof(char) );

    fread(str, sizeof(char), file_size, file_p);
    if ( ferror(file_p) != 0 )
    {
        free(str);
        return NULL;
    }

    fclose(file_p);

    return str;
}
