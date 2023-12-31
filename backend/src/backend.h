#ifndef BACKEND_H
#define BACKEND_H

#include "config_back.h"
#include "tr_asm_text_funcs.h"
#include "backend_common.h"


struct CompTreeOpBackend
{
    CompTreeOpNameEnum name;
    Status (*tr_asm_text)(FILE*,const Tree*,TreeNode*,Counters*,Context*,FuncFrames*);
};


#define DEF_STATUS(name, message) message,
const char * const status_messages[] =
{
    #include "back_statuses.h"
    "FICTIONAL MESSAGE!"
};
#undef DEF_STATUS

// REVIEW - ???
#define DEF_CMD(name, id, is_im_const, is_reg, is_mem, is_label, ...) CMD_##name = id,
enum Command
{
    #include ".\..\..\..\maybe_calculator\common\commands.h"
};
#undef DEF_CMD

#define DEF_CMD(name, id, is_im_const, is_reg, is_mem, is_label, ...) #name,
const char * const commands_list[] =
{
    #include ".\..\..\..\maybe_calculator\common\commands.h"
    "FICTIONAL_CMD!"
};
#undef DEF_CMD
// REVIEW - ???

const size_t DUMMY_OP_INDEX       = 0;
const size_t FUNCS_DEFAULT_NUMBER = 1;
const size_t MEMORY_SIZE          = 1024;

const CompTreeOpBackend COMP_TREE_OPS_BACKEND[] =
{
    { TREE_OP_DUMMY,                    tr_asm_text_dummy       },
    { TREE_OP_SEQ_EXEC,                 tr_asm_text_seq_exec    },
    { TREE_OP_FUNC_DEF,                 tr_asm_text_func_def    },
    { TREE_OP_MAIN_PROG,                tr_asm_text_main_prog   },

    { TREE_OP_ASSIGN,                   tr_asm_text_assign      },
    { TREE_OP_CMP_MORE,                 tr_asm_text_cmp_more    },
    { TREE_OP_CMP_LESS,                 tr_asm_text_cmp_less    },
    { TREE_OP_CMP_MORE_EQ,              tr_asm_text_cmp_more_eq },
    { TREE_OP_CMP_LESS_EQ,              tr_asm_text_cmp_less_eq },
    { TREE_OP_CMP_EQUAL,                tr_asm_text_cmp_equal   },
    { TREE_OP_CMP_NOT_EQ,               tr_asm_text_cmp_not_eq  },

    { TREE_OP_ADD,                      tr_asm_text_add         },
    { TREE_OP_SUB,                      tr_asm_text_sub         },
    { TREE_OP_MUL,                      tr_asm_text_mul         },
    { TREE_OP_DIV,                      tr_asm_text_div         },

    { TREE_OP_IF,                       tr_asm_text_if          },
    { TREE_OP_WHILE,                    tr_asm_text_while       },
    { TREE_OP_AND,                      tr_asm_text_and         },
    { TREE_OP_OR,                       tr_asm_text_or          },
    { TREE_OP_NOT,                      tr_asm_text_not         },

    { TREE_OP_SIN,                      tr_asm_text_sin         },
    { TREE_OP_COS,                      tr_asm_text_cos         },
    { TREE_OP_SQRT,                     tr_asm_text_sqrt        },
    { TREE_OP_LN,                       tr_asm_text_ln          },
    { TREE_OP_EXP,                      tr_asm_text_exp         },
    { TREE_OP_MINUS,                    tr_asm_text_minus       },

    { TREE_OP_CALL_FUNC,                tr_asm_text_call_func   },
    { TREE_OP_RETURN,                   tr_asm_text_return      },
    { TREE_OP_INPUT,                    tr_asm_text_input       },
    { TREE_OP_PRINT_NUM,                tr_asm_text_print_num   },
    { TREE_OP_PRINT_CHAR,               tr_asm_text_print_char  }
};

Status tr_node_asm_text( FILE *stream, const Tree *tree_ptr,
                         TreeNode *node, Counters *counters,
                         Context *context, FuncFrames *frames );

Status translate_to_asm_text( const Tree *tree_ptr, FILE *stream );

void print_status_message( FILE *stream, Status status );

int init_log( Config cfg );

#endif /* BACKEND_H */
