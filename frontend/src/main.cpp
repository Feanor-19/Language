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

    return 0;
}
