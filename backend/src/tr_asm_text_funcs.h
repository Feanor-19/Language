#ifndef TR_ASM_TEXT_FUNCS_H
#define TR_ASM_TEXT_FUNCS_H

#include "backend_common.h"

#define FORMAL_TR_ASM_TEXT_ARGS FILE *stream, const Tree *tree_ptr, TreeNode *node, \
                                Counters *counters, Context *context, FuncFrames *frames

Status tr_asm_text_seq_exec     ( FORMAL_TR_ASM_TEXT_ARGS );
Status tr_asm_text_dummy        ( FORMAL_TR_ASM_TEXT_ARGS );
Status tr_asm_text_func_def     ( FORMAL_TR_ASM_TEXT_ARGS );
Status tr_asm_text_main_prog    ( FORMAL_TR_ASM_TEXT_ARGS );

Status tr_asm_text_assign       ( FORMAL_TR_ASM_TEXT_ARGS );
Status tr_asm_text_cmp_more     ( FORMAL_TR_ASM_TEXT_ARGS );
Status tr_asm_text_cmp_less     ( FORMAL_TR_ASM_TEXT_ARGS );
Status tr_asm_text_cmp_more_eq  ( FORMAL_TR_ASM_TEXT_ARGS );
Status tr_asm_text_cmp_less_eq  ( FORMAL_TR_ASM_TEXT_ARGS );
Status tr_asm_text_cmp_equal    ( FORMAL_TR_ASM_TEXT_ARGS );
Status tr_asm_text_cmp_not_eq   ( FORMAL_TR_ASM_TEXT_ARGS );

Status tr_asm_text_add          ( FORMAL_TR_ASM_TEXT_ARGS );
Status tr_asm_text_sub          ( FORMAL_TR_ASM_TEXT_ARGS );
Status tr_asm_text_mul          ( FORMAL_TR_ASM_TEXT_ARGS );
Status tr_asm_text_div          ( FORMAL_TR_ASM_TEXT_ARGS );

Status tr_asm_text_if           ( FORMAL_TR_ASM_TEXT_ARGS );
Status tr_asm_text_while        ( FORMAL_TR_ASM_TEXT_ARGS );
Status tr_asm_text_and          ( FORMAL_TR_ASM_TEXT_ARGS );
Status tr_asm_text_or           ( FORMAL_TR_ASM_TEXT_ARGS );
Status tr_asm_text_not          ( FORMAL_TR_ASM_TEXT_ARGS );

Status tr_asm_text_cos          ( FORMAL_TR_ASM_TEXT_ARGS );
Status tr_asm_text_sin          ( FORMAL_TR_ASM_TEXT_ARGS );
Status tr_asm_text_sqrt         ( FORMAL_TR_ASM_TEXT_ARGS );
Status tr_asm_text_ln           ( FORMAL_TR_ASM_TEXT_ARGS );
Status tr_asm_text_exp          ( FORMAL_TR_ASM_TEXT_ARGS );
Status tr_asm_text_minus        ( FORMAL_TR_ASM_TEXT_ARGS );

Status tr_asm_text_call_func    ( FORMAL_TR_ASM_TEXT_ARGS );
Status tr_asm_text_return       ( FORMAL_TR_ASM_TEXT_ARGS );
Status tr_asm_text_input        ( FORMAL_TR_ASM_TEXT_ARGS );
Status tr_asm_text_print_num    ( FORMAL_TR_ASM_TEXT_ARGS );
Status tr_asm_text_print_char   ( FORMAL_TR_ASM_TEXT_ARGS );

#endif /* TR_ASM_TEXT_FUNCS_H */
