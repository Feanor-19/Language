#include "tr_asm_text_funcs.h"

#include <assert.h>

#include "backend.h"


// DSL

#define WRP( func ) do                                  \
{                                                       \
    Status _stat = func;                                \
    if (_stat != STATUS_OK) return _stat;               \
}while(0)

#define UNUSED(var) do{var = var;}while(0)

#define PRINT( format, ... ) do{fprintf(stream, format, ##__VA_ARGS__); putc('\n', stream);}while(0)
#define END_BLOCK() do{ putc('\n', stream); }while(0)

#define TR_LEFT_CHILD_CURR() \
    WRP(tr_node_asm_text( stream, tree_ptr, tree_get_left_child (node), counters ))

#define TR_RIGHT_CHILD_CURR() \
    WRP(tr_node_asm_text( stream, tree_ptr, tree_get_right_child(node), counters ))

#define TR_LEFT_CHILD_OF_NODE( _node )  \
    WRP(tr_node_asm_text( stream, tree_ptr, tree_get_left_child (_node), counters ))

#define TR_RIGHT_CHILD_OF_NODE( _node ) \
    WRP(tr_node_asm_text( stream, tree_ptr, tree_get_right_child(_node), counters ))

#define LEFT_CURR  (tree_get_left_child ( (node) ))
#define RIGHT_CURR (tree_get_right_child( (node) ))

#define LEFT( _node )  (tree_get_left_child ( (_node) ))
#define RIGHT( _node ) (tree_get_right_child( (_node) ))

#define GET_ID( _node )  (get_node_data( (_node) ).id)
#define GET_NUM( _node ) (get_node_data( (_node) ).num)

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

    UNUSED(counters);

    PRINT( "; dummy" );
    END_BLOCK();

    return STATUS_OK;
}

Status tr_asm_text_assign( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    TR_LEFT_CHILD_CURR();

    PRINT( "%s [%d]", _CMD_NAME(CMD_POP), GET_ID( RIGHT_CURR ) );
    END_BLOCK();

    return STATUS_OK;
}

Status tr_asm_text_cmp_more( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    return helper_asm_text_cmp( _CMD_NAME(CMD_JA), stream, tree_ptr, node, counters );
}

Status tr_asm_text_cmp_less( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    return helper_asm_text_cmp( _CMD_NAME(CMD_JB), stream, tree_ptr, node, counters );
}

Status tr_asm_text_cmp_more_eq( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    return helper_asm_text_cmp( _CMD_NAME(CMD_JAE), stream, tree_ptr, node, counters );
}

Status tr_asm_text_cmp_less_eq( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    return helper_asm_text_cmp( _CMD_NAME(CMD_JBE), stream, tree_ptr, node, counters );
}

Status tr_asm_text_cmp_equal( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    return helper_asm_text_cmp( _CMD_NAME(CMD_JE), stream, tree_ptr, node, counters );
}

Status tr_asm_text_cmp_not_eq( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    return helper_asm_text_cmp( _CMD_NAME(CMD_JNE), stream, tree_ptr, node, counters );
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

    PRINT( "%s 0", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s if_end_" SPECF_CNT_T, _CMD_NAME(CMD_JE), counters->if_c );

    TR_RIGHT_CHILD_CURR();

    PRINT( "if_end_" SPECF_CNT_T ":", counters->if_c );

    END_BLOCK();

    counters->if_c++;

    return STATUS_OK;
}

Status tr_asm_text_while( FORMAL_TR_ASM_TEXT_ARGS )
{
    assert( stream );
    assert( tree_ptr );
    assert( node );

    PRINT( "while_" SPECF_CNT_T ":", counters->while_c);

    TR_LEFT_CHILD_CURR();

    PRINT( "%s 0", _CMD_NAME(CMD_PUSH) );
    PRINT( "%s while_end_" SPECF_CNT_T, _CMD_NAME(CMD_JE), counters->while_c  );

    TR_RIGHT_CHILD_CURR();

    PRINT( "%s while_" SPECF_CNT_T, _CMD_NAME(CMD_JMP), counters->while_c );
    PRINT( "while_end_" SPECF_CNT_T ":", counters->while_c );
    END_BLOCK();

    counters->while_c++;

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

