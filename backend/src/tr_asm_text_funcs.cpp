#include "tr_asm_text_funcs.h"

#include <assert.h>

#include "backend.h"


// DSL

#define WRP( func ) do                                  \
{                                                       \
    Status _stat = func;                                \
    if (_stat != STATUS_OK) return _stat;               \
}while(0)

//REVIEW -
#define UNUSED(var) do{var = var;}while(0)

#define PRINT( format, ... ) do{fprintf(stream, format, ##__VA_ARGS__); putc('\n', stream);}while(0)
#define END_BLOCK() do{ putc('\n', stream); }while(0)

#define TR_LEFT_CHILD_CURR()                                            \
    WRP(tr_node_asm_text( stream, tree_ptr, tree_get_left_child (node), \
                          counters, context, frames ))

#define TR_RIGHT_CHILD_CURR()                                           \
    WRP(tr_node_asm_text( stream, tree_ptr, tree_get_right_child(node), \
                          counters, context, frames ))

#define TR_LEFT_CHILD_OF_NODE( _node )                                      \
    WRP(tr_node_asm_text( stream, tree_ptr, tree_get_left_child (_node),    \
                          counters, context, frames ))

#define TR_RIGHT_CHILD_OF_NODE( _node )                                     \
    WRP(tr_node_asm_text( stream, tree_ptr, tree_get_right_child(_node),    \
                          counters, context, frames ))

#define LEFT_CURR  (tree_get_left_child ( (node) ))
#define RIGHT_CURR (tree_get_right_child( (node) ))

#define LEFT( _node )  (tree_get_left_child ( (_node) ))
#define RIGHT( _node ) (tree_get_right_child( (_node) ))

#define GET_ID( _node )   (get_node_data( (_node) ).id)
#define GET_NUM( _node )  (get_node_data( (_node) ).num)
#define GET_TYPE( _node ) (get_node_data( (_node) ).type)
#define GET_OP( _node )   (get_node_data( (_node) ).op)

#define _CMD_NAME( cmd ) (commands_list[ cmd ])


// helpers
inline Status helper_asm_text_cmp( const char *jmp_type, FORMAL_TR_ASM_TEXT_ARGS )
{
    TR_LEFT_CHILD_CURR();
    TR_RIGHT_CHILD_CURR();

    PRINT( "%s cmp_yes_" SPECF_CNT_T , jmp_type, counters->cmp_c );
    PRINT( "%s 0", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s cmp_end_" SPECF_CNT_T, _CMD_NAME(CMD_JMP), counters->cmp_c );
    PRINT( "cmp_yes_" SPECF_CNT_T ":", counters->cmp_c );
    PRINT( "%s 1", _CMD_NAME(CMD_PUSH) );
    PRINT( "cmp_end_" SPECF_CNT_T ":", counters->cmp_c );
    END_BLOCK();

    counters->cmp_c++;

    return STATUS_OK;
}

Status tr_asm_text_seq_exec( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    TR_LEFT_CHILD_CURR();
    TR_RIGHT_CHILD_CURR();

    return STATUS_OK;
}


Status tr_asm_text_dummy( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    // REVIEW - ?
    UNUSED(counters);
    UNUSED(context);
    UNUSED(frames);

    PRINT( "; dummy" );
    END_BLOCK();

    return STATUS_OK;
}

//! @brief Determines the maximum value among all identifiers,
//! found in the given subtree, except for those, which are under TREE_OP_RETURN
//! nodes. In this way it avoids functions' identifiers and counts only variables' ones.
inline size_t count_max_var_id_in_func( TreeNode *node )
{
    if ( GET_TYPE(node) != TREE_NODE_TYPE_OP || GET_OP(node) != TREE_OP_CALL_FUNC )
    {
        if ( GET_TYPE(node) == TREE_NODE_TYPE_ID )
            return GET_ID(node);

        size_t res_left = 0, res_right = 0;
        if ( LEFT_CURR )
            res_left = count_max_var_id_in_func( LEFT_CURR );

        if ( RIGHT_CURR )
            res_right = count_max_var_id_in_func( RIGHT_CURR );

        return (res_left > res_right ? res_left : res_right);
    }

    return 0;
}

Status tr_asm_text_func_def( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    context->in_func = 1;

    PRINT( "func_%d:", GET_ID( LEFT_CURR ) );
    PRINT( "%s rbx", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s 2", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s", _CMD_NAME(CMD_ADD) );
    PRINT( "%s rdx", _CMD_NAME(CMD_POP) );
    PRINT( "%s [rdx]", _CMD_NAME(CMD_POP) );
    END_BLOCK();

    id_t func_id = GET_ID(LEFT_CURR);
    REALLOC_ARR_WRP( frames->list, size_t );
    frames->list[func_id] = count_max_var_id_in_func( RIGHT_CURR ) + 3;
    frames->list_curr_len++;

    TR_RIGHT_CHILD_OF_NODE( RIGHT_CURR );
    END_BLOCK();

    context->in_func = 0;

    return STATUS_OK;
}

Status tr_asm_text_main_prog( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    PRINT("main:");

    TR_RIGHT_CHILD_CURR();

    return STATUS_OK;
}


Status tr_asm_text_assign( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    TR_LEFT_CHILD_CURR();

    if ( context->in_func )
    {
        PRINT( "%s rbx", _CMD_NAME(CMD_PUSH) );
        PRINT( "%s %d", _CMD_NAME(CMD_PUSH), GET_ID( RIGHT_CURR ) );
        PRINT( "%s", _CMD_NAME(CMD_SUB) );
        PRINT( "%s rdx", _CMD_NAME(CMD_POP) );
        PRINT( "%s [rdx]", _CMD_NAME(CMD_POP) );
    }
    else
    {
        PRINT( "%s [%d]", _CMD_NAME(CMD_POP), GET_ID( RIGHT_CURR ) );
    }
    END_BLOCK();

    return STATUS_OK;
}

Status tr_asm_text_cmp_more( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    return helper_asm_text_cmp( _CMD_NAME(CMD_JA), stream, tree_ptr, node,
                                counters, context, frames );
}

Status tr_asm_text_cmp_less( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    return helper_asm_text_cmp( _CMD_NAME(CMD_JB), stream, tree_ptr, node,
                                counters, context, frames );
}

Status tr_asm_text_cmp_more_eq( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    return helper_asm_text_cmp( _CMD_NAME(CMD_JAE), stream, tree_ptr, node,
                                counters, context, frames );
}

Status tr_asm_text_cmp_less_eq( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    return helper_asm_text_cmp( _CMD_NAME(CMD_JBE), stream, tree_ptr, node,
                                counters, context, frames );
}

Status tr_asm_text_cmp_equal( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    return helper_asm_text_cmp( _CMD_NAME(CMD_JE), stream, tree_ptr, node,
                                counters, context, frames );
}

Status tr_asm_text_cmp_not_eq( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    return helper_asm_text_cmp( _CMD_NAME(CMD_JNE), stream, tree_ptr, node,
                                counters, context, frames );
}


Status tr_asm_text_add( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    TR_LEFT_CHILD_CURR();
    TR_RIGHT_CHILD_CURR();

    PRINT( "%s", _CMD_NAME(CMD_ADD) );
    END_BLOCK();

    return STATUS_OK;
}

Status tr_asm_text_sub( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    TR_LEFT_CHILD_CURR();
    TR_RIGHT_CHILD_CURR();

    PRINT( "%s", _CMD_NAME(CMD_SUB) );
    END_BLOCK();

    return STATUS_OK;
}

Status tr_asm_text_mul( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    TR_LEFT_CHILD_CURR();
    TR_RIGHT_CHILD_CURR();

    PRINT( "%s", _CMD_NAME(CMD_MUL) );
    END_BLOCK();

    return STATUS_OK;
}

Status tr_asm_text_div( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    TR_LEFT_CHILD_CURR();
    TR_RIGHT_CHILD_CURR();

    PRINT( "%s", _CMD_NAME(CMD_DIV) );
    END_BLOCK();

    return STATUS_OK;
}


Status tr_asm_text_if( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    TR_LEFT_CHILD_CURR();

    cnt_t curr_cnt = counters->if_c;
    counters->if_c++;

    PRINT( "%s 0", _CMD_NAME(CMD_PUSH) );

    if ( GET_TYPE( RIGHT_CURR ) == TREE_NODE_TYPE_OP && GET_OP( RIGHT_CURR ) == TREE_OP_ELSE )
    {
        PRINT( "%s if_else_" SPECF_CNT_T, _CMD_NAME(CMD_JE), curr_cnt );
        TR_LEFT_CHILD_OF_NODE( RIGHT_CURR );
        PRINT( "%s if_end_" SPECF_CNT_T, _CMD_NAME(CMD_JMP), curr_cnt );
        PRINT( "if_else_" SPECF_CNT_T ":", curr_cnt );
        TR_RIGHT_CHILD_OF_NODE( RIGHT_CURR );
        PRINT( "if_end_" SPECF_CNT_T ":", curr_cnt );
        END_BLOCK();
    }
    else
    {
        PRINT( "%s if_end_" SPECF_CNT_T, _CMD_NAME(CMD_JE), curr_cnt );
        TR_RIGHT_CHILD_CURR();
        PRINT( "if_end_" SPECF_CNT_T ":", curr_cnt );
        END_BLOCK();
    }

    return STATUS_OK;
}

Status tr_asm_text_while( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    context->in_while = 1;

    PRINT( "while_" SPECF_CNT_T ":", counters->while_c);

    TR_LEFT_CHILD_CURR();

    PRINT( "%s 0", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s while_end_" SPECF_CNT_T, _CMD_NAME(CMD_JE), counters->while_c  );

    TR_RIGHT_CHILD_CURR();

    PRINT( "%s while_" SPECF_CNT_T, _CMD_NAME(CMD_JMP), counters->while_c );
    PRINT( "while_end_" SPECF_CNT_T ":", counters->while_c );
    END_BLOCK();

    counters->while_c++;

    context->in_while = 0;

    return STATUS_OK;
}

Status tr_asm_text_and( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    TR_LEFT_CHILD_CURR();

    PRINT( "%s 0", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s and_false_" SPECF_CNT_T, _CMD_NAME(CMD_JE), counters->and_c );

    TR_RIGHT_CHILD_CURR();

    PRINT( "%s 0", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s and_false_" SPECF_CNT_T, _CMD_NAME(CMD_JE), counters->and_c );
    PRINT( "%s 1", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s and_end_" SPECF_CNT_T, _CMD_NAME(CMD_JMP), counters->and_c );
    PRINT( "and_false_" SPECF_CNT_T ":", counters->and_c );
    PRINT( "%s 0", _CMD_NAME(CMD_PUSH) );
    PRINT( "and_end_" SPECF_CNT_T ":", counters->and_c );
    END_BLOCK();

    counters->and_c++;

    return STATUS_OK;
}

Status tr_asm_text_or( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    TR_LEFT_CHILD_CURR();

    PRINT( "%s 1", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s or_true_" SPECF_CNT_T, _CMD_NAME(CMD_JE), counters->or_c );

    TR_RIGHT_CHILD_CURR();

    PRINT( "%s 1", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s or_true_" SPECF_CNT_T, _CMD_NAME(CMD_JE), counters->or_c );
    PRINT( "%s 0", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s or_end_" SPECF_CNT_T, _CMD_NAME(CMD_JMP), counters->or_c );
    PRINT( "or_true_" SPECF_CNT_T ":", counters->or_c );
    PRINT( "%s 1", _CMD_NAME(CMD_PUSH) );
    PRINT( "or_end_" SPECF_CNT_T ":", counters->or_c );
    END_BLOCK();

    counters->or_c++;

    return STATUS_OK;
}

Status tr_asm_text_not( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    TR_RIGHT_CHILD_CURR();

    PRINT( "%s 0", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s not_true_" SPECF_CNT_T, _CMD_NAME(CMD_JE), counters->not_c );
    PRINT( "%s 0", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s not_end_" SPECF_CNT_T, _CMD_NAME(CMD_JMP), counters->not_c );
    PRINT( "not_true_" SPECF_CNT_T ":", counters->not_c );
    PRINT( "%s 1", _CMD_NAME(CMD_PUSH) );
    PRINT( "not_end_" SPECF_CNT_T ":", counters->not_c );
    END_BLOCK();

    counters->not_c++;

    return STATUS_OK;
}


Status tr_asm_text_sin( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    TR_RIGHT_CHILD_CURR();

    PRINT( "%s", _CMD_NAME(CMD_SIN) );
    END_BLOCK();

    return STATUS_OK;
}

Status tr_asm_text_cos( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    TR_RIGHT_CHILD_CURR();

    PRINT( "%s", _CMD_NAME(CMD_COS) );
    END_BLOCK();

    return STATUS_OK;
}

Status tr_asm_text_sqrt( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    TR_RIGHT_CHILD_CURR();

    PRINT( "%s", _CMD_NAME(CMD_SQRT) );
    END_BLOCK();

    return STATUS_OK;
}

Status tr_asm_text_ln( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    TR_RIGHT_CHILD_CURR();

    PRINT( "%s", _CMD_NAME(CMD_LN) );
    END_BLOCK();

    return STATUS_OK;
}

Status tr_asm_text_exp( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    TR_RIGHT_CHILD_CURR();

    PRINT( "%s", _CMD_NAME(CMD_EXP) );
    END_BLOCK();

    return STATUS_OK;
}

Status tr_asm_text_minus( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    TR_RIGHT_CHILD_CURR();
    PRINT( "%s -1", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s", _CMD_NAME(CMD_MUL) );

    return STATUS_OK;
}


Status tr_asm_text_call_func( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    PRINT( "%s rbx", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s rcx", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s 1", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s", _CMD_NAME(CMD_ADD) );
    PRINT( "%s rdx", _CMD_NAME(CMD_POP) );
    PRINT( "%s [rdx]", _CMD_NAME(CMD_POP) );
    END_BLOCK();

    TreeNode *node_list_elem = RIGHT_CURR;
    id_t curr_fact_arg_id = 0;
    while (node_list_elem)
    {
        TR_LEFT_CHILD_OF_NODE( node_list_elem );
        PRINT( "%s rcx", _CMD_NAME(CMD_PUSH) );
        PRINT( "%s %d", _CMD_NAME(CMD_PUSH), curr_fact_arg_id );
        PRINT( "%s", _CMD_NAME(CMD_SUB) );
        PRINT( "%s rdx", _CMD_NAME(CMD_POP) );
        PRINT( "%s [rdx]", _CMD_NAME(CMD_POP) );

        curr_fact_arg_id++;
        node_list_elem = RIGHT(node_list_elem);
    }

    PRINT( "%s rcx", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s rbx", _CMD_NAME(CMD_POP) );
    END_BLOCK();

    PRINT( "%s rbx", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s %d", _CMD_NAME(CMD_PUSH), (int) frames->list[ GET_ID(LEFT_CURR) ] );
    PRINT( "%s", _CMD_NAME(CMD_SUB) );
    PRINT( "%s rcx", _CMD_NAME(CMD_POP) );
    END_BLOCK();

    PRINT( "%s func_%d", _CMD_NAME(CMD_CALL), GET_ID(LEFT_CURR) );
    PRINT( "%s rax", _CMD_NAME(CMD_PUSH) );
    END_BLOCK();

    return STATUS_OK;
}

Status tr_asm_text_return( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    if ( RIGHT_CURR )
    {
        TR_RIGHT_CHILD_CURR();
        PRINT( "%s rax", _CMD_NAME(CMD_POP) );
    }

    PRINT( "%s rbx", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s rcx", _CMD_NAME(CMD_POP) );
    END_BLOCK();

    PRINT( "%s rbx", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s 1", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s", _CMD_NAME(CMD_ADD) );
    PRINT( "%s rdx", _CMD_NAME(CMD_POP) );
    PRINT( "%s [rdx]", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s rbx", _CMD_NAME(CMD_POP) );
    END_BLOCK();

    PRINT( "%s rcx", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s 2", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s", _CMD_NAME(CMD_ADD) );
    PRINT( "%s rdx", _CMD_NAME(CMD_POP) );
    PRINT( "%s [rdx]", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s", _CMD_NAME(CMD_RET) );
    END_BLOCK();

    return STATUS_OK;
}

Status tr_asm_text_input( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    UNUSED(counters);
    UNUSED(context);
    UNUSED(frames);

    PRINT( "%s", _CMD_NAME(CMD_IN) );
    END_BLOCK();

    return STATUS_OK;
}

Status tr_asm_text_print_num( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    TR_RIGHT_CHILD_CURR();
    PRINT( "%s", _CMD_NAME( CMD_PRN ) );
    END_BLOCK();

    return STATUS_OK;
}

Status tr_asm_text_print_char ( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    TR_RIGHT_CHILD_CURR();
    PRINT( "%s", _CMD_NAME(CMD_PRC) );
    END_BLOCK();

    return STATUS_OK;
}
