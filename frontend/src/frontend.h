#ifndef FRONTEND_H
#define FRONTEND_H

#include "common.h"
#include "config_front.h"
#include "tokenizer.h"


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



enum NametableType
{
    NT_TYPE_FUNC,
    NT_TYPE_FUNC_VAR,
};

struct FuncInfo
{
    FuncType func_type;
    size_t func_args_count;
};

struct NametableElem
{
    Identificator ident;
    union
    {
        FuncInfo func_info;
    };
};

struct Nametable
{
    NametableType type;
    NametableElem *list;
    size_t list_curr_len;
    size_t list_cap;
};

struct Nametables
{
    Nametable funcs;
    Nametable func_vars; //NOTE - переиспользуется внутри каждой функции, обязательно чистить после использования!
                         //NOTE - main на самом деле тоже ф-я, хоть это и не выглядит так грамматически
};

struct Context
{
    int in_func_action  = 0;
    int in_func_recipe  = 0;
    int in_while        = 0;
};

struct CompiledProgram
{
    Tree tree;
    Nametables nametables;
};


const size_t NAMES_DEFAULT_COUNT    = 10;


#define SYN_ASSERT( cond, prog, error_ptr, expected ) if (!(cond))  \
{                                                                   \
    print_rec_fall_err_msg( prog, error_ptr, expected );            \
    return NULL;                                                    \
}


//! @brief Prints message, corresponding to given 'status'.
void print_status_message( FILE *stream, Status status );

//! @brief Inits log according to given config.
int init_log( Config cfg );

Status compile_prog( const char *prog, CompiledProgram *comp_prog );

void CompiledProgram_dtor( CompiledProgram *comp_prog_ptr );

void print_rec_fall_err_msg( const char *prog, const char *error_ptr, const char *expected );

Status nametable_ctor( Nametable *nt_ptr, NametableType type );

Status Nametables_ctor( Nametables *nametables );

void nametable_dtor( Nametable *nametable );

void Nametables_dtor( Nametables *nametables );

#endif /* FRONTEND_H */
