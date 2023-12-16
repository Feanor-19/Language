#ifndef COMMON_H
#define COMMON_H

#include "..\..\..\..\mylibheaders\tree.h"
#include "logger.h"


typedef uint32_t op_t;
typedef uint32_t id_t;
typedef float num_t;


enum TreeNodeType
{
    TREE_NODE_TYPE_OP,
    TREE_NODE_TYPE_NUM,
    TREE_NODE_TYPE_ID,
};

struct TreeNodeData
{
    TreeNodeType type;
    union
    {
        op_t op;
        num_t num;
        id_t id;
    };

};



const size_t REALLOC_DEFAULT_MULTIPLIER = 2;
const double DBL_PRECISION              = 1E-10;



#define FREE(ptr) do{ if (ptr) free(ptr); ptr = NULL; }while(0)

#define ASSERT_UNREACHEABLE() assert( 0 && "Unreacheable line!" )

#define SIZEARR(arr) (sizeof(arr)/sizeof(arr[0]))


//! @brief Reads file with 'file_name' to dynamically allocated string
//! and returns it. If some error happens, returns NULL.
char *read_file_to_str( const char *file_name );

//! @brief Returns pointer to the first non-space symbol.
//! @note If '\0' is met, returns NULL!
const char *skip_spaces( const char *str );

//! @brief Returns new node of type operator and writes value 'op' into it.
TreeNode *new_node_op( Tree *tree_ptr, op_t op );

//! @brief Returns new node of type number and writes value 'num' into it.
TreeNode *new_node_num( Tree *tree_ptr, num_t num );

//! @brief Returns new node of type identificator and writes value 'id' into it.
TreeNode *new_node_id( Tree *tree_ptr, id_t id );

TreeNodeData get_node_data( TreeNode *node_ptr );

//! @brief Checks given node's type and value.
int is_node_op( TreeNode *node_ptr, op_t op );

//! @brief Checks given node's type and value.
int is_node_num( TreeNode *node_ptr, num_t num );

//! @brief Checks given node's type and value.
int is_node_id( TreeNode *node_ptr, id_t id );

void realloc_arr_if_needed( void **arr, size_t *arr_cap_ptr, size_t arr_ind, size_t elem_size );

#define REALLOC_ARR_WRP(arr, elem_t) do {                                               \
    realloc_arr_if_needed( (void**) &arr, &arr##_cap, arr##_curr_len, sizeof(elem_t) );      \
    if (!arr)                                                                           \
        ERROR("REALLOC ERROR: array named \"%s\", "                                     \
              "attempt to request %d elems of size %d. "                                \
              "Closing log automatically...",                                           \
              #arr, 2 * arr##_cap, sizeof(elem_t))                                      \
} while (0)

int is_dbl_zero( double a );

int are_dbls_equal( double a, double b );

#endif /* COMMON_H */
