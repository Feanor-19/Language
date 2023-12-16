#ifndef FRONTEND_H
#define FRONTEND_H

// TODO - вторая попытка разобраться с этим
#include ".\..\..\common\src\common.h"
#include "config_front.h"
#include ".\..\..\common\src\ops_in_tree.h"
#include "tokenizer.h"


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



struct Nametable
{
    Identificator *list;
    size_t list_curr_len;
    size_t list_cap;
};

struct Nametables
{
    Nametable global_vars;
    Nametable funcs;
    Nametable func_vars; //NOTE - переиспользуется внутри каждой функции, обязательно чистить после использования!
};

struct CompiledProgram
{
    Tree tree;
    Nametables nametables;
};


const size_t NAMES_DEFAULT_COUNT    = 5;
const size_t ERROR_MSG_LEN          = 20;


#define SYN_ASSERT( cond, str, expected ) if (!(cond))    \
{                                                       \
    print_rec_fall_err_msg( str, expected );            \
    return NULL;                                        \
}


//! @brief Prints message, corresponding to given 'status'.
void print_status_message( FILE *stream, Status status );

//! @brief Inits log according to given config.
int init_log( Config cfg );

CompiledProgram compile_prog( const char *prog );

void CompiledProgram_dtor( CompiledProgram *comp_prog_ptr );

void print_tree_node_data( FILE *stream, void *data_ptr );

void print_rec_fall_err_msg( const char *str, const char *expected );

Status nametable_ctor( Nametable *nt_ptr );

Status Nametables_ctor( Nametables *nametables );

void nametable_dtor( Nametable *nametable );

void Nametables_dtor( Nametables *nametables );

#endif /* FRONTEND_H */
