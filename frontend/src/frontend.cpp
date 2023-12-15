#include "frontend.h"

#include <ctype.h>
#include <math.h>

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

//! @brief Returns SepCharName corresponding to given char 'c'.
//! If there is no such SepChar, return SEP_FICTIONAL.
inline SepCharName get_sep_char( char c )
{
    for ( size_t sep_char = 0; sep_char < SIZEARR( SEPCHARS ); sep_char++ )
    {
        if ( SEPCHARS[sep_char].c == c )
            return SEPCHARS[sep_char].name;
    }
    return SEP_FICTIONAL;
}

//! @brief If prefix of 'str' is equal to the whole 'keyword',
//! returns strlen(keyword), otherwise returns -1.
inline int cmp_keyword( const char *keyword, const char *str )
{
    assert(keyword);
    assert(str);

    size_t kw_ind = 0;
    size_t str_ind = 0;

    while (keyword[kw_ind] != '\0')
    {
        if ( str[str_ind] != keyword[kw_ind] )
            return -1;

        kw_ind++;
        str_ind++;
    }

    return (int) kw_ind;
}

//! @brief Returns KeywordName, found in the prefix of the given 'str'.
//! Puts length of the found keyword into *len_ptr.
//! If there is no corresponding KeywordName, returns KW_FICTIONAL.
inline KeywordName get_keyword( const char *str, size_t *len_ptr )
{
    assert(str);
    assert(len_ptr);

    int kw_len = 0;
    for (size_t keyword = 0; keyword < SIZEARR( KEYWORDS ); keyword++)
    {
        if ( (kw_len = cmp_keyword( KEYWORDS[keyword].str, str ) ) != -1 )
        {
            *len_ptr = (size_t) kw_len;
            return KEYWORDS[keyword].name;
        }
    }

    return KW_FICTIONAL;
}

//! @brief Returns identificator, found in the prefix of the given 'str'.
inline Identificator get_identificator( const char *str )
{
    size_t str_ind = 0;

    // first char
    if ( !isalpha(str[str_ind]) )
        return {NULL, 0};

    str_ind++;

    // other chars
    while ( isalnum(str[str_ind]) || str[str_ind] == '_' )
        str_ind++;

    return {str, str_ind};
}

Token get_token( const char *str )
{
    assert(str);

    str = skip_spaces(str);
    if (!str)
        return {NULL, TKN_TYPE_ERROR};

    Token tkn = {};
    tkn.start = str;

    if ( isdigit(*str) )
    {
        if ( sscanf( str, "%f%n", &tkn.num, (int*) &tkn.len ) == 1 )
            tkn.type = TKN_TYPE_NUM;
        else
            tkn.type = TKN_TYPE_ERROR;

        return tkn;
    }
    else if ( (tkn.sep_char = get_sep_char( *str ) ) != SEP_FICTIONAL )
    {
        tkn.type = TKN_TYPE_SEP_CHAR;
        tkn.len = 1;

        return tkn;
    }
    else if ( ( tkn.keyword = get_keyword(str, &tkn.len) ) != KW_FICTIONAL )
    {
        tkn.type = TKN_TYPE_KEYWORD;

        return tkn;
    }
    else
    {
        Identificator id = get_identificator( str );
        if (id.start == NULL)
            return {NULL, TKN_TYPE_ERROR};

        tkn.type = TKN_TYPE_ID;
        tkn.len = id.len;
        tkn.id = id;

        return tkn;
    }

    ASSERT_UNREACHEABLE();
}

TreeNode *get_op(  CompiledProgram *comp_prog, const char *str )
{

}

TreeNode *get_root( CompiledProgram *comp_prog, const char *str )
{
    assert(comp_prog);
    assert(str);

    Token prog_start = get_token( str );
    SYN_ASSERT( prog_start.type == TKN_TYPE_KEYWORD && prog_start.keyword == KW_ProgStart,
                str, KEYWORDS[KW_ProgStart].str );
    str += prog_start.len;

    // first operator (obligatory)
    TreeNode *node_first_op = get_op( comp_prog, str );
    SYN_ASSERT( node_first_op, str, "Operator" );

    // other operators (optional)
    TreeNode *node_new_op   = NULL;
    TreeNode *node_curr_op  = NULL;
    while ( node_new_op = get_op( comp_prog, str ) )
    {
        if ( !is_node( node_curr_op, TREE_NODE_TYPE_OP, TREE_OP_SEQ_EXEC ) )
        {
            TreeNode *node_first_seq = new_node_op( &comp_prog->tree, TREE_OP_SEQ_EXEC );
            tree_hang_loose_node_at_left( &comp_prog->tree, node_curr_op, node_first_seq );
            node_curr_op = node_first_seq;
            node_first_op = node_first_seq;
        }

        TreeNode *node_new_seq = new_node_op( &comp_prog->tree, TREE_OP_SEQ_EXEC );
        tree_hang_loose_node_at_left( &comp_prog->tree, node_new_op, node_new_seq );
        tree_hang_loose_node_at_right( &comp_prog->tree, node_new_seq, node_curr_op );

        node_curr_op = node_new_seq;
    }

    tree_migrate_into_right( &comp_prog->tree, tree_get_parent(node_curr_op), node_new_op );

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
        fprintf(stream, "data_type: NUM, data_value: %d");
        break;
    case TREE_NODE_TYPE_OP:
        fprintf(stream, "data_type: OP, data_value: %d");
        break;
    case TREE_NODE_TYPE_ID:
        fprintf(stream, "data_type: ID, data_value: %d");
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
