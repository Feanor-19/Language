#include "frontend.h"

#include <ctype.h>
#include <math.h>
#include ".\..\..\common\src\compiler_tree_dump.h"



// TODO - норм дефайны? вроде и сокращают, и вроде бы пониманию не мешают... "вродеы"
#define CURR (*curr_ptr)
#define FACT_REC_FALL_ARGS comp_prog, prog, curr_ptr
#define FORMAL_REC_FALL_ARGS CompiledProgram *comp_prog, const char *prog, char **curr_ptr
#define TREE &comp_prog->tree


static TreeNode *get_num        ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_while      ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_if         ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_assign     ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_var_death  ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_var_birth  ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_op         ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_prog       ( FORMAL_REC_FALL_ARGS );



//! @brief Returns index of given 'ident' in the 'nametable', if found;
//! otherwise returns -1;
inline int find_ident_in_nametable( Nametable nametable, Identificator ident )
{
    for (size_t ind = 0; ind < nametable.list_curr_len; ind++)
    {
        if ( cmp_idents( ident, nametable.list[ind] ) )
            return ind;
    }

    return -1;
}

//! @brief Returns 1 if ident is not found in any of the comp_prog's nametables, 0 otherwise.
inline int check_is_ident_fresh( CompiledProgram *comp_prog, Identificator ident )
{
    return find_ident_in_nametable( comp_prog->nametables.global_vars, ident )  == -1
        && find_ident_in_nametable( comp_prog->nametables.funcs,       ident )  == -1
        && find_ident_in_nametable( comp_prog->nametables.func_vars,   ident )  == -1;
}

static TreeNode *get_var( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);


}

static TreeNode *get_num( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);


}

static TreeNode *get_num( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);


}

static TreeNode *get_var_death( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);


}

static TreeNode *get_assign( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);


}

static TreeNode *get_if( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);


}

static TreeNode *get_while( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

}

static TreeNode *get_var_birth( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    Token var_birth = get_token( CURR );
    if ( !is_tkn_keyword( var_birth, KW_VarBirthOp ) )
        return NULL;
    CURR += var_birth.len;

    TreeNode *node_num = get_num( FACT_REC_FALL_ARGS );
    SYN_ASSERT(node_num, CURR, "Number");

    Token units_of = get_token( CURR );
    SYN_ASSERT( is_tkn_keyword(units_of, KW_UnitsOf), CURR, KEYWORDS[KW_UnitsOf].str );
    CURR += units_of.len;

    Token tkn_id = get_token( CURR );
    SYN_ASSERT( tkn_id.type != TKN_TYPE_ID || check_is_ident_fresh( comp_prog, tkn_id.id ) != -1,
                CURR, "Not mentioned before identificator" );

    Token dot = get_token( CURR );
    SYN_ASSERT( is_tkn_sep_char(dot, SEP_Dot), CURR, "\'!\'" );
    CURR += units_of.len;

    // TODO -
}

static TreeNode *get_op( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    TreeNode *node_op = NULL;

    // TODO - норм идея?

       ( node_op = get_var_birth( FACT_REC_FALL_ARGS ) )
    || ( node_op = get_var_death( FACT_REC_FALL_ARGS ) )
    || ( node_op =    get_assign( FACT_REC_FALL_ARGS ) )
    || ( node_op =        get_if( FACT_REC_FALL_ARGS ) )
    || ( node_op =     get_while( FACT_REC_FALL_ARGS ) );

    return node_op;
}

static TreeNode *get_prog( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    // ProgStart
    Token prog_start = get_token( CURR );
    SYN_ASSERT( is_tkn_keyword(prog_start, KW_ProgStart), CURR, KEYWORDS[KW_ProgStart].str );
    CURR += prog_start.len;

    // first operator (obligatory)
    TreeNode *node_first_op = get_op( FACT_REC_FALL_ARGS );
    SYN_ASSERT( node_first_op, CURR, "Operator" );

    // other operators (optional)
    TreeNode *node_new_op   = NULL;
    TreeNode *node_curr_op  = NULL;
    while ( (node_new_op = get_op( FACT_REC_FALL_ARGS )) )
    {
        if ( !node_curr_op )
        {
            TreeNode *node_first_seq = new_node_op( TREE, TREE_OP_SEQ_EXEC );
            tree_hang_loose_node_at_left( TREE, node_first_op, node_first_seq );
            node_curr_op = node_first_seq;
            node_first_op = node_first_seq;
        }

        TreeNode *node_new_seq = new_node_op( TREE, TREE_OP_SEQ_EXEC );
        tree_hang_loose_node_at_left( TREE, node_new_op, node_new_seq );
        tree_hang_loose_node_at_right( TREE, node_new_seq, node_curr_op );

        node_curr_op = node_new_seq;
    }

    if (node_curr_op)
        tree_migrate_into_right( TREE, tree_get_parent(node_curr_op), node_new_op );

    //ProgEnd
    Token prog_end = get_token( CURR );
    SYN_ASSERT( is_tkn_keyword(prog_end, KW_ProgEnd), CURR, KEYWORDS[KW_ProgEnd].str );
    CURR += prog_start.len;

    return node_first_op;
}

CompiledProgram compile_prog( const char *prog )
{
    assert(prog);

    Tree tree = {};
    tree_ctor( &tree, sizeof( TreeNodeData ), NULL, print_tree_node_data );

    CompiledProgram comp_prog = {};
    comp_prog.tree = tree;
    Nametables_ctor( &comp_prog.nametables );

    char *curr = 0;
    TreeNode *root = get_prog( &comp_prog, prog, &curr );
    if (!root)
    {
        tree_dtor( &tree ); // TODO - как-то вернуть ошибку в main
        return comp_prog;
    }

    tree_hang_loose_node_as_root( &tree, root );

    dump_compiler_tree( &tree );
}

void CompiledProgram_dtor( CompiledProgram *comp_prog_ptr )
{
    if ( comp_prog_ptr )
    {
        Nametables_dtor( &comp_prog_ptr->nametables );

        tree_dtor(&comp_prog_ptr->tree);
        comp_prog_ptr->tree = {};
    }
}

void print_tree_node_data( FILE *stream, void *data_ptr )
{
    assert(data_ptr);

    TreeNodeData data = *((TreeNodeData*)data_ptr);

    switch (data.type)
    {
    case TREE_NODE_TYPE_NUM:
        fprintf(stream, "data_type: NUM, data_value: %g", data.num);
        break;
    case TREE_NODE_TYPE_OP:
        fprintf(stream, "data_type: OP, data_value: %d", data.op);
        break;
    case TREE_NODE_TYPE_ID:
        fprintf(stream, "data_type: ID, data_value: %d", data.id);
        break;
    default:
        ASSERT_UNREACHEABLE();
        break;
    }
}

void print_rec_fall_err_msg( const char *str, const char *expected )
{
    assert(str);
    assert(expected);

    ERROR("Syntax error: expected <%s>, found:\n", expected);
    for (size_t ind = 0; ind < ERROR_MSG_LEN && str[ind] != '\0'; ind++)
    {
        ERROR( "%c", str[ind] );
    }
    ERROR( "%c", '\n' );
}

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

Status nametable_ctor( Nametable *nt_ptr )
{
    assert(nt_ptr);

    nt_ptr->list = (Identificator*) calloc( NAMES_DEFAULT_COUNT, sizeof(Identificator) );
    if (!nt_ptr->list)
    {
        ERROR(  "Memory allocation error: attempt to allocate %llu bytes",
                NAMES_DEFAULT_COUNT * sizeof(Identificator));
        return STATUS_ERROR_MEMORY_ALLOC_ERROR;
    }

    nt_ptr->list_curr_len = 0;
    nt_ptr->list_cap = NAMES_DEFAULT_COUNT;

    return STATUS_OK;
}

Status Nametables_ctor( Nametables *nametables )
{
    assert(nametables);

    if ( !nametable_ctor( &nametables->global_vars ) )
        return STATUS_ERROR_MEMORY_ALLOC_ERROR;

    if ( !nametable_ctor( &nametables->funcs ) )
    {
        FREE(nametables->global_vars.list);
        return STATUS_ERROR_MEMORY_ALLOC_ERROR;
    }

    if ( !nametable_ctor( &nametables->func_vars ) )
    {
        FREE(nametables->global_vars.list);
        FREE(nametables->funcs.list);
        return STATUS_ERROR_MEMORY_ALLOC_ERROR;
    }

    return STATUS_OK;
}

void nametable_dtor( Nametable *nametable )
{
    assert(nametable);

    FREE(nametable->list);
    nametable->list_cap = 0;
    nametable->list_curr_len = 0;
}

void Nametables_dtor( Nametables *nametables )
{
    assert(nametables);

    nametable_dtor( &nametables->global_vars );
    nametable_dtor( &nametables->funcs );
    nametable_dtor( &nametables->func_vars );
}
