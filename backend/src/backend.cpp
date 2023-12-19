#include "backend.h"

#define PRINT_ASM(format, ...) do{fprintf(stream, format, ##__VA_ARGS__); putc('\n', stream);}while(0)

inline size_t find_op_by_name( CompTreeOpName name )
{
    for (size_t ind = 0; ind < SIZEARR(COMP_TREE_OPS); ind++)
    {
        if ( COMP_TREE_OPS[ind].name == name )
            return ind;
    }

    WARNING( "ERROR: Can't find operators with name id <%d> in COMP_TREE_OPS. "
             "Pretending it is a dummy operator...", (int) name );
    return DUMMY_OP_INDEX;
}

Status tr_node_asm_text( FILE *stream, const Tree *tree_ptr, TreeNode *node, Counters *counters )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    TreeNodeData data = get_node_data( node );
    switch (data.type)
    {
    case TREE_NODE_TYPE_NUM:
        PRINT_ASM( "push %f", data.num );
        return STATUS_OK;
        break;
    case TREE_NODE_TYPE_ID:
        // NOTE - Identifiers must be dealt in operators' functions.
        ERROR( "Translation called for an alone identificator. Someting's wrong "
        "with the compiler tree and/or operators' functions. Stopping translation..." );
        return STATUS_ERROR_DURING_TRANSLATION;
        break;
    case TREE_NODE_TYPE_OP:
        // REVIEW - как нормально избавиться от crosses initialization??
        {size_t op_ind = find_op_by_name( data.op );
        Status status = COMP_TREE_OPS[ op_ind ].tr_asm_text( stream, tree_ptr, node, counters );
        return status;}
        break;
    default:
        ASSERT_UNREACHEABLE();
        break;
    }

    ASSERT_UNREACHEABLE();
    return STATUS_OK;
}

#undef PRINT_ASM

Status translate_to_asm_text( const Tree *tree_ptr, FILE *stream )
{
    assert(tree_ptr);
    assert(stream);

    Counters counters = {};

    return tr_node_asm_text( stream, tree_ptr, tree_get_root( tree_ptr ), &counters );
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
