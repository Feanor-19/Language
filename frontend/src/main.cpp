#include <stdio.h>

#include "frontend.h"

int main( int argc, const char *argv[])
{
    Config cfg = get_config(argc, argv);
    if (cfg.error)
    {
        print_cfg_error_message( stderr, cfg.error );
        return cfg.error;
    }
    print_config(stdout, cfg);

    logger_init_log( "test_log.txt" );

    LOG("test1 %d", 0);

    WARNING("test2 %d", 19);

    ERROR("test3 %s", "hehehe");

    log_end();

    return 0;
}
