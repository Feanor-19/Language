#include "frontend.h"

#include <ctype.h>
#include <math.h>
#include "compiler_tree_dump.h"

// TODO - В ГРАММАТИКЕ Вынести "Cond" ?CmpOp (group)? Expr "Than" Expr
// в отдельное правило, а потом эти правила разрешить соединять с помощью NOT, AND
// и OR (приоритет разный, придется всего три правила делать)

// TODO - ИДЕЯ: добавление в грамматику умножения числа на переменную или выражение из
// переменных, а именно: Unr выражается через XXX, XXX ::= (Num "UnitsOf" Primal) | Primal

//REVIEW -
#define UNUSED(var) do{var = var;}while(0)
#define CURR (*curr_ptr)
#define FACT_REC_FALL_ARGS comp_prog, prog, curr_ptr, context
#define FORMAL_REC_FALL_ARGS \
    CompiledProgram *comp_prog, const char *prog, const char **curr_ptr, Context *context
#define TREE &comp_prog->tree
#define NT_GLOBAL comp_prog->nametables.global_vars
#define NT_FUNCS comp_prog->nametables.funcs
#define NT_FUNC_VARS comp_prog->nametables.func_vars
#define MOVE_CURR_TO_END_OF_TOKEN( token ) (CURR = token.start + token.len)



static TreeNode *get_num               ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_if                ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_while             ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_primal            ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_unr               ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_mulive            ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_expr              ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_var               ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_assign            ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_var_death         ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_var_birth         ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_call_func_action  ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_call_func_recipe  ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_return            ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_op                ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_operators         ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_fact_args         ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_formal_args       ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_func_recipe       ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_func_action       ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_func_def          ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_func_defs         ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_prog              ( FORMAL_REC_FALL_ARGS );



//! @brief Returns index of given 'ident' in the 'nametable', if found;
//! otherwise returns ABSENT_ID;
inline id_t find_ident_in_nametable( Nametable nametable, Identificator ident )
{
    for (id_t ind = 0; ind < (id_t) nametable.list_curr_len; ind++)
    {
        if ( cmp_idents( ident, nametable.list[ind].ident ) )
            return ind;
    }

    return ABSENT_ID;
}

//! @brief States what 'empty identificator' means.
inline int is_ident_empty( Identificator ident )
{
    return ident.len == 0;
}

//! @brief Adds given 'ident' into given 'nametable', placing it in
//! the free place with the least index. If needed, does reallocation.
//! @return index where 'ident' was placed.
inline id_t add_ident_into_nametable( Nametable *nametable, Identificator ident )
{
    assert(nametable);

    for (id_t ind = 0; ind < (id_t) nametable->list_curr_len; ind++)
    {
        if ( is_ident_empty(nametable->list[ind].ident)  )
        {
            nametable->list[ind].ident = ident;
            return ind;
        }
    }

    REALLOC_ARR_WRP( nametable->list, NametableElem );
    nametable->list[nametable->list_curr_len++].ident = ident;
    return (id_t) nametable->list_curr_len - 1;
}

//! @brief Founds given 'ident' in the 'nametable' and deletes it.
//! (replaces it with 'empty identificator'). If this identificator isn't found,
//! returns 0, otherwise (no errors) returns 1.
inline int del_ident_from_nametable( Nametable *nametable, Identificator ident )
{
    assert(nametable);

    for (size_t ind = 0; ind < nametable->list_curr_len; ind++)
    {
        if ( cmp_idents( nametable->list[ind].ident, ident ) )
        {
            nametable->list[ind].ident.start = NULL;
            nametable->list[ind].ident.len = 0;
            return 1;
        }
    }

    return 0;
}

//! @brief Returns 1 if ident is not found in any of the comp_prog's nametables, 0 otherwise.
inline int check_is_ident_fresh( CompiledProgram *comp_prog, Identificator ident, Context *context )
{
    if ( context->in_func_action || context->in_func_recipe )
    {
        return find_ident_in_nametable( NT_FUNCS, ident )     == ABSENT_ID
            && find_ident_in_nametable( NT_FUNC_VARS, ident ) == ABSENT_ID;
    }
    else
    {
        return find_ident_in_nametable( NT_FUNCS, ident )       == ABSENT_ID
            && find_ident_in_nametable( NT_GLOBAL, ident ) == ABSENT_ID;
    }
}

//! @brief Counts number of nodes connected as a list using TREE_OP_LIST_CONNECTOR.
//! Receives the root of the list.
inline size_t count_list_len( TreeNode *list_root )
{
    assert(list_root);

    size_t num = 1;
    TreeNode *curr = list_root;
    while ( tree_get_right_child( curr ) )
    {
        num++;
        curr = tree_get_right_child( curr );
    }

    return num;
}

static TreeNode *get_num( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    UNUSED(context);

    TreeNode *node_num = NULL;
    Token tkn_input = get_token( CURR );
    if ( is_tkn_keyword( tkn_input, KW_InputOp ) )
    {
        MOVE_CURR_TO_END_OF_TOKEN( tkn_input );

        node_num = new_node_op( TREE, TREE_OP_INPUT );
    }
    else
    {
        Token tkn_num = get_token( CURR );
        SYN_ASSERT( tkn_num.type == TKN_TYPE_NUM, prog, CURR, "Number" );
        MOVE_CURR_TO_END_OF_TOKEN( tkn_num );

        node_num = new_node_num( TREE, tkn_num.num );
    }

    SYN_ASSERT( node_num, prog, CURR, "Input operator or a number" );

    return node_num;
}

//! @brief Checks is 'tkn' of type 'Keyword' and belongs to
//! group 'CmpOp'. If yes, returns corresponding CompTreeOpName,
//! otherwise returns TREE_OP_DUMMY.
inline CompTreeOpName translate_kw_cmp_op( Token tkn )
{

// Only keywords from the group 'CmpOp' must be held in
// in this switch, so the warning "-Wswitch-enum" must be ignored.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
    switch (tkn.keyword)
    {
    case KW_CmpOp_equal:
        return TREE_OP_CMP_EQUAL;
        break;
    case KW_CmpOp_less:
        return TREE_OP_CMP_LESS;
        break;
    case KW_CmpOp_lessOrEqual:
        return TREE_OP_CMP_LESS_EQ;
        break;
    case KW_CmpOp_more:
        return TREE_OP_CMP_MORE;
        break;
    case KW_CmpOp_moreOrEqual:
        return TREE_OP_CMP_MORE_EQ;
        break;
    case KW_CmpOp_notEqual:
        return TREE_OP_CMP_NOT_EQ;
        break;
    default:
        return TREE_OP_DUMMY;
        break;
    }
#pragma GCC diagnostic pop
}

static TreeNode *get_if( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    Token tkn_if1 = get_token( CURR );
    if ( !is_tkn_keyword( tkn_if1, KW_If1 ) )
        return NULL;
    MOVE_CURR_TO_END_OF_TOKEN(tkn_if1);

    TreeNode *node_if = new_node_op( TREE, TREE_OP_IF );

    Token tkn_cond = get_token( CURR );
    SYN_ASSERT( is_tkn_keyword( tkn_cond, KW_Cond ), prog, CURR, "Keyword Cond" );
    MOVE_CURR_TO_END_OF_TOKEN(tkn_cond);

    Token tkn_cmp_op = get_token( CURR );
    SYN_ASSERT( translate_kw_cmp_op( tkn_cmp_op ) != TREE_OP_DUMMY, prog, CURR, "Comparing operator" );
    MOVE_CURR_TO_END_OF_TOKEN(tkn_cmp_op);

    TreeNode *node_cmp_op = new_node_op( TREE, translate_kw_cmp_op(tkn_cmp_op) );

    tree_hang_loose_node_at_left( TREE, node_cmp_op, node_if );

    TreeNode *node_expr_left = get_expr(FACT_REC_FALL_ARGS);
    SYN_ASSERT( node_expr_left, prog, CURR, "Expression");
    tree_hang_loose_node_at_left( TREE, node_expr_left, node_cmp_op );

    Token tkn_cmp_with = get_token( CURR );
    SYN_ASSERT( is_tkn_keyword( tkn_cmp_with, KW_CmpWith ), prog, CURR, KEYWORDS[KW_CmpWith].str );
    MOVE_CURR_TO_END_OF_TOKEN(tkn_cmp_with);

    TreeNode *node_expr_right = get_expr(FACT_REC_FALL_ARGS);
    SYN_ASSERT( node_expr_right, prog, CURR, "Expression");
    tree_hang_loose_node_at_right( TREE, node_expr_right, node_cmp_op );

    Token tkn_if2 = get_token( CURR );
    SYN_ASSERT( is_tkn_keyword(tkn_if2, KW_If2), prog, CURR, KEYWORDS[KW_If2].str );
    MOVE_CURR_TO_END_OF_TOKEN(tkn_if2);

    TreeNode *node_operators = get_operators( FACT_REC_FALL_ARGS );
    SYN_ASSERT( node_operators, prog, CURR, "Operators" );
    tree_hang_loose_node_at_right( TREE, node_operators, node_if );

    Token tkn_if_end = get_token( CURR );
    SYN_ASSERT( is_tkn_keyword( tkn_if_end, KW_IfEnd ), prog, CURR, KEYWORDS[KW_IfEnd].str )
    MOVE_CURR_TO_END_OF_TOKEN(tkn_if_end);

    return node_if;
}

static TreeNode *get_while( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    Token tkn_while1 = get_token( CURR );
    if ( !is_tkn_keyword( tkn_while1, KW_While1 ) )
        return NULL;
    MOVE_CURR_TO_END_OF_TOKEN(tkn_while1);

    context->in_while = 1;

    TreeNode *node_while = new_node_op( TREE, TREE_OP_WHILE );

    Token tkn_cond = get_token( CURR );
    SYN_ASSERT( is_tkn_keyword( tkn_cond, KW_Cond ), prog, CURR, "Keyword Cond" );
    MOVE_CURR_TO_END_OF_TOKEN(tkn_cond);

    Token tkn_cmp_op = get_token( CURR );
    SYN_ASSERT( translate_kw_cmp_op( tkn_cmp_op ) != TREE_OP_DUMMY, prog, CURR, "Comparing operator" );
    MOVE_CURR_TO_END_OF_TOKEN(tkn_cmp_op);

    TreeNode *node_cmp_op = new_node_op( TREE, translate_kw_cmp_op(tkn_cmp_op) );

    tree_hang_loose_node_at_left( TREE, node_cmp_op, node_while );

    TreeNode *node_expr_left = get_expr(FACT_REC_FALL_ARGS);
    SYN_ASSERT( node_expr_left, prog, CURR, "Expression");
    tree_hang_loose_node_at_left( TREE, node_expr_left, node_cmp_op );

    Token tkn_cmp_with = get_token( CURR );
    SYN_ASSERT( is_tkn_keyword( tkn_cmp_with, KW_CmpWith ), prog, CURR, KEYWORDS[KW_CmpWith].str );
    MOVE_CURR_TO_END_OF_TOKEN(tkn_cmp_with);

    TreeNode *node_expr_right = get_expr(FACT_REC_FALL_ARGS);
    SYN_ASSERT( node_expr_right, prog, CURR, "Expression");
    tree_hang_loose_node_at_right( TREE, node_expr_right, node_cmp_op );

    Token tkn_while2 = get_token( CURR );
    SYN_ASSERT( is_tkn_keyword(tkn_while2, KW_While2), prog, CURR, KEYWORDS[KW_While2].str );
    MOVE_CURR_TO_END_OF_TOKEN(tkn_while2);

    TreeNode *node_operators = get_operators( FACT_REC_FALL_ARGS );
    SYN_ASSERT( node_operators, prog, CURR, "Operators" );
    tree_hang_loose_node_at_right( TREE, node_operators, node_while );

    Token tkn_while_end = get_token( CURR );
    SYN_ASSERT( is_tkn_keyword( tkn_while_end, KW_WhileEnd ), prog, CURR, KEYWORDS[KW_WhileEnd].str )
    MOVE_CURR_TO_END_OF_TOKEN(tkn_while_end);

    context->in_while = 0;

    return node_while;
}

static TreeNode *get_primal( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    Token tkn = get_token( CURR );
    TreeNode *node_primal = NULL;
    if ( is_tkn_keyword( tkn, KW_InBracketsStart ) )
    {
        MOVE_CURR_TO_END_OF_TOKEN(tkn);

        TreeNode *node_expr = get_expr(FACT_REC_FALL_ARGS);
        SYN_ASSERT(node_expr, prog, CURR, "Expression");

        tkn = get_token( CURR );
        SYN_ASSERT( is_tkn_sep_char( tkn, SEP_Semicolon ), prog, CURR, ";" );
        MOVE_CURR_TO_END_OF_TOKEN(tkn);

        node_primal = node_expr;
    }
    else
    {
           ( node_primal = get_call_func_recipe( FACT_REC_FALL_ARGS ) )
        || ( node_primal =              get_var( FACT_REC_FALL_ARGS ) );
    }


    SYN_ASSERT( node_primal, prog, CURR, "BracketOpn, Call of Recipe Function or Variable" );

    return node_primal;
}

//! @brief Checks is 'tkn' of type 'Keyword' and belongs to
//! group 'UnrOp'. If yes, returns corresponding CompTreeOpName,
//! otherwise returns TREE_OP_DUMMY.
inline CompTreeOpName translate_tkn_unr_op( Token tkn )
{
// Only keywords from the group 'CmpOp' must be held in
// in this switch, so the warning "-Wswitch-enum" must be ignored.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
    switch (tkn.keyword)
    {
    case KW_UnrOp_cos:
        return TREE_OP_COS;
        break;
    case KW_UnrOp_exp:
        return TREE_OP_EXP;
        break;
    case KW_UnrOp_ln:
        return TREE_OP_LN;
        break;
    case KW_UnrOp_minus:
        return TREE_OP_MINUS;
        break;
    case KW_UnrOp_sin:
        return TREE_OP_SIN;
        break;
    case KW_UnrOp_sqrt:
        return TREE_OP_SQRT;
        break;
    default:
        return TREE_OP_DUMMY;
        break;
    }
#pragma GCC diagnostic pop
}

static TreeNode *get_unr( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    Token tkn_amp = get_token( CURR );
    Token tkn_op = get_token( tkn_amp.start + tkn_amp.len );
    if ( is_tkn_amp( tkn_amp ) && translate_tkn_unr_op( tkn_op ) != TREE_OP_DUMMY  )
    {
        // TODO - отмечать в списке использованных амлпификаторов

        MOVE_CURR_TO_END_OF_TOKEN(tkn_op);

        TreeNode *node_op = new_node_op( TREE, translate_tkn_unr_op( tkn_op ) );

        TreeNode *node_primal = get_primal( FACT_REC_FALL_ARGS );
        SYN_ASSERT( node_primal, prog, CURR, "Primal" );

        tree_hang_loose_node_at_right( TREE, node_primal, node_op );

        return node_op;
    }
    else
    {
        TreeNode *node_primal = get_primal( FACT_REC_FALL_ARGS );
        SYN_ASSERT( node_primal, prog, CURR, "Primal" );
        return node_primal;
    }
}

static TreeNode *get_mulive( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    TreeNode *node_mulive = get_unr( FACT_REC_FALL_ARGS );
    SYN_ASSERT( node_mulive, prog, CURR, "Expression part: unr" );

    Token tkn_amp = {};
    while ( tkn_amp = get_token(CURR), is_tkn_amp( tkn_amp ) )
    {
        // TODO - отмечать в списке использованных амлпификаторов

        Token tkn_op = get_token( tkn_amp.start + tkn_amp.len ); // not yet sure it is our case
        if ( tkn_op.type != TKN_TYPE_KEYWORD )
            break;
        TreeNode *node_op = NULL;
        if      ( is_tkn_keyword( tkn_op, KW_OpMul ) )
            node_op = new_node_op( TREE, TREE_OP_MUL );
        else if ( is_tkn_keyword( tkn_op, KW_OpDiv ) )
            node_op = new_node_op( TREE, TREE_OP_DIV );
        else
            break;
        MOVE_CURR_TO_END_OF_TOKEN(tkn_op); // now sure it is really our case

        TreeNode *node_new_unr = get_unr( FACT_REC_FALL_ARGS );
        SYN_ASSERT( node_new_unr, prog, CURR, "Unr" );

        tree_hang_loose_node_at_left( TREE, node_mulive, node_op );
        tree_hang_loose_node_at_right( TREE, node_new_unr, node_op );
        node_mulive = node_op;
    }

    return node_mulive;
}

static TreeNode *get_expr( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    TreeNode *node_expr = get_mulive( FACT_REC_FALL_ARGS );
    SYN_ASSERT( node_expr, prog, CURR, "Expression part: mulive" );

    Token tkn_amp = {};
    while ( tkn_amp = get_token(CURR), is_tkn_amp( tkn_amp ) )
    {
        // TODO - отмечать в списке использованных амлпификаторов

        Token tkn_op = get_token( tkn_amp.start + tkn_amp.len ); // not yet sure it is our case
        if ( tkn_op.type != TKN_TYPE_KEYWORD )
            break;
        TreeNode *node_op = NULL;
        if      ( is_tkn_keyword( tkn_op, KW_OpAdd ) )
            node_op = new_node_op( TREE, TREE_OP_ADD );
        else if ( is_tkn_keyword( tkn_op, KW_OpSub ) )
            node_op = new_node_op( TREE, TREE_OP_SUB );
        else
            break;
        MOVE_CURR_TO_END_OF_TOKEN(tkn_op); // now sure it is really our case

        TreeNode *node_new_mulive = get_mulive( FACT_REC_FALL_ARGS );
        SYN_ASSERT( node_new_mulive, prog, CURR, "Mulive" );

        tree_hang_loose_node_at_left( TREE, node_expr, node_op );
        tree_hang_loose_node_at_right( TREE, node_new_mulive, node_op );
        node_expr = node_op;
    }

    return node_expr;
}

static TreeNode *get_var( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    Token tkn_ident = get_token( CURR );
    if ( tkn_ident.type != TKN_TYPE_ID )
        return NULL;

    id_t id = ABSENT_ID;
    if ( context->in_func_action || context->in_func_recipe )
        id = find_ident_in_nametable( NT_FUNC_VARS, tkn_ident.id );
    else
        id = find_ident_in_nametable( NT_GLOBAL, tkn_ident.id );
    SYN_ASSERT( id != ABSENT_ID, prog, CURR, "Variable" );
    MOVE_CURR_TO_END_OF_TOKEN(tkn_ident);

    return new_node_id( TREE, id );
}

static TreeNode *get_assign( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    Token asgn1 = get_token( CURR );
    if ( !is_tkn_keyword( asgn1, KW_Asgn1 ) )
        return NULL;
    MOVE_CURR_TO_END_OF_TOKEN(asgn1);

    TreeNode *node_expr = get_expr( FACT_REC_FALL_ARGS );
    SYN_ASSERT( node_expr, prog, CURR, "Expression" );

    Token asgn2 = get_token( CURR );
    SYN_ASSERT( is_tkn_keyword( asgn2, KW_Asgn2 ), prog, CURR, KEYWORDS[KW_Asgn2].str );
    MOVE_CURR_TO_END_OF_TOKEN(asgn2);

    TreeNode *node_var = get_var( FACT_REC_FALL_ARGS );
    SYN_ASSERT( node_var, prog, CURR, "Variable" );


    TreeNode *node_assign = new_node_op( TREE, TREE_OP_ASSIGN );
    tree_hang_loose_node_at_left( TREE, node_expr, node_assign );
    tree_hang_loose_node_at_right( TREE, node_var, node_assign );


    Token dot = get_token( CURR );
    SYN_ASSERT( is_tkn_sep_char( dot, SEP_Dot ), prog, CURR, "!" );
    MOVE_CURR_TO_END_OF_TOKEN(dot);

    return node_assign;
}

static TreeNode *get_var_death( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    Token var_death = get_token( CURR );
    if ( !is_tkn_keyword( var_death, KW_VarDeathOp ) )
        return NULL;
    MOVE_CURR_TO_END_OF_TOKEN(var_death);

    Token var_ident = get_token( CURR );
    SYN_ASSERT( var_ident.type == TKN_TYPE_ID, prog, CURR, "Identificator" );

    int res = 0;
    if ( context->in_func_action || context->in_func_recipe )
        res = del_ident_from_nametable( &NT_FUNC_VARS, var_ident.id );
    else
        res = del_ident_from_nametable( &NT_GLOBAL, var_ident.id );

    SYN_ASSERT( res == 1, prog, CURR, "Variable" );
    MOVE_CURR_TO_END_OF_TOKEN(var_ident);

    TreeNode *node_dummy = new_node_op( TREE, TREE_OP_DUMMY );

    Token dot = get_token( CURR );
    SYN_ASSERT( is_tkn_sep_char(dot, SEP_Dot), prog, CURR, "\'!\'" );
    MOVE_CURR_TO_END_OF_TOKEN(dot);

    return node_dummy;
}

static TreeNode *get_var_birth( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    Token var_birth = get_token( CURR );
    if ( !is_tkn_keyword( var_birth, KW_VarBirthOp ) )
        return NULL;
    MOVE_CURR_TO_END_OF_TOKEN(var_birth);

    TreeNode *node_num = get_num( FACT_REC_FALL_ARGS );
    SYN_ASSERT(node_num, prog, CURR, "Number");

    Token units_of = get_token( CURR );
    SYN_ASSERT( is_tkn_keyword(units_of, KW_UnitsOf), prog, CURR, KEYWORDS[KW_UnitsOf].str );
    MOVE_CURR_TO_END_OF_TOKEN(units_of);

    Token tkn_id = get_token( CURR );
    SYN_ASSERT( tkn_id.type == TKN_TYPE_ID && check_is_ident_fresh( comp_prog, tkn_id.id, context ),
                prog, CURR, "A fresh identificator" );
    MOVE_CURR_TO_END_OF_TOKEN(tkn_id);

    id_t var_id = ABSENT_ID;
    if ( context->in_func_action || context->in_func_recipe )
        var_id = add_ident_into_nametable( &NT_FUNC_VARS, tkn_id.id );
    else
        var_id = add_ident_into_nametable( &NT_GLOBAL, tkn_id.id );

    TreeNode *node_assign = new_node_op( TREE, TREE_OP_ASSIGN );
    tree_hang_loose_node_at_left( TREE, node_num, node_assign );
    TreeNode *node_var = new_node_id( TREE, var_id );
    tree_hang_loose_node_at_right( TREE, node_var, node_assign );

    Token dot = get_token( CURR );
    SYN_ASSERT( is_tkn_sep_char(dot, SEP_Dot), prog, CURR, "\'!\'" );
    MOVE_CURR_TO_END_OF_TOKEN(dot);

    return node_assign;
}

static TreeNode *get_call_func_action( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    Token tkn_call_func = get_token( CURR );
    if ( !is_tkn_keyword( tkn_call_func, KW_CallFuncAction ) )
        return NULL;
    MOVE_CURR_TO_END_OF_TOKEN( tkn_call_func );

    Token tkn_func_id = get_token( CURR );
    SYN_ASSERT( tkn_func_id.type == TKN_TYPE_ID, prog, CURR, "Function name" );
    MOVE_CURR_TO_END_OF_TOKEN( tkn_func_id );

    id_t func_id = find_ident_in_nametable( NT_FUNCS, tkn_func_id.id );
    SYN_ASSERT( func_id != ABSENT_ID, prog, CURR, "A defined function's name" );
    SYN_ASSERT( NT_FUNCS.list[func_id].func_info.func_type == FUNC_TYPE_ACTION, prog,
                CURR, "Name of an ACTION function" );

    TreeNode *node_func_id = new_node_id( TREE, func_id );
    TreeNode *node_call_op = new_node_op( TREE, TREE_OP_CALL_FUNC );
    tree_hang_loose_node_at_left( TREE, node_func_id, node_call_op );

    Token tkn_bracket_opn = get_token( CURR );
    if ( is_tkn_sep_char(tkn_bracket_opn, SEP_BracketOpn) )
    {
        MOVE_CURR_TO_END_OF_TOKEN( tkn_bracket_opn );

        Token tkn_using = get_token( CURR );
        SYN_ASSERT( is_tkn_keyword( tkn_using, KW_Using ), prog,
                    CURR, KEYWORDS[KW_Using].str );
        MOVE_CURR_TO_END_OF_TOKEN( tkn_using );

        TreeNode *node_fact_args = get_fact_args( FACT_REC_FALL_ARGS );
        SYN_ASSERT( node_fact_args, prog, CURR, "At least one arg" );

        size_t num_args = count_list_len( node_fact_args );
        size_t needed_num_args = NT_FUNCS.list[func_id].func_info.func_args_count;
        SYN_ASSERT( num_args == needed_num_args, prog, CURR,
                    "Number of args doesn't match function's definition" );

        tree_hang_loose_node_at_right( TREE, node_fact_args, node_call_op );

        Token tkn_as_ingr = get_token( CURR );
        SYN_ASSERT( is_tkn_keyword( tkn_as_ingr, KW_AsIngr ), prog,
                    CURR, KEYWORDS[KW_AsIngr].str );
        MOVE_CURR_TO_END_OF_TOKEN( tkn_as_ingr );

        Token tkn_bracket_cls = get_token( CURR );
        SYN_ASSERT( is_tkn_sep_char( tkn_bracket_cls, SEP_BracketCls ), prog,
                    CURR, ")" );
        MOVE_CURR_TO_END_OF_TOKEN( tkn_bracket_cls );
    }

    Token dot = get_token( CURR );
    SYN_ASSERT( is_tkn_sep_char(dot, SEP_Dot), prog, CURR, "\'!\'" );
    MOVE_CURR_TO_END_OF_TOKEN(dot);

    return node_call_op;
}

static TreeNode *get_call_func_recipe( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    Token tkn_call_func = get_token( CURR );
    if ( !is_tkn_keyword( tkn_call_func, KW_CallFuncRecipe ) )
        return NULL;
    MOVE_CURR_TO_END_OF_TOKEN( tkn_call_func );

    Token tkn_func_id = get_token( CURR );
    SYN_ASSERT( tkn_func_id.type == TKN_TYPE_ID, prog, CURR, "Function name" );
    MOVE_CURR_TO_END_OF_TOKEN( tkn_func_id );

    id_t func_id = find_ident_in_nametable( NT_FUNCS, tkn_func_id.id );
    SYN_ASSERT( func_id != ABSENT_ID, prog, CURR, "A defined function's name" );
    SYN_ASSERT( NT_FUNCS.list[func_id].func_info.func_type == FUNC_TYPE_RECIPE, prog,
                CURR, "Name of a RECIPE function" );

    TreeNode *node_func_id = new_node_id( TREE, func_id );
    TreeNode *node_call_op = new_node_op( TREE, TREE_OP_CALL_FUNC );
    tree_hang_loose_node_at_left( TREE, node_func_id, node_call_op );

    Token tkn_bracket_opn = get_token( CURR );
    if ( is_tkn_sep_char(tkn_bracket_opn, SEP_BracketOpn) )
    {
        MOVE_CURR_TO_END_OF_TOKEN( tkn_bracket_opn );

        Token tkn_using = get_token( CURR );
        SYN_ASSERT( is_tkn_keyword( tkn_using, KW_Using ), prog,
                    CURR, KEYWORDS[KW_Using].str );
        MOVE_CURR_TO_END_OF_TOKEN( tkn_using );

        TreeNode *node_fact_args = get_fact_args( FACT_REC_FALL_ARGS );
        SYN_ASSERT( node_fact_args, prog, CURR, "At least one arg" );

        size_t num_args = count_list_len( node_fact_args );
        size_t needed_num_args = NT_FUNCS.list[func_id].func_info.func_args_count;
        SYN_ASSERT( num_args == needed_num_args, prog, CURR,
                    "Number of args doesn't match function's definition" );

        tree_hang_loose_node_at_right( TREE, node_fact_args, node_call_op );

        Token tkn_as_ingr = get_token( CURR );
        SYN_ASSERT( is_tkn_keyword( tkn_as_ingr, KW_AsIngr ), prog,
                    CURR, KEYWORDS[KW_AsIngr].str );
        MOVE_CURR_TO_END_OF_TOKEN( tkn_as_ingr );

        Token tkn_bracket_cls = get_token( CURR );
        SYN_ASSERT( is_tkn_sep_char( tkn_bracket_cls, SEP_BracketCls ), prog,
                    CURR, ")" );
        MOVE_CURR_TO_END_OF_TOKEN( tkn_bracket_cls );
    }

    return node_call_op;
}

static TreeNode *get_return( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    Token tkn_ret1 = get_token( CURR );
    if ( !is_tkn_keyword( tkn_ret1, KW_Return1 ) )
        return NULL;
    SYN_ASSERT( context->in_func_recipe, prog, CURR,
                "Return can be used only in Recipe Functions" );
    MOVE_CURR_TO_END_OF_TOKEN( tkn_ret1 );

    TreeNode *node_expr = get_expr( FACT_REC_FALL_ARGS );
    SYN_ASSERT( node_expr, prog, CURR, "An expression" );

    TreeNode *node_ret = new_node_op( TREE, TREE_OP_RETURN );
    tree_hang_loose_node_at_right( TREE, node_expr, node_ret );

    Token tkn_ret2 = get_token( CURR );
    SYN_ASSERT( is_tkn_keyword( tkn_ret2, KW_Return2 ), prog, CURR, KEYWORDS[KW_Return2].str);
    MOVE_CURR_TO_END_OF_TOKEN(tkn_ret2);

    return node_ret;
}

static TreeNode *get_print_num( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    Token tkn_print_num = get_token( CURR );
    if ( !is_tkn_keyword( tkn_print_num, KW_PrintNum ) )
        return NULL;
    MOVE_CURR_TO_END_OF_TOKEN( tkn_print_num );

    TreeNode *node_var = get_var( FACT_REC_FALL_ARGS );
    SYN_ASSERT( node_var, prog, CURR, "Variable" );

    TreeNode *node_print_num = new_node_op( TREE, TREE_OP_PRINT_NUM );
    tree_hang_loose_node_at_right( TREE, node_var, node_print_num );

    Token dot = get_token( CURR );
    SYN_ASSERT( is_tkn_sep_char(dot, SEP_Dot), prog, CURR, "\'!\'" );
    MOVE_CURR_TO_END_OF_TOKEN(dot);

    return node_print_num;
}

static TreeNode *get_op( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    TreeNode *node_op = NULL;

       ( node_op =        get_var_birth( FACT_REC_FALL_ARGS ) )
    || ( node_op =        get_var_death( FACT_REC_FALL_ARGS ) )
    || ( node_op =           get_assign( FACT_REC_FALL_ARGS ) )
    || ( node_op =               get_if( FACT_REC_FALL_ARGS ) )
    || ( node_op =            get_while( FACT_REC_FALL_ARGS ) )
    || ( node_op =           get_return( FACT_REC_FALL_ARGS ) )
    || ( node_op = get_call_func_action( FACT_REC_FALL_ARGS ) )
    || ( node_op =        get_print_num( FACT_REC_FALL_ARGS ) );

    return node_op;
}

static TreeNode *get_operators( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    // first operator (obligatory)
    TreeNode *node_first_op = get_op( FACT_REC_FALL_ARGS );
    SYN_ASSERT( node_first_op, prog, CURR, "Operator" );

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
        tree_migrate_into_right( TREE, tree_get_parent(node_curr_op), tree_get_left_child( node_curr_op ) );

    return node_first_op;
}

static TreeNode *get_fact_args( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    TreeNode *node_list = new_node_op( TREE, TREE_OP_LIST_CONNECTOR );

    TreeNode *node_first_expr = get_expr( FACT_REC_FALL_ARGS );
    SYN_ASSERT( node_first_expr, prog, CURR, "At least one expression" );
    tree_hang_loose_node_at_left( TREE, node_first_expr, node_list );

    Token tkn_comma = {};
    TreeNode *curr_node = node_list;
    while ( is_tkn_sep_char( tkn_comma = get_token( CURR ), SEP_Comma) )
    {
        MOVE_CURR_TO_END_OF_TOKEN(tkn_comma);

        TreeNode *node_new_expr = get_expr( FACT_REC_FALL_ARGS );
        SYN_ASSERT( node_new_expr, prog, CURR, "An expression" );

        TreeNode *node_connect = new_node_op( TREE, TREE_OP_LIST_CONNECTOR );
        tree_hang_loose_node_at_left( TREE, node_new_expr, node_connect );
        tree_hang_loose_node_at_right( TREE, node_connect, curr_node );

        curr_node = node_connect;
    }

    return node_list;
}

static TreeNode *get_formal_args( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    TreeNode *node_list = new_node_op( TREE, TREE_OP_LIST_CONNECTOR );

    Token tkn_id = get_token( CURR );
    SYN_ASSERT( tkn_id.type == TKN_TYPE_ID && check_is_ident_fresh( comp_prog, tkn_id.id, context ),
                prog, CURR, "At least one fresh variable name" );
    MOVE_CURR_TO_END_OF_TOKEN( tkn_id );

    id_t id = add_ident_into_nametable( &NT_FUNC_VARS, tkn_id.id );
    TreeNode *node_first_arg = new_node_id( TREE, id );
    tree_hang_loose_node_at_left( TREE, node_first_arg, node_list );

    Token tkn_comma = {};
    TreeNode *curr_node = node_list;
    while ( is_tkn_sep_char( tkn_comma = get_token( CURR ), SEP_Comma) )
    {
        MOVE_CURR_TO_END_OF_TOKEN(tkn_comma);

        Token tkn_new_id = get_token( CURR );
        SYN_ASSERT( tkn_new_id.type == TKN_TYPE_ID && check_is_ident_fresh( comp_prog, tkn_new_id.id, context ),
                    prog, CURR, "A fresh variable name" );
        MOVE_CURR_TO_END_OF_TOKEN( tkn_new_id );

        id_t new_id = add_ident_into_nametable( &NT_FUNC_VARS, tkn_new_id.id );
        TreeNode *node_new_id = new_node_id( TREE, new_id );

        TreeNode *node_connect = new_node_op( TREE, TREE_OP_LIST_CONNECTOR );
        tree_hang_loose_node_at_left( TREE, node_new_id, node_connect );
        tree_hang_loose_node_at_right( TREE, node_connect, curr_node );

        curr_node = node_connect;
    }

    return node_list;
}

static TreeNode *get_func_recipe( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    Token func_header = get_token( CURR );
    if ( !is_tkn_keyword(func_header, KW_FuncRecipeHeader) )
        return NULL;
    MOVE_CURR_TO_END_OF_TOKEN( func_header );

    context->in_func_recipe = 1;

    Token func_ident = get_token( CURR );
    SYN_ASSERT( func_ident.type == TKN_TYPE_ID && check_is_ident_fresh(comp_prog, func_ident.id, context),
                prog, CURR, "Fresh function identificator" );
    MOVE_CURR_TO_END_OF_TOKEN( func_ident );

    id_t func_id = add_ident_into_nametable( &NT_FUNCS, func_ident.id );
    NT_FUNCS.list[func_id].func_info.func_type = FUNC_TYPE_RECIPE;
    TreeNode *node_func_id = new_node_id( TREE, func_id );

    TreeNode *node_func_def = new_node_op( TREE, TREE_OP_FUNC_DEF );
    TreeNode *node_func_def_helper = new_node_op( TREE, TREE_OP_FUNC_DEF_HELPER );
    tree_hang_loose_node_at_left( TREE, node_func_id, node_func_def );
    tree_hang_loose_node_at_right( TREE, node_func_def_helper, node_func_def );

    Token tkn_using = get_token( CURR );
    TreeNode *node_formal_args = NULL;
    if ( is_tkn_keyword( tkn_using, KW_Using ) )
    {
        MOVE_CURR_TO_END_OF_TOKEN( tkn_using );

        node_formal_args = get_formal_args( FACT_REC_FALL_ARGS );
        SYN_ASSERT( node_formal_args, prog, CURR, "Formal args" );

        NT_FUNCS.list[func_id].func_info.func_args_count = count_list_len( node_formal_args );

        Token tkn_as_ingr = get_token( CURR );
        SYN_ASSERT( is_tkn_keyword( tkn_as_ingr, KW_AsIngr ), prog,
                    CURR, KEYWORDS[KW_AsIngr].str );
        MOVE_CURR_TO_END_OF_TOKEN( tkn_as_ingr );
    }

    Token tkn_colon = get_token( CURR );
    SYN_ASSERT( is_tkn_sep_char( tkn_colon, SEP_Colon ), prog, CURR, ":" );
    MOVE_CURR_TO_END_OF_TOKEN( tkn_colon );

    TreeNode *node_operators = get_operators( FACT_REC_FALL_ARGS );
    SYN_ASSERT( node_operators, prog, CURR, "Operators" );

    if (node_formal_args)
        tree_hang_loose_node_at_left( TREE, node_formal_args, node_func_def_helper );

    tree_hang_loose_node_at_right( TREE, node_operators, node_func_def_helper);

    context->in_func_recipe = 0;

    return node_func_def;
}

static TreeNode *get_func_action( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    Token func_header = get_token( CURR );
    if ( !is_tkn_keyword(func_header, KW_FuncActionHeader) )
        return NULL;
    MOVE_CURR_TO_END_OF_TOKEN( func_header );

    context->in_func_action = 1;

    Token func_ident = get_token( CURR );
    SYN_ASSERT( func_ident.type == TKN_TYPE_ID && check_is_ident_fresh(comp_prog, func_ident.id, context),
                prog, CURR, "Fresh function identificator" );
    MOVE_CURR_TO_END_OF_TOKEN( func_ident );

    id_t func_id = add_ident_into_nametable( &NT_FUNCS, func_ident.id );
    NT_FUNCS.list[func_id].func_info.func_type = FUNC_TYPE_ACTION;
    TreeNode *node_func_id = new_node_id( TREE, func_id );

    TreeNode *node_func_def = new_node_op( TREE, TREE_OP_FUNC_DEF );
    TreeNode *node_func_def_helper = new_node_op( TREE, TREE_OP_FUNC_DEF_HELPER );
    tree_hang_loose_node_at_left( TREE, node_func_id, node_func_def );
    tree_hang_loose_node_at_right( TREE, node_func_def_helper, node_func_def );

    Token tkn_using = get_token( CURR );
    TreeNode *node_formal_args = NULL;
    if ( is_tkn_keyword( tkn_using, KW_Using ) )
    {
        MOVE_CURR_TO_END_OF_TOKEN( tkn_using );

        node_formal_args = get_formal_args( FACT_REC_FALL_ARGS );
        SYN_ASSERT( node_formal_args, prog, CURR, "Formal args" );

        NT_FUNCS.list[func_id].func_info.func_args_count = count_list_len( node_formal_args );

        Token tkn_as_ingr = get_token( CURR );
        SYN_ASSERT( is_tkn_keyword( tkn_as_ingr, KW_AsIngr ), prog,
                    CURR, KEYWORDS[KW_AsIngr].str );
        MOVE_CURR_TO_END_OF_TOKEN( tkn_as_ingr );
    }

    Token tkn_colon = get_token( CURR );
    SYN_ASSERT( is_tkn_sep_char( tkn_colon, SEP_Colon ), prog, CURR, ":" );
    MOVE_CURR_TO_END_OF_TOKEN( tkn_colon );

    TreeNode *node_operators = get_operators( FACT_REC_FALL_ARGS );
    SYN_ASSERT( node_operators, prog, CURR, "Operators" );

    if (node_formal_args)
        tree_hang_loose_node_at_left( TREE, node_formal_args, node_func_def_helper );

    tree_hang_loose_node_at_right( TREE, node_operators, node_func_def_helper);

    context->in_func_action = 0;

    return node_func_def;
}

static TreeNode *get_func_def( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    TreeNode *node_func_recipe = get_func_recipe( FACT_REC_FALL_ARGS );
    if ( node_func_recipe )
        return node_func_recipe;

    TreeNode *node_func_action = get_func_action( FACT_REC_FALL_ARGS );
    if ( node_func_action )
        return node_func_action;

    return NULL;
}

static TreeNode *get_func_defs( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    Token func_defs_start = get_token( CURR );
    if ( !is_tkn_keyword( func_defs_start, KW_FuncDefsStart ) )
        return NULL;
    MOVE_CURR_TO_END_OF_TOKEN( func_defs_start );

    // first function definition (obligatory)
    TreeNode *node_func_defs = get_func_def( FACT_REC_FALL_ARGS );
    SYN_ASSERT( node_func_defs, prog, CURR, "Function definition" );

    // other function definitions (optional)
    TreeNode *node_new_func   = NULL;
    TreeNode *node_curr_func  = NULL;
    while ( (node_new_func = get_func_def( FACT_REC_FALL_ARGS )) )
    {
        if ( !node_curr_func )
        {
            TreeNode *node_first_seq = new_node_op( TREE, TREE_OP_SEQ_EXEC );
            tree_hang_loose_node_at_left( TREE, node_func_defs, node_first_seq );
            node_curr_func = node_first_seq;
            node_func_defs = node_first_seq;
        }

        TreeNode *node_new_seq = new_node_op( TREE, TREE_OP_SEQ_EXEC );
        tree_hang_loose_node_at_left( TREE, node_new_func, node_new_seq );
        tree_hang_loose_node_at_right( TREE, node_new_seq, node_curr_func );

        node_curr_func = node_new_seq;
    }

    if (node_curr_func)
        tree_migrate_into_right( TREE, tree_get_parent(node_curr_func), tree_get_left_child( node_curr_func ) );


    Token func_defs_end = get_token( CURR );
    SYN_ASSERT( is_tkn_keyword( func_defs_end, KW_FuncDefsEnd ),
                prog, CURR, KEYWORDS[KW_FuncDefsStart].str );
    MOVE_CURR_TO_END_OF_TOKEN( func_defs_end );

    return node_func_defs;
}

static TreeNode *get_prog( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    Token prog_start = get_token( CURR );
    SYN_ASSERT( is_tkn_keyword(prog_start, KW_ProgStart), prog, CURR, KEYWORDS[KW_ProgStart].str );
    MOVE_CURR_TO_END_OF_TOKEN(prog_start);

    TreeNode *node_func_defs = get_func_defs( FACT_REC_FALL_ARGS );

    TreeNode *node_operators = get_operators( FACT_REC_FALL_ARGS );
    SYN_ASSERT( node_operators, prog, CURR, "Operators" );

    TreeNode *node_prog = NULL;
    if ( node_func_defs )
    {
        node_prog = new_node_op( TREE, TREE_OP_SEQ_EXEC );
        tree_hang_loose_node_at_left( TREE, node_func_defs, node_prog );
        tree_hang_loose_node_at_right( TREE, node_operators, node_prog );
    }
    else
    {
        node_prog = node_operators;
    }

    Token prog_end = get_token( CURR );
    SYN_ASSERT( is_tkn_keyword(prog_end, KW_ProgEnd), prog, CURR, KEYWORDS[KW_ProgEnd].str );
    MOVE_CURR_TO_END_OF_TOKEN(prog_end);

    return node_prog;
}

Status compile_prog( const char *prog, CompiledProgram *comp_prog )
{
    assert(prog);

    *comp_prog = {};
    comp_prog->tree = {};
    tree_ctor( &comp_prog->tree, sizeof( TreeNodeData ), NULL, print_tree_node_data );
    Status err = Nametables_ctor( &comp_prog->nametables );
    if ( err )
    {
        tree_dtor( &comp_prog->tree );
        return err;
    }

    const char *curr = prog;
    Context context = {};
    TreeNode *root = get_prog( comp_prog, prog, &curr, &context );
    if (!root)
        return STATUS_ERROR_COMPILATION_ERROR;

    tree_hang_loose_node_as_root( &comp_prog->tree, root );

    dump_compiler_tree( &comp_prog->tree );

    return STATUS_OK;
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

void print_rec_fall_err_msg( const char *prog, const char *error_ptr, const char *expected )
{
    assert(prog);
    assert(error_ptr);
    assert(expected);

    error_ptr = skip_spaces(error_ptr);

    if (!error_ptr)
        error_ptr = prog + strlen( prog ) - 1;

    ERROR("Syntax error: expected <%s>, found:", expected);

    const char *line_start = error_ptr;
    while ( *line_start != '\n' && line_start != prog )
        line_start--;

    if ( *line_start == '\n' )
        line_start++;

    FILE *err_stream = log_get_stream();

    const char *ptr = line_start;
    while ( *ptr != '\n' && *ptr != '\0' )
    {
        putc( *ptr, err_stream );
        ptr++;
    }
    putc( '\n', err_stream );

    put_n_chars( err_stream, ' ', error_ptr - line_start );
    putc( '^', err_stream );
    putc( '\n', err_stream );
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

Status nametable_ctor( Nametable *nt_ptr, NametableType type )
{
    assert(nt_ptr);

    nt_ptr->list = (NametableElem*) calloc( NAMES_DEFAULT_COUNT, sizeof(NametableElem) );
    if (!nt_ptr->list)
    {
        ERROR(  "Memory allocation error: attempt to allocate %llu bytes",
                NAMES_DEFAULT_COUNT * sizeof(NametableElem));
        return STATUS_ERROR_MEMORY_ALLOC_ERROR;
    }

    nt_ptr->list_curr_len = 0;
    nt_ptr->list_cap = NAMES_DEFAULT_COUNT;
    nt_ptr->type = type;

    return STATUS_OK;
}

Status Nametables_ctor( Nametables *nametables )
{
    assert(nametables);

    if ( nametable_ctor( &nametables->global_vars, NT_TYPE_GLOBAL_VAR ) != STATUS_OK )
        return STATUS_ERROR_MEMORY_ALLOC_ERROR;

    if ( nametable_ctor( &nametables->funcs, NT_TYPE_FUNC ) != STATUS_OK )
    {
        FREE(nametables->global_vars.list);
        return STATUS_ERROR_MEMORY_ALLOC_ERROR;
    }

    if ( nametable_ctor( &nametables->func_vars, NT_TYPE_FUNC_VAR ) != STATUS_OK )
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

