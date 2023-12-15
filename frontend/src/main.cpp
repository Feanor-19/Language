#include <stdio.h>

#include "frontend.h"

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


    char *prog_str = read_file_to_str( cfg.input_file_name );
    if (!prog_str)
    {
        print_status_message( log_get_stream(), STATUS_ERROR_CANT_READ_INPUT_FILE );
        return STATUS_ERROR_CANT_READ_INPUT_FILE;
    }

    // DEBUG
    printf("<%s>\n", prog_str);

    const char *str = "The Following Prepared Beforehand: blah blah blah";
    Token tkn = get_token( str );
    if ( tkn.type == TKN_TYPE_KEYWORD )
    {
        printf( "Found keyword: <%s>\n", KEYWORDS[tkn.keyword].str );
        printf( "<%s>\n", str + tkn.len );
    }
    else
    {
        printf( "not found keyword :(\n");
    }
    // DEBUG

    CompiledProgram compiled_prog = compile_prog( prog_str );

    // write into file(s)

    CompiledProgram_dtor( &compiled_prog );

    log_end();

    return 0;
}
