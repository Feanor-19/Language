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

const char *skip_spaces( const char *str )
{
    assert(str);

    while ( isspace(*str) )
        str++;

    if ( *str != '\0' )
        return str;

    return NULL;
}

TreeNode *new_node_op( Tree *tree_ptr, node_cmn_type_t op )
{
    assert(tree_ptr);

    TreeNodeData data = { TREE_NODE_TYPE_OP, op };
    return op_new_TreeNode( tree_ptr, &data );
}

TreeNode *new_node_num( Tree *tree_ptr, node_cmn_type_t num )
{
    assert(tree_ptr);

    TreeNodeData data = { TREE_NODE_TYPE_NUM, num };
    return op_new_TreeNode( tree_ptr, &data );
}

TreeNode *new_node_id( Tree *tree_ptr, node_cmn_type_t id )
{
    assert(tree_ptr);

    TreeNodeData data = { TREE_NODE_TYPE_ID, id };
    return op_new_TreeNode( tree_ptr, &data );
}

TreeNodeData get_node_data( TreeNode *node_ptr )
{
    assert(node_ptr);

    return *((TreeNodeData*)node_ptr->data_ptr);
}

int is_node( TreeNode *node_ptr, TreeNodeType type, node_cmn_type_t value )
{
    assert(node_ptr);

    return get_node_data(node_ptr).type == type && get_node_data(node_ptr).value == value;
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
