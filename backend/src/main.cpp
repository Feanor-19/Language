#include <stdio.h>

#include "backend.h"

int main(int argc, const char *argv[])
{
    Config cfg = get_config(argc, argv);

    if ( !init_log(cfg) )
    {
        print_status_message( stdout, STATUS_ERROR_CANT_INITIALIZE_LOG );
        return STATUS_ERROR_CANT_INITIALIZE_LOG;
    }

    if (cfg.error)
    {
        print_cfg_error_message( log_get_stream(), cfg.error );
        return cfg.error;
    }
    print_config(log_get_stream(), cfg);





    log_end();

    return 0;
}
