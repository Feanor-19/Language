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
                          counters, context ))

#define TR_RIGHT_CHILD_CURR()                                           \
    WRP(tr_node_asm_text( stream, tree_ptr, tree_get_right_child(node), \
                          counters, context ))

#define TR_LEFT_CHILD_OF_NODE( _node )                                      \
    WRP(tr_node_asm_text( stream, tree_ptr, tree_get_left_child (_node),    \
                          counters, context ))

#define TR_RIGHT_CHILD_OF_NODE( _node )                                     \
    WRP(tr_node_asm_text( stream, tree_ptr, tree_get_right_child(_node),    \
                          counters, context ))

#define LEFT_CURR  (tree_get_left_child ( (node) ))
#define RIGHT_CURR (tree_get_right_child( (node) ))

#define LEFT( _node )  (tree_get_left_child ( (_node) ))
#define RIGHT( _node ) (tree_get_right_child( (_node) ))

#define GET_ID( _node )   (get_node_data( (_node) ).id)
#define GET_NUM( _node )  (get_node_data( (_node) ).num)
#define GET_TYPE( _node ) (get_node_data( (_node) ).type)
#define GET_OP( _node )   (get_node_data( (_node) ).op)

#define _CMD_NAME( cmd ) (commands_list[ cmd ])

#define PRINT_FUNC_NAME() PRINT( "; %s", __func__ )
#define PRINT_FUNC_NAME_END() PRINT( "; %s END\n", __func__ )

// helpers
inline Status helper_asm_text_cmp( const char *jmp_type, FORMAL_TR_ASM_TEXT_ARGS )
{
    assert(jmp_type);
    PRINT_FUNC_NAME();

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

    PRINT_FUNC_NAME_END();
    return STATUS_OK;
}

Status tr_asm_text_seq_exec( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

    TR_LEFT_CHILD_CURR();
    TR_RIGHT_CHILD_CURR();

    PRINT_FUNC_NAME_END();
    return STATUS_OK;
}


Status tr_asm_text_dummy( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

    // REVIEW - ?
    UNUSED(counters);
    UNUSED(context);

    PRINT( "; dummy" );
    END_BLOCK();

    PRINT_FUNC_NAME_END();
    return STATUS_OK;
}

Status tr_asm_text_func_def( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

    PRINT( "func_%d:", GET_ID( LEFT_CURR ) );
    PRINT( "%s " REG_FRAME_REF_CELL, _CMD_NAME(CMD_PUSH) );
    PRINT( "%s 2", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s", _CMD_NAME(CMD_SUB) );
    PRINT( "%s " REG_COMP, _CMD_NAME(CMD_POP) );
    PRINT( "%s [" REG_COMP "]", _CMD_NAME(CMD_POP) );
    END_BLOCK();

    context->curr_func_frame_size = 3;

    TR_RIGHT_CHILD_OF_NODE( RIGHT_CURR );
    END_BLOCK();

    context->curr_func_frame_size = 0;

    PRINT_FUNC_NAME_END();
    return STATUS_OK;
}

Status tr_asm_text_main_prog( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

    PRINT("main:");

    context->curr_func_frame_size = 3;
    TR_RIGHT_CHILD_CURR();
    context->curr_func_frame_size = 0;

    PRINT_FUNC_NAME_END();
    return STATUS_OK;
}


Status tr_asm_text_assign( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

    TR_LEFT_CHILD_CURR();

    id_t var_id = GET_ID( RIGHT_CURR );
    if ( var_id + 3 > context->curr_func_frame_size )
        context->curr_func_frame_size = var_id + 3;

    PRINT( "%s " REG_FRAME_REF_CELL, _CMD_NAME(CMD_PUSH) );
    PRINT( "%s %d", _CMD_NAME(CMD_PUSH), var_id );
    PRINT( "%s", _CMD_NAME(CMD_ADD) );
    PRINT( "%s " REG_COMP, _CMD_NAME(CMD_POP) );
    PRINT( "%s [" REG_COMP "]", _CMD_NAME(CMD_POP) );
    END_BLOCK();

    PRINT_FUNC_NAME_END();
    return STATUS_OK;
}

Status tr_asm_text_cmp_more( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

    return helper_asm_text_cmp( _CMD_NAME(CMD_JA), stream, tree_ptr, node,
                                counters, context );
}

Status tr_asm_text_cmp_less( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

    return helper_asm_text_cmp( _CMD_NAME(CMD_JB), stream, tree_ptr, node,
                                counters, context );
}

Status tr_asm_text_cmp_more_eq( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

    return helper_asm_text_cmp( _CMD_NAME(CMD_JAE), stream, tree_ptr, node,
                                counters, context );
}

Status tr_asm_text_cmp_less_eq( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

    return helper_asm_text_cmp( _CMD_NAME(CMD_JBE), stream, tree_ptr, node,
                                counters, context );
}

Status tr_asm_text_cmp_equal( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

    return helper_asm_text_cmp( _CMD_NAME(CMD_JE), stream, tree_ptr, node,
                                counters, context );
}

Status tr_asm_text_cmp_not_eq( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

    return helper_asm_text_cmp( _CMD_NAME(CMD_JNE), stream, tree_ptr, node,
                                counters, context );
}


Status tr_asm_text_add( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

    TR_LEFT_CHILD_CURR();
    TR_RIGHT_CHILD_CURR();

    PRINT( "%s", _CMD_NAME(CMD_ADD) );
    END_BLOCK();

    PRINT_FUNC_NAME_END();
    return STATUS_OK;
}

Status tr_asm_text_sub( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

    TR_LEFT_CHILD_CURR();
    TR_RIGHT_CHILD_CURR();

    PRINT( "%s", _CMD_NAME(CMD_SUB) );
    END_BLOCK();

    PRINT_FUNC_NAME_END();
    return STATUS_OK;
}

Status tr_asm_text_mul( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

    TR_LEFT_CHILD_CURR();
    TR_RIGHT_CHILD_CURR();

    PRINT( "%s", _CMD_NAME(CMD_MUL) );
    END_BLOCK();

    PRINT_FUNC_NAME_END();
    return STATUS_OK;
}

Status tr_asm_text_div( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

    TR_LEFT_CHILD_CURR();
    TR_RIGHT_CHILD_CURR();

    PRINT( "%s", _CMD_NAME(CMD_DIV) );
    END_BLOCK();

    PRINT_FUNC_NAME_END();
    return STATUS_OK;
}


Status tr_asm_text_if( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

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

    PRINT_FUNC_NAME_END();
    return STATUS_OK;
}

Status tr_asm_text_while( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

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

    PRINT_FUNC_NAME_END();
    return STATUS_OK;
}

Status tr_asm_text_and( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

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

    PRINT_FUNC_NAME_END();
    return STATUS_OK;
}

Status tr_asm_text_or( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

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

    PRINT_FUNC_NAME_END();
    return STATUS_OK;
}

Status tr_asm_text_not( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

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

    PRINT_FUNC_NAME_END();
    return STATUS_OK;
}


Status tr_asm_text_sin( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

    TR_RIGHT_CHILD_CURR();

    PRINT( "%s", _CMD_NAME(CMD_SIN) );
    END_BLOCK();

    PRINT_FUNC_NAME_END();
    return STATUS_OK;
}

Status tr_asm_text_cos( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

    TR_RIGHT_CHILD_CURR();

    PRINT( "%s", _CMD_NAME(CMD_COS) );
    END_BLOCK();

    PRINT_FUNC_NAME_END();
    return STATUS_OK;
}

Status tr_asm_text_sqrt( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

    TR_RIGHT_CHILD_CURR();

    PRINT( "%s", _CMD_NAME(CMD_SQRT) );
    END_BLOCK();

    PRINT_FUNC_NAME_END();
    return STATUS_OK;
}

Status tr_asm_text_ln( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

    TR_RIGHT_CHILD_CURR();

    PRINT( "%s", _CMD_NAME(CMD_LN) );
    END_BLOCK();

    PRINT_FUNC_NAME_END();
    return STATUS_OK;
}

Status tr_asm_text_exp( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

    TR_RIGHT_CHILD_CURR();

    PRINT( "%s", _CMD_NAME(CMD_EXP) );
    END_BLOCK();

    PRINT_FUNC_NAME_END();
    return STATUS_OK;
}

Status tr_asm_text_minus( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

    TR_RIGHT_CHILD_CURR();
    PRINT( "%s -1", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s", _CMD_NAME(CMD_MUL) );

    PRINT_FUNC_NAME_END();
    return STATUS_OK;
}


Status tr_asm_text_call_func( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

    PRINT( "; storing old reg_frame_ref value in the being created frame" );
    PRINT( "%s " REG_FRAME_REF_CELL, _CMD_NAME(CMD_PUSH) );
    PRINT( "%s %d ; func_frame_size - 1", _CMD_NAME(CMD_PUSH), context->curr_func_frame_size - 1 );
    PRINT( "%s", _CMD_NAME(CMD_ADD) );
    PRINT( "%s " REG_COMP, _CMD_NAME(CMD_POP) );
    PRINT( "%s " REG_FRAME_REF_CELL, _CMD_NAME(CMD_PUSH) );
    PRINT( "%s [" REG_COMP "]", _CMD_NAME(CMD_POP) );
    END_BLOCK();

    PRINT( "; storing new reg_frame_ref into reg_tmp" );
    PRINT( "%s " REG_COMP, _CMD_NAME(CMD_PUSH) );
    PRINT( "%s 1", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s", _CMD_NAME(CMD_ADD) );
    PRINT( "%s " REG_TMP, _CMD_NAME(CMD_POP) );
    END_BLOCK();

    PRINT( "; passing fact args (if there are any)" );
    TreeNode *node_list_elem = RIGHT_CURR;
    id_t curr_fact_arg_id = 0;
    while (node_list_elem)
    {
        TR_LEFT_CHILD_OF_NODE( node_list_elem );
        PRINT( "%s " REG_TMP, _CMD_NAME(CMD_PUSH) );
        PRINT( "%s %d ; curr_fact_arg_id", _CMD_NAME(CMD_PUSH), curr_fact_arg_id );
        PRINT( "%s", _CMD_NAME(CMD_ADD) );
        PRINT( "%s " REG_COMP, _CMD_NAME(CMD_POP) );
        PRINT( "%s [" REG_COMP "]", _CMD_NAME(CMD_POP) );

        curr_fact_arg_id++;
        node_list_elem = RIGHT(node_list_elem);
    }

    PRINT( "; updating reg_frame_ref from reg_tmp to end the process of creating new frame" );
    PRINT( "%s " REG_TMP, _CMD_NAME(CMD_PUSH));
    PRINT( "%s " REG_FRAME_REF_CELL, _CMD_NAME(CMD_POP));

    PRINT( "%s func_%d", _CMD_NAME(CMD_CALL), GET_ID(LEFT_CURR) );
    PRINT( "%s rax", _CMD_NAME(CMD_PUSH) );
    END_BLOCK();

    PRINT_FUNC_NAME_END();
    return STATUS_OK;
}

Status tr_asm_text_return( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

    if ( RIGHT_CURR )
    {
        PRINT( "; computing ret value" );
        TR_RIGHT_CHILD_CURR();
        PRINT( "%s rax", _CMD_NAME(CMD_POP) );
    }

    // push [reg_fr_ref - 2] (pushing ret address)
    PRINT( "; push [reg_fr_ref - 2] (pushing ret address)" );
    PRINT( "%s " REG_FRAME_REF_CELL, _CMD_NAME(CMD_PUSH) );
    PRINT( "%s 2", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s", _CMD_NAME(CMD_SUB) );
    PRINT( "%s " REG_COMP, _CMD_NAME(CMD_POP) );
    PRINT( "%s [" REG_COMP "]", _CMD_NAME(CMD_PUSH) );
    END_BLOCK();

    // reg_fr_ref = mem[reg_fr_ref - 1] (updating reg_fr_ref)
    PRINT( "; reg_fr_ref = mem[reg_fr_ref - 1] (updating reg_fr_ref)" );
    PRINT( "%s " REG_FRAME_REF_CELL, _CMD_NAME(CMD_PUSH) );
    PRINT( "%s 1", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s", _CMD_NAME(CMD_SUB) );
    PRINT( "%s " REG_COMP, _CMD_NAME(CMD_POP) );
    PRINT( "%s [" REG_COMP "]", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s " REG_FRAME_REF_CELL, _CMD_NAME(CMD_POP) );
    END_BLOCK();

    PRINT( "%s", _CMD_NAME(CMD_RET) );
    END_BLOCK();

    PRINT_FUNC_NAME_END();
    return STATUS_OK;
}

Status tr_asm_text_input( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

    UNUSED(counters);
    UNUSED(context);

    PRINT( "%s", _CMD_NAME(CMD_IN) );
    END_BLOCK();

    PRINT_FUNC_NAME_END();
    return STATUS_OK;
}

Status tr_asm_text_print_num( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

    TR_RIGHT_CHILD_CURR();
    PRINT( "%s", _CMD_NAME( CMD_PRN ) );
    END_BLOCK();

    PRINT_FUNC_NAME_END();
    return STATUS_OK;
}

Status tr_asm_text_print_char ( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );
    PRINT_FUNC_NAME();

    TR_RIGHT_CHILD_CURR();
    PRINT( "%s", _CMD_NAME(CMD_PRC) );
    END_BLOCK();

    PRINT_FUNC_NAME_END();
    return STATUS_OK;
}
