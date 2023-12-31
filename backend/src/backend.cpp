#include "backend.h"

#define PRINT_ASM(format, ...) do{fprintf(stream, format, ##__VA_ARGS__); putc('\n', stream);}while(0)

inline size_t find_op_by_name( CompTreeOpNameEnum name )
{
    for (size_t ind = 0; ind < SIZEARR(COMP_TREE_OPS_BACKEND); ind++)
    {
        if ( COMP_TREE_OPS_BACKEND[ind].name == name )
            return ind;
    }

    WARNING( "ERROR: Can't find operators with name id <%d> in COMP_TREE_OPS_BACKEND. "
             "Pretending it is a dummy operator...", (int) name );
    return DUMMY_OP_INDEX;
}

Status tr_node_asm_text( FILE *stream, const Tree *tree_ptr,
                         TreeNode *node, Counters *counters,
                         Context *context, FuncFrames *frames )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    TreeNodeData data = get_node_data( node );
    switch (data.type)
    {
    case TREE_NODE_TYPE_NUM:
        PRINT_ASM( "%s %f", commands_list[CMD_PUSH], data.num );
        return STATUS_OK;
        break;
    case TREE_NODE_TYPE_ID:
        // NOTE - Function identifiers must be dealt in operators' functions.
        // Otherwise it is supposed that's a variable identificator.
        if ( context->in_func )
        {
            PRINT_ASM( "%s rbx", commands_list[CMD_PUSH] );
            PRINT_ASM( "%s %d", commands_list[CMD_PUSH], data.id );
            PRINT_ASM( "%s", commands_list[CMD_SUB] );
            PRINT_ASM( "%s rdx", commands_list[CMD_POP] );
            PRINT_ASM( "%s [rdx]", commands_list[CMD_PUSH] );
        }
        else
        {
            PRINT_ASM( "%s [%d]", commands_list[CMD_PUSH], data.id );
        }
        return STATUS_OK;
        break;
    case TREE_NODE_TYPE_OP:
        // REVIEW - как нормально избавиться от crosses initialization??
        {size_t op_ind = find_op_by_name( data.op );
        Status status = COMP_TREE_OPS_BACKEND[ op_ind ].tr_asm_text( stream, tree_ptr, node,
                                                             counters, context, frames );
        return status;}
        break;
    default:
        ASSERT_UNREACHEABLE();
        break;
    }

    ASSERT_UNREACHEABLE();
    return STATUS_OK;
}

Status translate_to_asm_text( const Tree *tree_ptr, FILE *stream )
{
    assert(tree_ptr);
    assert(stream);

    Counters counters = {};
    Context context   = {};
    FuncFrames frames = {};
    frames.list = (size_t*) calloc( FUNCS_DEFAULT_NUMBER, sizeof(size_t) );
    frames.list_cap = FUNCS_DEFAULT_NUMBER;

    PRINT_ASM( "%s %d", commands_list[CMD_PUSH], (int) MEMORY_SIZE - 3 );
    PRINT_ASM( "%s rcx", commands_list[CMD_POP] );
    PRINT_ASM( "%s main", commands_list[CMD_JMP] );

    Status err = tr_node_asm_text( stream, tree_ptr, tree_get_root( tree_ptr ),
                                   &counters, &context, &frames );
    if (err)
        return err;

    PRINT_ASM( "%s", commands_list[CMD_HLT] );

    return STATUS_OK;
}

#undef PRINT_ASM

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
