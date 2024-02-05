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

    // TODO - определение самого длинного префикса, который является ключевым словом,
    // а не первого попавшегося

    // TODO - идея алгоритма "равномерного распространения" по всем ключевым словам сразу?

    int kw_len = 0;
    KeywordName ans = KW_FICTIONAL;
    size_t ans_len = 0;
    for (size_t keyword = 0; keyword < SIZEARR( KEYWORDS ); keyword++)
    {
        if ( (kw_len = cmp_keyword( KEYWORDS[keyword].str, str ) ) != -1 && (size_t) kw_len > ans_len )
        {
            ans_len = (size_t) kw_len;
            ans = KEYWORDS[keyword].name;
        }
    }

    *len_ptr = ans_len;
    return ans;
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

//! @brief Returns everything surrounded by qoutes. *str must be \"
//! (given string must start with quote).
inline Identificator get_str( const char *str )
{
    Identificator res = {};
    str++;
    res.start = str;
    while( *str != '"' && *str != '\n' && *str != '\0' )
    {
        str++;
    }

    if (*str == '\n' || *str == '\0')
        return {NULL, 0};

    res.len = str - res.start;
    if (res.len == 0)
        return {NULL, 0};

    return res;
}

Token get_token( const char *str )
{
    assert(str);

    str = skip_spaces(str);
    if (!str)
        return {NULL, TKN_TYPE_ERROR};

    Token tkn = {};
    tkn.start = str;

    if ( *str == '"' )
    {
        tkn.type = TKN_TYPE_STR;
        tkn.str = get_str( str );
        if ( !tkn.str.start )
            tkn.type = TKN_TYPE_ERROR;
        tkn.len = tkn.str.len + 2;
        return tkn;
    }
    else if ( isdigit(*str) )
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

int is_tkn_keyword( Token tkn, KeywordName keyword )
{
    return tkn.type == TKN_TYPE_KEYWORD && tkn.keyword == keyword;
}

int is_tkn_sep_char( Token tkn, SepCharName sep_char )
{
    return tkn.type == TKN_TYPE_SEP_CHAR && tkn.sep_char == sep_char;
}

int cmp_idents( Identificator a, Identificator b )
{
    if ( a.len != b.len || !a.start || !b.start )
        return 0;

    for (size_t ind = 0; ind < a.len; ind++)
    {
        if ( a.start[ind] != b.start[ind] )
            return 0;
    }

    return 1;
}

// NOTE - не забывать обновлять список амплифайеров
int is_tkn_amp( Token tkn )
{
    return tkn.type == TKN_TYPE_KEYWORD &&
           (tkn.keyword == KW_Amp_1
         || tkn.keyword == KW_Amp_2
         || tkn.keyword == KW_Amp_3
         || tkn.keyword == KW_Amp_4
         || tkn.keyword == KW_Amp_5
         || tkn.keyword == KW_Amp_6
         || tkn.keyword == KW_Amp_7
         || tkn.keyword == KW_Amp_8
         || tkn.keyword == KW_Amp_9
         || tkn.keyword == KW_Amp_10);
}

inline int is_space_or_comment( char c )
{
    static int is_in_comment = 0;

    if (!is_in_comment)
    {
        if (c == COMMENT_SYMBOL)
        {
            is_in_comment = 1;
            return 1;
        }
        else
        {
            return isspace(c);
        }
    }
    else if (is_in_comment)
    {
        if ( c == COMMENT_SYMBOL )
            is_in_comment = 0;
        return 1;
    }

    ASSERT_UNREACHEABLE();
    return 0;
}

const char *skip_spaces( const char *str )
{
    assert(str);

    while ( is_space_or_comment(*str) )
        str++;

    if ( *str != '\0' )
        return str;

    return NULL;
}
