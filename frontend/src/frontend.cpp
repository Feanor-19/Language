#include "frontend.h"

#include <ctype.h>
#include <math.h>
#include ".\..\..\common\src\compiler_tree_dump.h"


static TreeNode *get_while( CompiledProgram *comp_prog, const char *str );
static TreeNode *get_if( CompiledProgram *comp_prog, const char *str );
static TreeNode *get_assign( CompiledProgram *comp_prog, const char *str );
static TreeNode *get_var_death( CompiledProgram *comp_prog, const char *str );
static TreeNode *get_var_birth( CompiledProgram *comp_prog, const char *str );
static TreeNode *get_op(  CompiledProgram *comp_prog, const char *str );
static TreeNode *get_prog( CompiledProgram *comp_prog, const char *str );


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


// TODO - норм дефайны? вроде и сокращают, и вроде пониманию не мешают
#define CURR (*curr_ptr)
#define ARGS comp_prog, prog, curr_ptr
#define TREE &comp_prog->tree



static TreeNode *get_var_death( CompiledProgram *comp_prog,
                                const char *prog,
                                char **curr_ptr )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);


}

static TreeNode *get_assign( CompiledProgram *comp_prog,
                             const char *prog,
                             char **curr_ptr )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);


}

static TreeNode *get_if( CompiledProgram *comp_prog,
                         const char *prog,
                         char **curr_ptr )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);


}

static TreeNode *get_while( CompiledProgram *comp_prog,
                            const char *prog,
                            char **curr_ptr )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

}

static TreeNode *get_var_birth( CompiledProgram *comp_prog,
                                const char *prog,
                                char **curr_ptr )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    Token var_birth_op = get_token( CURR );
    SYN_ASSERT( var_birth_op.type == TKN_TYPE_KEYWORD && var_birth_op.keyword == KW_VarBirthOp,
                 )
}

static TreeNode *get_op( CompiledProgram *comp_prog,
                         const char *prog,
                         char **curr_ptr )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    TreeNode *node_op = NULL;

    // TODO - норм идея?

       ( node_op = get_var_birth( ARGS ) )
    || ( node_op = get_var_death( ARGS ) )
    || ( node_op =    get_assign( ARGS ) )
    || ( node_op =        get_if( ARGS ) )
    || ( node_op =     get_while( ARGS ) );

    return node_op;
}

static TreeNode *get_prog( CompiledProgram *comp_prog,
                           const char *prog,
                           char **curr_ptr )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    // ProgStart
    Token prog_start = get_token( CURR );
    SYN_ASSERT( prog_start.type == TKN_TYPE_KEYWORD && prog_start.keyword == KW_ProgStart,
                CURR, KEYWORDS[KW_ProgStart].str );
    CURR += prog_start.len;

    // first operator (obligatory)
    TreeNode *node_first_op = get_op( ARGS );
    SYN_ASSERT( node_first_op, CURR, "Operator" );

    // other operators (optional)
    TreeNode *node_new_op   = NULL;
    TreeNode *node_curr_op  = NULL;
    while ( (node_new_op = get_op( ARGS )) )
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
    SYN_ASSERT( prog_start.type == TKN_TYPE_KEYWORD && prog_start.keyword == KW_ProgEnd,
                CURR, KEYWORDS[KW_ProgEnd].str );
    CURR += prog_start.len;

    return node_first_op;
}

CompiledProgram compile_prog( const char *prog )
{
    assert(prog);

    Tree tree = {};
    tree_ctor( &tree, sizeof( TreeNodeData ), NULL, print_tree_node_data );

    NametableElem *nametable = (NametableElem*) calloc( NAMES_DEFAULT_COUNT, sizeof( NametableElem ) );

    CompiledProgram comp_prog = {};
    comp_prog.tree = tree;
    comp_prog.nametable = nametable;
    comp_prog.nametable_ind = 0;
    comp_prog.nametable_cap = NAMES_DEFAULT_COUNT;

    char *curr = 0;
    TreeNode *root = get_prog( &comp_prog, prog, &curr );

    tree_hang_loose_node_as_root( &tree, root );

    dump_compiler_tree( &tree );
}

void CompiledProgram_dtor( CompiledProgram *comp_prog_ptr )
{
    if ( comp_prog_ptr )
    {
        FREE( comp_prog_ptr->nametable );

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
