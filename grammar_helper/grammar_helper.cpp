#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./../../../mylibs/onegin/headers/onegin.h"

int cmp_strs_by_in_prog( const void *a_ptr, const void *b_ptr );

int cmp_strs_by_in_grammar( const void *a_ptr, const void *b_ptr );

int main()
{

    FileBuf file_buf = read_file_to_buf( "input.txt" );

    Text text = parse_buf_to_text( file_buf );

    qsort( text.line_array, text.nLines, sizeof(char*), cmp_strs_by_in_grammar );

    print_text_to_file( "for_grammar.txt", text, 0, NULL );

    qsort( text.line_array, text.nLines, sizeof(char*), cmp_strs_by_in_prog );

    FILE *keywords_h = fopen( "keywords.h", "w" );

    for (size_t ind = 0; ind < text.nLines; ind++)
    {
        char *sep_char = strchr( text.line_array[ind] + 1, '|' );
        *sep_char = '\0';

        fprintf( keywords_h, "DEF_KEYWORD( %s, ", text.line_array[ind] + 1);

        *sep_char = '|';
        *( text.line_array[ind] + strlen(text.line_array[ind]) - 1 ) = '\0';
        fprintf(keywords_h, "\"%s\" )\n\n", sep_char + 1);
    }

    fclose(keywords_h);
}

int cmp_strs_by_in_prog( const void *a_ptr, const void *b_ptr )
{
    const char* a = *((const char*const*)a_ptr);
    const char* b = *((const char*const*)b_ptr);

    char* a_c = strchr( a + 1, '|' );
    char* b_c = strchr( b + 1, '|' );

    return strcmp( a_c + 1, b_c + 1 );
}

int cmp_strs_by_in_grammar( const void *a_ptr, const void *b_ptr )
{
    const char* a = *((const char*const*)a_ptr);
    const char* b = *((const char*const*)b_ptr);

    return strcmp( a, b );
}
