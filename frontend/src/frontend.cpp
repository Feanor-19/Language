#include "frontend.h"

#include <ctype.h>
#include <math.h>

void print_status_message( FILE *stream, Status status )
{
    fprintf(stream, "%s", status_messages[status]);
}

int init_log( Config cfg )
{
    if (cfg.log_file_name)
    {
        if (!logger_init_log( cfg.log_file_name ))
        {
            return 0;
        }
    }
    else
    {
        if (!logger_init_log( stdout ))
        {
            return 0;
        }
    }

    return 1;
}

inline SepCharName get_sep_char( char c )
{
    for ( size_t sep_char = 0; sep_char < SIZEARR( SEPCHARS ); sep_char++ )
    {
        if ( SEPCHARS[sep_char].c == c )
            return SEPCHARS[sep_char].name;
    }
    return SEP_FICTIONAL;
}

inline KeywordName get_keyword( const char *str )
{
    for (size_t keyword = 0; keyword < SIZEARR( KEYWORDS ); keyword++)
    {

    }

    return KW_FICTIONAL;
}

Token get_token( const char *str )
{
    assert(str);

    Token tkn = {};
    if ( isdigit(*str) )
    {
        if ( sscanf( str, "%f%n", &tkn.num, (int*) &tkn.len ) == 1 )
            tkn.type = TKN_TYPE_NUM;
        else
            tkn.type = TKN_TYPE_ERROR;

        return tkn;
    }
    else if ( (tkn.sep_char = get_sep_char( *str ) ) != SEP_FICTIONAL )
    {
        tkn.type = TKN_TYPE_SEP_CHAR;
        tkn.len = 1;

        return tkn;
    }
    else if (  )
    {

    }
    else
    {

    }

    ASSERT_UNREACHEABLE();
}
