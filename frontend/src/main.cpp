#include <stdio.h>

#include "frontend.h"
#include ".\..\..\common\src\compiler_tree_dump.h"

int main( int argc, const char *argv[])
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


    if (cfg.img_dumps_folder)
       init_img_dumps( cfg.img_dumps_folder );

    char *prog_str = read_file_to_str( cfg.input_file_name );
    if (!prog_str)
    {
        print_status_message( log_get_stream(), STATUS_ERROR_CANT_READ_INPUT_FILE );
        return STATUS_ERROR_CANT_READ_INPUT_FILE;
    }

    CompiledProgram compiled_prog = compile_prog( prog_str );

    // write into file(s)

    CompiledProgram_dtor( &compiled_prog );

    log_end();

    return 0;
}
