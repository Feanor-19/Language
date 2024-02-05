#include <stdio.h>

#include "frontend.h"
#include "compiler_tree_dump.h"

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

    LOG( "Initializing image dumps folder..." );
    if (cfg.img_dumps_folder)
       init_img_dumps( cfg.img_dumps_folder );
    LOG( "Initializing image dumps folder is done!" );

    char *prog_str = read_file_to_str( cfg.input_file_name );
    if (!prog_str)
    {
        print_status_message( log_get_stream(), STATUS_ERROR_CANT_READ_INPUT_FILE );
        return STATUS_ERROR_CANT_READ_INPUT_FILE;
    }

    LOG( "Starting compilation..." );
    CompiledProgram compiled_prog = {};
    Status comp_err = compile_prog( prog_str, &compiled_prog );
    if (comp_err == STATUS_OK)
    {
        LOG( "Compilation is done!" );
    }

    if (!comp_err)
    {
        LOG( "Starting writing tree to file..." );
        write_tree_to_file( cfg.output_file_name, &compiled_prog.tree );
        LOG( "Writing tree to file is done!" );
    }

    CompiledProgram_dtor( &compiled_prog );

    close_img_dumps();

    LOG( "All work done, shutting down!" );

    log_end();

    return 0;
}
