#ifndef COMMON_H
#define COMMON_H

#include "..\..\..\..\mylibheaders\tree.h"
#include "logger.h"
#include "comp_tree_op_name.h"


typedef int32_t id_t;
#define ABSENT_ID ((id_t) -1)
typedef float num_t;


enum TreeNodeType
{
    TREE_NODE_TYPE_OP   = 1,
    TREE_NODE_TYPE_NUM  = 2,
    TREE_NODE_TYPE_ID   = 3,
};

struct TreeNodeData
{
    TreeNodeType type;
    union
    {
        CompTreeOpName op;
        num_t num;
        id_t id;
    };

};

enum FuncType
{
    FUNC_TYPE_RECIPE,
    FUNC_TYPE_ACTION,
};



const size_t REALLOC_DEFAULT_MULTIPLIER = 2;
const double DBL_PRECISION              = 1E-10;



#define FREE(ptr) do{ if (ptr) free(ptr); ptr = NULL; }while(0)

#define ASSERT_UNREACHEABLE() assert( 0 && "Unreacheable line!" )

#define SIZEARR(arr) (sizeof(arr)/sizeof(arr[0]))

#define FCLOSE(file) do{if (file) fclose(file); }while(0)


//! @brief Reads file with 'file_name' to dynamically allocated string
//! and returns it. If some error happens, returns NULL.
char *read_file_to_str( const char *file_name );

//! @brief Writes given tree to the given file. Returns 1 if
//! all's ok, 0 if some error happens.
int write_tree_to_file( const char *file_name, const Tree *tree_ptr );

//! @brief Reads tree from the given file. Returns 1 if
//! all's ok, 0 if some error happens.
//! @note Given tree is NOT ctored() by this func, YOU NEED to do it
//! BEFORE calling this function.
int read_tree_from_file( const char *file_name, Tree *tree_ptr );

//! @brief Returns new node of type operator and writes value 'op' into it.
TreeNode *new_node_op( Tree *tree_ptr, CompTreeOpName op );

//! @brief Returns new node of type number and writes value 'num' into it.
TreeNode *new_node_num( Tree *tree_ptr, num_t num );

//! @brief Returns new node of type identificator and writes value 'id' into it.
TreeNode *new_node_id( Tree *tree_ptr, id_t id );

TreeNodeData get_node_data( TreeNode *node_ptr );

//! @brief Checks given node's type and value.
int is_node_op( TreeNode *node_ptr, CompTreeOpName op );

//! @brief Checks given node's type and value.
int is_node_num( TreeNode *node_ptr, num_t num );

//! @brief Checks given node's type and value.
int is_node_id( TreeNode *node_ptr, id_t id );

void realloc_arr_if_needed( void **arr, size_t *arr_cap_ptr, size_t arr_ind, size_t elem_size );

#define REALLOC_ARR_WRP(arr, elem_t) do {                                                       \
    realloc_arr_if_needed( (void**) &(arr), &(arr##_cap), (arr##_curr_len), sizeof(elem_t) );   \
    if (!(arr))                                                                                 \
        ERROR("REALLOC ERROR: array named \"%s\", "                                             \
              "attempt to request %d elems of size %d.",                                        \
              #arr, 2 * (arr##_cap), sizeof(elem_t));                                           \
} while (0)

void print_tree_node_data( FILE *stream, void *data_ptr );

int is_dbl_zero( double a );

int are_dbls_equal( double a, double b );

void put_n_chars( FILE *stream, char c, size_t n );

#endif /* COMMON_H */
