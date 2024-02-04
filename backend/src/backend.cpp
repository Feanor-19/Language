#include "backend.h"

#define PRINT_ASM(format, ...) do{fprintf(stream, format, ##__VA_ARGS__); putc('\n', stream);}while(0)

// TODO - разобраться какого фига эта ф-я вообще существует
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
                         Context *context )
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
        if ( data.id + 3 > context->curr_func_frame_size )
            context->curr_func_frame_size = data.id + 3;

        PRINT_ASM( "%s " REG_FRAME_REF_CELL , commands_list[CMD_PUSH] );
        PRINT_ASM( "%s %d", commands_list[CMD_PUSH], data.id );
        PRINT_ASM( "%s", commands_list[CMD_ADD] );
        PRINT_ASM( "%s " REG_COMP, commands_list[CMD_POP] );
        PRINT_ASM( "%s [" REG_COMP "]", commands_list[CMD_PUSH] );
        return STATUS_OK;
        break;
    case TREE_NODE_TYPE_OP:
        // REVIEW - как нормально избавиться от crosses initialization??
        {size_t op_ind = find_op_by_name( data.op );
        Status status = COMP_TREE_OPS_BACKEND[ op_ind ].tr_asm_text( stream, tree_ptr, node,
                                                             counters, context );
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

    PRINT_ASM( "%s %d", commands_list[CMD_PUSH], FRAME_REF_CELL_OFF );
    PRINT_ASM( "%s " REG_FRAME_REF_CELL, commands_list[CMD_POP] );
    PRINT_ASM( "%s main", commands_list[CMD_JMP] );

    Status err = tr_node_asm_text( stream, tree_ptr, tree_get_root( tree_ptr ),
                                   &counters, &context );
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
