#ifndef COMP_TREE_OP_ENUM_H
#define COMP_TREE_OP_ENUM_H

#define DEF_COMP_TREE_OP(id, name) TREE_OP_##name = id,
enum CompTreeOpNameEnum
{
    #include "comp_tree_ops_defs.h"
};
#undef DEF_COMP_TREE_OP

struct CompTreeOpName
{
    CompTreeOpNameEnum name;
    const char *str;
};

#define DEF_COMP_TREE_OP(id, name) { TREE_OP_##name, #name },
const CompTreeOpName COMP_TREE_OPS_NAMES[] =
{
    #include "comp_tree_ops_defs.h"
    { TREE_OP_DUMMY, "FICTIONAL!" }
};
#undef DEF_COMP_TREE_OP

#endif /* COMP_TREE_OP_ENUM_H */
