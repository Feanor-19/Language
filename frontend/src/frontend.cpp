#include "frontend.h"

#include <ctype.h>
#include <math.h>
#include ".\..\..\common\src\compiler_tree_dump.h"



// TODO - норм дефайны? вроде и сокращают, и вроде бы пониманию не мешают... "вроде бы"
#define CURR (*curr_ptr)
#define FACT_REC_FALL_ARGS comp_prog, prog, curr_ptr
#define FORMAL_REC_FALL_ARGS CompiledProgram *comp_prog, const char *prog, const char **curr_ptr
#define TREE &comp_prog->tree
#define NT_GLOBAL comp_prog->nametables.global_vars
#define NT_FUNCS comp_prog->nametables.funcs
#define NT_FUNC_VARS comp_prog->nametables.func_vars
#define MOVE_CURR_TO_END_OF_TOKEN( token ) (CURR = token.start + token.len)



static TreeNode *get_num        ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_while      ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_if         ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_primal     ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_unr        ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_mulive     ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_expr       ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_var        ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_assign     ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_var_death  ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_var_birth  ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_op         ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_operators  ( FORMAL_REC_FALL_ARGS );
static TreeNode *get_prog       ( FORMAL_REC_FALL_ARGS );



//! @brief Returns index of given 'ident' in the 'nametable', if found;
//! otherwise returns ABSENT_ID;
inline id_t find_ident_in_nametable( Nametable nametable, Identificator ident )
{
    for (id_t ind = 0; ind < (id_t) nametable.list_curr_len; ind++)
    {
        if ( cmp_idents( ident, nametable.list[ind] ) )
            return ind;
    }

    return ABSENT_ID;
}

//! @brief Returns 1 if ident is not found in any of the comp_prog's nametables, 0 otherwise.
inline int check_is_ident_fresh( CompiledProgram *comp_prog, Identificator ident )
{
    return find_ident_in_nametable( NT_GLOBAL, ident )      == ABSENT_ID
        && find_ident_in_nametable( NT_FUNCS, ident )       == ABSENT_ID
        && find_ident_in_nametable( NT_FUNC_VARS, ident )   == ABSENT_ID;
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
        if ( is_ident_empty(nametable->list[ind])  )
        {
            nametable->list[ind] = ident;
            return ind;
        }
    }

    REALLOC_ARR_WRP( nametable->list, Identificator );
    nametable->list[nametable->list_curr_len++] = ident;
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
        if ( cmp_idents( nametable->list[ind], ident ) )
        {
            nametable->list[ind].start = NULL;
            nametable->list[ind].len = 0;
            return 1;
        }
    }

    return 0;
}

static TreeNode *get_num( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    Token tkn_num = get_token( CURR );
    SYN_ASSERT( tkn_num.type == TKN_TYPE_NUM, CURR, "Number" );
    MOVE_CURR_TO_END_OF_TOKEN( tkn_num );

    return new_node_num( TREE, tkn_num.num );
}

//! @brief Checks is 'tkn' of type 'Keyword' and belongs to
//! group 'CmpOp'. If yes, returns corresponding OpsInTree,
//! otherwise returns TREE_OP_DUMMY.
inline OpsInTree translate_kw_cmp_op( Token tkn )
{
// TODO - норм решение?
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
    SYN_ASSERT( is_tkn_keyword( tkn_cond, KW_Cond ), CURR, "Keyword Cond" );
    MOVE_CURR_TO_END_OF_TOKEN(tkn_cond);

    Token tkn_cmp_op = get_token( CURR );
    SYN_ASSERT( translate_kw_cmp_op( tkn_cmp_op ) != TREE_OP_DUMMY, CURR, "Comparing operator" );
    MOVE_CURR_TO_END_OF_TOKEN(tkn_cmp_op);

    TreeNode *node_cmp_op = new_node_op( TREE, translate_kw_cmp_op(tkn_cmp_op) );

    tree_hang_loose_node_at_left( TREE, node_cmp_op, node_if );

    TreeNode *node_expr_left = get_expr(FACT_REC_FALL_ARGS);
    SYN_ASSERT( node_expr_left, CURR, "Expression");
    tree_hang_loose_node_at_left( TREE, node_expr_left, node_cmp_op );

    Token tkn_than = get_token( CURR );
    SYN_ASSERT( is_tkn_keyword( tkn_than, KW_Than ), CURR, KEYWORDS[KW_Than].str );
    MOVE_CURR_TO_END_OF_TOKEN(tkn_than);

    TreeNode *node_expr_right = get_expr(FACT_REC_FALL_ARGS);
    SYN_ASSERT( node_expr_right, CURR, "Expression");
    tree_hang_loose_node_at_right( TREE, node_expr_right, node_cmp_op );

    Token tkn_if2 = get_token( CURR );
    SYN_ASSERT( is_tkn_keyword(tkn_if2, KW_If2), CURR, KEYWORDS[KW_If2].str );
    MOVE_CURR_TO_END_OF_TOKEN(tkn_if2);

    TreeNode *node_operators = get_operators( FACT_REC_FALL_ARGS );
    SYN_ASSERT( node_operators, CURR, "Operators" );
    tree_hang_loose_node_at_right( TREE, node_operators, node_if );

    Token tkn_if_end = get_token( CURR );
    SYN_ASSERT( is_tkn_keyword( tkn_if_end, KW_IfEnd ), CURR, KEYWORDS[KW_IfEnd].str )
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

    TreeNode *node_while = new_node_op( TREE, TREE_OP_WHILE );

    Token tkn_cond = get_token( CURR );
    SYN_ASSERT( is_tkn_keyword( tkn_cond, KW_Cond ), CURR, "Keyword Cond" );
    MOVE_CURR_TO_END_OF_TOKEN(tkn_cond);

    Token tkn_cmp_op = get_token( CURR );
    SYN_ASSERT( translate_kw_cmp_op( tkn_cmp_op ) != TREE_OP_DUMMY, CURR, "Comparing operator" );
    MOVE_CURR_TO_END_OF_TOKEN(tkn_cmp_op);

    TreeNode *node_cmp_op = new_node_op( TREE, translate_kw_cmp_op(tkn_cmp_op) );

    tree_hang_loose_node_at_left( TREE, node_cmp_op, node_while );

    TreeNode *node_expr_left = get_expr(FACT_REC_FALL_ARGS);
    SYN_ASSERT( node_expr_left, CURR, "Expression");
    tree_hang_loose_node_at_left( TREE, node_expr_left, node_cmp_op );

    Token tkn_than = get_token( CURR );
    SYN_ASSERT( is_tkn_keyword( tkn_than, KW_Than ), CURR, KEYWORDS[KW_Than].str );
    MOVE_CURR_TO_END_OF_TOKEN(tkn_than);

    TreeNode *node_expr_right = get_expr(FACT_REC_FALL_ARGS);
    SYN_ASSERT( node_expr_right, CURR, "Expression");
    tree_hang_loose_node_at_right( TREE, node_expr_right, node_cmp_op );

    Token tkn_while2 = get_token( CURR );
    SYN_ASSERT( is_tkn_keyword(tkn_while2, KW_While2), CURR, KEYWORDS[KW_While2].str );
    MOVE_CURR_TO_END_OF_TOKEN(tkn_while2);

    TreeNode *node_operators = get_operators( FACT_REC_FALL_ARGS );
    SYN_ASSERT( node_operators, CURR, "Operators" );
    tree_hang_loose_node_at_right( TREE, node_operators, node_while );

    Token tkn_while_end = get_token( CURR );
    SYN_ASSERT( is_tkn_keyword( tkn_while_end, KW_WhileEnd ), CURR, KEYWORDS[KW_WhileEnd].str )
    MOVE_CURR_TO_END_OF_TOKEN(tkn_while_end);

    return node_while;
}

static TreeNode *get_primal( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    Token tkn = get_token( CURR );
    if ( is_tkn_keyword( tkn, KW_BracketOpn ) )
    {
        MOVE_CURR_TO_END_OF_TOKEN(tkn);

        TreeNode *node_expr = get_expr(FACT_REC_FALL_ARGS);
        SYN_ASSERT(node_expr, CURR, "Expression");

        tkn = get_token( CURR );
        SYN_ASSERT( is_tkn_sep_char( tkn, SEP_BracketCls ), CURR, "," );
        MOVE_CURR_TO_END_OF_TOKEN(tkn);

        return node_expr;
    }
    else
    {
        TreeNode *node_var = get_var(FACT_REC_FALL_ARGS);
        SYN_ASSERT( node_var, CURR, "Variable" );

        return node_var;
    }

    SYN_ASSERT( 0, CURR, "BracketOpn or Variable" );
}

//! @brief Checks is 'tkn' of type 'Keyword' and belongs to
//! group 'UnrOp'. If yes, returns corresponding OpsInTree,
//! otherwise returns TREE_OP_DUMMY.
inline OpsInTree translate_tkn_unr_op( Token tkn )
{
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
        SYN_ASSERT( node_primal, CURR, "Primal" );

        tree_hang_loose_node_at_right( TREE, node_primal, node_op );

        return node_op;
    }
    else
    {
        TreeNode *node_primal = get_primal( FACT_REC_FALL_ARGS );
        SYN_ASSERT( node_primal, CURR, "Primal" );
        return node_primal;
    }
}

static TreeNode *get_mulive( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    TreeNode *node_mulive = get_unr( FACT_REC_FALL_ARGS );
    SYN_ASSERT( node_mulive, CURR, "Expression part: unr" );

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
        SYN_ASSERT( node_new_unr, CURR, "Unr" );

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
    SYN_ASSERT( node_expr, CURR, "Expression part: mulive" );

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
        SYN_ASSERT( node_new_mulive, CURR, "Mulive" );

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

    id_t id = find_ident_in_nametable( NT_GLOBAL, tkn_ident.id );
    SYN_ASSERT( id != ABSENT_ID, CURR, "Variable" );
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
    SYN_ASSERT( node_expr, CURR, "Expression" );

    Token asgn2 = get_token( CURR );
    SYN_ASSERT( is_tkn_keyword( asgn2, KW_Asgn2 ), CURR, KEYWORDS[KW_Asgn2].str );
    MOVE_CURR_TO_END_OF_TOKEN(asgn2);

    TreeNode *node_var = get_var( FACT_REC_FALL_ARGS );
    SYN_ASSERT( node_var, CURR, "Variable" );


    TreeNode *node_assign = new_node_op( TREE, TREE_OP_ASSIGN );
    tree_hang_loose_node_at_left( TREE, node_expr, node_assign );
    tree_hang_loose_node_at_right( TREE, node_var, node_assign );


    Token dot = get_token( CURR );
    SYN_ASSERT( is_tkn_sep_char( dot, SEP_Dot ), CURR, "!" );
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
    SYN_ASSERT( var_ident.type == TKN_TYPE_ID, CURR, "Identificator" );
    int res = del_ident_from_nametable( &NT_GLOBAL, var_ident.id );
    SYN_ASSERT( res == 1, CURR, "Variable" );
    MOVE_CURR_TO_END_OF_TOKEN(var_ident);

    TreeNode *node_dummy = new_node_op( TREE, TREE_OP_DUMMY );

    Token dot = get_token( CURR );
    SYN_ASSERT( is_tkn_sep_char(dot, SEP_Dot), CURR, "\'!\'" );
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
    SYN_ASSERT(node_num, CURR, "Number");

    Token units_of = get_token( CURR );
    SYN_ASSERT( is_tkn_keyword(units_of, KW_UnitsOf), CURR, KEYWORDS[KW_UnitsOf].str );
    MOVE_CURR_TO_END_OF_TOKEN(units_of);

    Token tkn_id = get_token( CURR );
    SYN_ASSERT( tkn_id.type != TKN_TYPE_ID || check_is_ident_fresh( comp_prog, tkn_id.id ),
                CURR, "A fresh identificator" );
    MOVE_CURR_TO_END_OF_TOKEN(tkn_id);

    id_t var_id = add_ident_into_nametable( &NT_GLOBAL, tkn_id.id );

    TreeNode *node_assign = new_node_op( TREE, TREE_OP_ASSIGN );
    tree_hang_loose_node_at_left( TREE, node_num, node_assign );
    TreeNode *node_var = new_node_id( TREE, var_id );
    tree_hang_loose_node_at_right( TREE, node_var, node_assign );

    Token dot = get_token( CURR );
    SYN_ASSERT( is_tkn_sep_char(dot, SEP_Dot), CURR, "\'!\'" );
    MOVE_CURR_TO_END_OF_TOKEN(dot);

    return node_assign;
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

static TreeNode *get_operators( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

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
        tree_migrate_into_right( TREE, tree_get_parent(node_curr_op), tree_get_left_child( node_curr_op ) );

    return node_first_op;
}

static TreeNode *get_prog( FORMAL_REC_FALL_ARGS )
{
    assert(comp_prog);
    assert(prog);
    assert(curr_ptr);

    Token prog_start = get_token( CURR );
    SYN_ASSERT( is_tkn_keyword(prog_start, KW_ProgStart), CURR, KEYWORDS[KW_ProgStart].str );
    MOVE_CURR_TO_END_OF_TOKEN(prog_start);

    TreeNode *node_operators = get_operators( FACT_REC_FALL_ARGS );
    SYN_ASSERT( node_operators, CURR, "Operators" );

    Token prog_end = get_token( CURR );
    SYN_ASSERT( is_tkn_keyword(prog_end, KW_ProgEnd), CURR, KEYWORDS[KW_ProgEnd].str );
    MOVE_CURR_TO_END_OF_TOKEN(prog_end);

    return node_operators;
}

CompiledProgram compile_prog( const char *prog )
{
    assert(prog);

    CompiledProgram comp_prog = {};
    comp_prog.tree = {};
    tree_ctor( &comp_prog.tree, sizeof( TreeNodeData ), NULL, print_tree_node_data );
    Nametables_ctor( &comp_prog.nametables );

    const char *curr = prog;
    TreeNode *root = get_prog( &comp_prog, prog, &curr );
    if (!root)
    {
        tree_dtor( &comp_prog.tree ); // TODO - как-то вернуть ошибку в main
        return comp_prog;
    }

    tree_hang_loose_node_as_root( &comp_prog.tree, root );

    dump_compiler_tree( &comp_prog.tree );

    return comp_prog;
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

    ERROR("Syntax error: expected <%s>, found:", expected);
    char found[ERROR_MSG_LEN + 1] = "";
    for (size_t ind = 0; ind < ERROR_MSG_LEN && str[ind] != '\0'; ind++)
    {
        found[ind] = str[ind];
    }
    ERROR( "%s\n", found );
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
