#include "tokenizer.h"

#include <assert.h>
#include <ctype.h>


//! @brief Returns SepCharName corresponding to given char 'c'.
//! If there is no such SepChar, return SEP_FICTIONAL.
inline SepCharName get_sep_char( char c )
{
    for ( size_t sep_char = 0; sep_char < SIZEARR( SEPCHARS ); sep_char++ )
    {
        if ( SEPCHARS[sep_char].c == c )
            return SEPCHARS[sep_char].name;
    }
    return SEP_FICTIONAL;
}

//! @brief If prefix of 'str' is equal to the whole 'keyword',
//! returns strlen(keyword), otherwise returns -1.
inline int cmp_keyword( const char *keyword, const char *str )
{
    assert(keyword);
    assert(str);

    size_t kw_ind = 0;
    size_t str_ind = 0;

    while (keyword[kw_ind] != '\0')
    {
        if ( str[str_ind] != keyword[kw_ind] )
            return -1;

        kw_ind++;
        str_ind++;
    }

    return (int) kw_ind;
}

//! @brief Returns KeywordName, found in the prefix of the given 'str'.
//! Puts length of the found keyword into *len_ptr.
//! If there is no corresponding KeywordName, returns KW_FICTIONAL.
inline KeywordName get_keyword( const char *str, size_t *len_ptr )
{
    assert(str);
    assert(len_ptr);

    int kw_len = 0;
    for (size_t keyword = 0; keyword < SIZEARR( KEYWORDS ); keyword++)
    {
        if ( (kw_len = cmp_keyword( KEYWORDS[keyword].str, str ) ) != -1 )
        {
            *len_ptr = (size_t) kw_len;
            return KEYWORDS[keyword].name;
        }
    }

    return KW_FICTIONAL;
}

//! @brief Returns identificator, found in the prefix of the given 'str'.
inline Identificator get_identificator( const char *str )
{
    size_t str_ind = 0;

    // first char
    if ( !isalpha(str[str_ind]) )
        return {NULL, 0};

    str_ind++;

    // other chars
    while ( isalnum(str[str_ind]) || str[str_ind] == '_' )
        str_ind++;

    return {str, str_ind};
}

Token get_token( const char *str )
{
    assert(str);

    str = skip_spaces(str);
    if (!str)
        return {NULL, TKN_TYPE_ERROR};

    Token tkn = {};
    tkn.start = str;

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
    else if ( ( tkn.keyword = get_keyword(str, &tkn.len) ) != KW_FICTIONAL )
    {
        tkn.type = TKN_TYPE_KEYWORD;

        return tkn;
    }
    else
    {
        Identificator id = get_identificator( str );
        if (id.start == NULL)
            return {NULL, TKN_TYPE_ERROR};

        tkn.type = TKN_TYPE_ID;
        tkn.len = id.len;
        tkn.id = id;

        return tkn;
    }

    ASSERT_UNREACHEABLE();
}