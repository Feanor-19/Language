#ifndef MIDDLEEND_H
#define MIDDLEEND_H


// TODO - вторая попытка разобраться с этим
#include ".\..\..\common\src\common.h"
#include "config_middle.h"


#define DEF_STATUS(name, msg) STATUS_##name,
enum Status
{
    #include "middle_statuses.h"
};
#undef DEF_STATUS


#define DEF_STATUS(name, message) message,
const char * const status_messages[] =
{
    #include "middle_statuses.h"
    "FICTIONAL MESSAGE!"
};
#undef DEF_STATUS


void print_status_message( FILE *stream, Status status );

int init_log( Config cfg );

#endif /* MIDDLEEND_H */
