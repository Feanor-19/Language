#ifndef FRONTEND_H
#define FRONTEND_H

// TODO - вторая попытка разобраться с этим
#include ".\..\..\common\src\common.h"
#include "config_front.h"



typedef float num_t;



#define DEF_STATUS(name, msg) STATUS_##name,
enum Status
{
    #include "front_statuses.h"
};
#undef DEF_STATUS

#define DEF_STATUS(name, message) message,
const char * const status_messages[] =
{
    #include "front_statuses.h"
    "FICTIONAL MESSAGE!"
};
#undef DEF_STATUS


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


struct Identificator
{
    const char *str;
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

struct Token
{
    TokenType type;
    union
    {
        num_t num;
        SepCharName sep_char;
        KeywordName keywrod;
        Identificator id;
    };
    size_t len;
};




void print_status_message( FILE *stream, Status status );

int init_log( Config cfg );

Token get_token( const char *str );

#endif /* FRONTEND_H */
