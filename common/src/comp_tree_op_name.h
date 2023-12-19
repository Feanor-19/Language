#ifndef OPS_IN_TREE_H
#define OPS_IN_TREE_H

enum CompTreeOpName
{
    TREE_OP_SEQ_EXEC            = 1,
    TREE_OP_DUMMY               = 2,

    TREE_OP_ASSIGN              = 51,
    TREE_OP_CMP_MORE            = 52,
    TREE_OP_CMP_LESS            = 53,
    TREE_OP_CMP_MORE_EQ         = 54,
    TREE_OP_CMP_LESS_EQ         = 55,
    TREE_OP_CMP_EQUAL           = 56,
    TREE_OP_CMP_NOT_EQ          = 57,

    TREE_OP_ADD                 = 101,
    TREE_OP_SUB                 = 102,
    TREE_OP_MUL                 = 103,
    TREE_OP_DIV                 = 104,
    TREE_OP_PRE_PLUS_PLUS       = 105,
    TREE_OP_POST_PLUS_PLUS      = 106,
    TREE_OP_PRE_MINUS_MINUS     = 107,
    TREE_OP_POST_MINUS_MINUS    = 108,

    TREE_OP_IF                  = 201,
    TREE_OP_WHILE               = 202,
    TREE_OP_AND                 = 203,
    TREE_OP_OR                  = 204,
    TREE_OP_NOT                 = 205,

    TREE_OP_SIN                 = 301,
    TREE_OP_COS                 = 302,
    TREE_OP_FLOOR               = 303,
    TREE_OP_DIFF                = 304,
    TREE_OP_SQRT                = 305,
    TREE_OP_LN                  = 306,
    TREE_OP_EXP                 = 307,
    TREE_OP_MINUS               = 308,
};

#endif /* OPS_IN_TREE_H */
