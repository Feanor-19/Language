#ifndef TOKENIZER_H
#define TOKENIZER_H

#include ".\..\..\common\src\common.h"



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
    };
    size_t len;
};


#endif /* TOKENIZER_H */
