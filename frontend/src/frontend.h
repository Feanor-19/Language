#ifndef FRONTEND_H
#define FRONTEND_H

// TODO - вторая попытка разобраться с этим
#include ".\..\..\common\src\common.h"
#include "config_front.h"
#include ".\..\..\common\src\ops_in_tree.h"



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

enum NametableElemType
{
    NT_ELEM_TYPE_VAR,
};

//! @note 'start' points at the place in the _original_ string.
//! It's not a c-string, so it doesn't end with '\0'! It must NOT be sent to free()!
struct NametableElem
{
    NametableElemType type;
    const char *name_start;
    size_t name_len;
};

struct CompiledProgram
{
    Tree tree;
    struct
    {
        NametableElem *nametable;
        size_t nametable_ind;
        size_t nametable_cap;
    };
};


const size_t NAMES_DEFAULT_COUNT    = 5;
const size_t ERROR_MSG_LEN          = 20;


#define SYN_ASSERT( cond, str, expected ) if (!cond)    \
{                                                       \
    print_rec_fall_err_msg( str, expected );            \
    return NULL;                                        \
}


//! @brief Prints message, corresponding to given 'status'.
void print_status_message( FILE *stream, Status status );

//! @brief Inits log according to given config.
int init_log( Config cfg );

//! @brief Retutns token, found in the prefix of the given 'str'.
//! See struct Token for details.
Token get_token( const char *str );

CompiledProgram compile_prog( const char *prog );

void CompiledProgram_dtor( CompiledProgram *comp_prog_ptr );

void print_tree_node_data( FILE *stream, void *data_ptr );

void print_rec_fall_err_msg( const char *str, const char *expected );

#endif /* FRONTEND_H */
