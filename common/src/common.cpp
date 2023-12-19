#include "common.h"

#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>

inline off_t get_file_size(const char *file_name)
{
    assert(file_name);

    struct stat st_buf = {};

    if ( stat(file_name, &st_buf) == -1) return -1;

    return st_buf.st_size;
}

char *read_file_to_str( const char *file_name )
{
    assert(file_name);

    off_t file_size = get_file_size(file_name);
    if (file_size == -1)
        return NULL;

    FILE *file_p = fopen(file_name, "r");
    if (file_p == NULL)
        return NULL;

    char *str = (char *) calloc( file_size + 1, sizeof(char) );

    fread(str, sizeof(char), file_size, file_p);
    if ( ferror(file_p) != 0 )
    {
        free(str);
        return NULL;
    }

    fclose(file_p);

    return str;
}

inline void write_tree_node( FILE *stream, TreeNode *node )
{
    assert(stream);
    assert(node);

    putc('(', stream);

    putc(' ', stream);
    switch (get_node_data(node).type)
    {
    case TREE_NODE_TYPE_OP:
        putc('1', stream);
        fprintf(stream, " %d ", get_node_data(node).op);
        break;
    case TREE_NODE_TYPE_NUM:
        putc('2', stream);
        fprintf(stream, " %g ", get_node_data(node).num);
        break;
    case TREE_NODE_TYPE_ID:
        putc('3', stream);
        fprintf(stream, " %d ", get_node_data(node).id);
        break;
    default:
        ASSERT_UNREACHEABLE();
        break;
    }

    if ( tree_get_left_child( node ) )
        write_tree_node( stream, tree_get_left_child( node ) );
    else
        putc('_', stream);

    putc(' ', stream);

    if ( tree_get_right_child( node ) )
        write_tree_node( stream, tree_get_right_child( node ) );
    else
        putc('_', stream);

    putc(' ', stream);
    putc(')', stream);
}

int write_tree_to_file( const char *file_name, const Tree *tree_ptr )
{
    assert(file_name);
    assert(tree_ptr);

    FILE *file = fopen( file_name, "w" );
    if (!file)
        return 0;

    write_tree_node( file, tree_get_root( tree_ptr ) );

    fclose(file);

    return 1;
}

#define CHECK( char ) do{if ( getc(stream) != char ) return NULL;}while(0);

inline TreeNode *read_tree_node( FILE *stream, Tree *tree_ptr )
{
    assert(stream);
    assert(tree_ptr);

    CHECK( '(' );

    CHECK( ' ' );

    TreeNodeType type = (TreeNodeType) ( getc( stream ) - '0' );

    TreeNode *node      = NULL;
    CompTreeOpName op   = TREE_OP_DUMMY;
    num_t num           = 0;
    id_t id             = 0;
    switch (type)
    {
    case TREE_NODE_TYPE_OP:
        fscanf( stream, "%d", (int*) &op );
        node = new_node_op( tree_ptr, op );
        break;
    case TREE_NODE_TYPE_NUM:
        fscanf( stream, "%f", &num );
        node = new_node_num( tree_ptr, num );
        break;
    case TREE_NODE_TYPE_ID:
        fscanf( stream, "%d", &id );
        node = new_node_id( tree_ptr, id );
        break;
    default:
        ASSERT_UNREACHEABLE();
        break;
    }

    CHECK( ' ' );

    int c = 0;
    if ( (c = getc( stream ) ) != '_' )
    {
        ungetc( c, stream );
        TreeNode *node_left = read_tree_node( stream, tree_ptr );
        if ( !node_left )
            return NULL;
        tree_hang_loose_node_at_left( tree_ptr, node_left, node );
    }

    CHECK( ' ' );

    if ( (c = getc( stream ) ) != '_' )
    {
        ungetc( c, stream );
        TreeNode *node_right = read_tree_node( stream, tree_ptr );
        if (!node_right)
            return NULL;
        tree_hang_loose_node_at_right( tree_ptr, node_right, node );
    }

    CHECK( ' ' );

    CHECK( ')' );

    return node;
}

int read_tree_from_file( const char *file_name, Tree *tree_ptr )
{
    assert(file_name);
    assert(tree_ptr);

    FILE *file = fopen( file_name, "r" );
    if (!file)
        return 0;

    TreeNode *root = read_tree_node( file, tree_ptr );

    if( !root )
    {
        tree_dtor( tree_ptr );
        return 0;
    }

    tree_hang_loose_node_as_root( tree_ptr, root );

    fclose(file);

    return 1;
}

const char *skip_spaces( const char *str )
{
    assert(str);

    while ( isspace(*str) )
        str++;

    if ( *str != '\0' )
        return str;

    return NULL;
}

TreeNode *new_node_op( Tree *tree_ptr, CompTreeOpName op )
{
    assert(tree_ptr);

    TreeNodeData data = {};
    data.type = TREE_NODE_TYPE_OP;
    data.op = op;
    return op_new_TreeNode( tree_ptr, &data );
}

TreeNode *new_node_num( Tree *tree_ptr, num_t num )
{
    assert(tree_ptr);

    TreeNodeData data = {};
    data.type = TREE_NODE_TYPE_NUM;
    data.num = num;
    return op_new_TreeNode( tree_ptr, &data );
}

TreeNode *new_node_id( Tree *tree_ptr, id_t id )
{
    assert(tree_ptr);

    TreeNodeData data = {};
    data.type = TREE_NODE_TYPE_ID;
    data.id = id;
    return op_new_TreeNode( tree_ptr, &data );
}

TreeNodeData get_node_data( TreeNode *node_ptr )
{
    assert(node_ptr);

    return *((TreeNodeData*)node_ptr->data_ptr);
}

int is_node_op( TreeNode *node_ptr, CompTreeOpName op )
{
    assert(node_ptr);

    return get_node_data(node_ptr).type == TREE_NODE_TYPE_OP && get_node_data(node_ptr).op == op;
}

int is_node_num( TreeNode *node_ptr, num_t num )
{
    assert(node_ptr);

    return get_node_data(node_ptr).type == TREE_NODE_TYPE_NUM
        && are_dbls_equal(get_node_data(node_ptr).num, num);
}

int is_node_id( TreeNode *node_ptr, id_t id )
{
    assert(node_ptr);

    return get_node_data(node_ptr).type == TREE_NODE_TYPE_ID && get_node_data(node_ptr).id == id;
}

void realloc_arr_if_needed( void **arr_ptr, size_t *arr_cap_ptr, size_t arr_ind, size_t elem_size )
{
    if ( arr_ind >= *arr_cap_ptr )
    {
        size_t new_cap = REALLOC_DEFAULT_MULTIPLIER * (*arr_cap_ptr);
        void *new_mem = (void*) realloc( *arr_ptr, new_cap*elem_size );
        if (!new_mem)
        {
            free(*arr_ptr);
            *arr_ptr = NULL;
        }
        *arr_ptr = new_mem;
        *arr_cap_ptr = new_cap;
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

int is_dbl_zero( double a )
{
    return DBL_PRECISION <= a && a <= DBL_PRECISION;
}

int are_dbls_equal( double a, double b )
{
    return is_dbl_zero(a - b);
}

void put_n_chars( FILE *stream, char c, size_t n )
{
    assert(stream);

    while (n-- > 0)
    {
        putc( c, stream );
    }
}
