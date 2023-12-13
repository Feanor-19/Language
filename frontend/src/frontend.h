#ifndef FRONTEND_H
#define FRONTEND_H

// TODO - вторая попытка разобраться с этим
#include ".\..\..\common\src\common.h"
#include "config_front.h"


#define DEF_STATUS(name, msg) STATUS_##name,
enum Status
{
    #include "front_statuses.h"
};
#undef DEF_STATUS


#define DEF_STATUS(name, message) message,
const char * const status_messages[] =
{
    #include "front_statuses.h"
    "FICTIONAL MESSAGE!"
};
#undef DEF_STATUS


void print_status_message( FILE *stream, Status status );

int init_log( Config cfg );

#endif /* FRONTEND_H */
