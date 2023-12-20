#include <stdio.h>

#include "backend.h"
#include "compiler_tree_dump.h"

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

    LOG( "Reading compiler tree from the input file..." );
    Tree compiler_tree = {};
    tree_ctor( &compiler_tree, sizeof( TreeNodeData ), NULL, print_tree_node_data );
    if (!read_tree_from_file( cfg.input_file_name, &compiler_tree ))
    {
        ERROR( "Something went wrong during reading the input file! Shutting down..." );
        tree_dtor( &compiler_tree );
        log_end();
        return STATUS_ERROR_CANT_READ_INPUT_FILE;
    }
    LOG( "Reading compiler tree from the input file is done!" );

    if ( cfg.img_dumps_folder )
    {
        LOG( "Because config includes image dumps folder, creating image dump of the input compiler tree..." );
        init_img_dumps( cfg.img_dumps_folder );
        dump_compiler_tree( &compiler_tree );
        LOG( "Compiler tree image dump is created." );
    }

    // REVIEW - вот вроде и одна строчка, а поэтому не хочется выносить в отдельную ф-ю
    // но и вроде отдельное конкретное действие...
    FILE *out_stream = fopen( cfg.output_file_name, "w" );
    if (!out_stream)
    {
        ERROR( "Can't open output file. Shutting down..." );
        tree_dtor( &compiler_tree );
        close_img_dumps();
        log_end();
        return STATUS_ERROR_CANT_OPEN_OUTPUT_FILE;
    }

    LOG( "Starting translation to text asm..." );
    Status err = translate_to_asm_text( &compiler_tree, out_stream );
    if (err)
    {
        ERROR( "Some error happened during translation:" );
        print_status_message( log_get_stream(), err );
        putc( '\n', log_get_stream() );
        ERROR( "Shutting down." );
    }
    else
        LOG( "Translation to text asm is done!" );

    FCLOSE(out_stream);
    tree_dtor( &compiler_tree );
    close_img_dumps();
    log_end();

    return 0;
}
