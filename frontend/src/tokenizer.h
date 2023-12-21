#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "common.h"



#define DEF_KEYWORD(name, str) KW_##name,
enum KeywordName
{
    #include "keywords.h"
    KW_FICTIONAL
};
#undef DEF_KEYWORD

struct Keyword
{
    KeywordName name;
    const char *str;
};

#define DEF_KEYWORD(name, str) { KW_##name, str },
const Keyword KEYWORDS[] =
{
    #include "keywords.h"
    {KW_FICTIONAL, "FICTIONAL!"}
};
#undef DEF_KEYWORD


#define DEF_SEPCHAR(name, char) SEP_##name,
enum SepCharName
{
    #include "sepchars.h"
    SEP_FICTIONAL,
};
#undef DEF_SEPCHAR

struct SepChar
{
    SepCharName name;
    char c;
};

#define DEF_SEPCHAR(name, char) { SEP_##name, char },
const SepChar SEPCHARS[] =
{
    #include "sepchars.h"
    {SEP_FICTIONAL, '\0'}
};
#undef DEF_SEPCHAR

//! @note 'start' points at the place in the _original_ string which is received by get_identificator().
//! It's not a c-string, so it doesn't end with '\0'! It must NOT be sent to free()!
struct Identificator
{
    const char *start;
    size_t len;
};

enum TokenType
{
    TKN_TYPE_NUM,
    TKN_TYPE_SEP_CHAR,
    TKN_TYPE_KEYWORD,
    TKN_TYPE_ID,
    TKN_TYPE_STR,
    TKN_TYPE_ERROR,
};

//! @note 'start' points at the place in the _original_ string which is received by get_token().
//! It's not a c-string, so it doesn't end with '\0'! It must NOT be sent to free()!
struct Token
{
    const char *start;
    TokenType type;
    union
    {
        num_t num;
        SepCharName sep_char;
        KeywordName keyword;
        Identificator id;
        Identificator str;
    };
    size_t len;
};



const char COMMENT_SYMBOL = '#';


//! @brief Retutns token, found in the prefix of the given 'str'.
//! See struct Token for details.
Token get_token( const char *str );

//! @brief Returns 1 if given token is a keyword and is the specfied keyword,
//! 0 otherwise.
int is_tkn_keyword( Token tkn, KeywordName keyword );

//! @brief Returns 1 if given token is one of the amplifiers, 0 otherwise.
int is_tkn_amp( Token tkn );

//! @brief Returns 1 if given token is a separating char
//! and is the specfied separating char, 0 otherwise.
int is_tkn_sep_char( Token tkn, SepCharName sep_char );

//! @brief Returns 1 if given identificators are equal,
//! i.e. if the const strings they point to are equal, otherwise 0.
int cmp_idents( Identificator a, Identificator b );

//! @brief Returns pointer to the first non-space symbol.
//! @note If '\0' is met, returns NULL!
const char *skip_spaces( const char *str );

#endif /* TOKENIZER_H */
