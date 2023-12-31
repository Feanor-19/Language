#include "compiler_tree_dump.h"

#include <assert.h>
#include <dirent.h>


static const char *FOLDER_PATH  = NULL;
static char *CMN_FILE_NAME      = NULL;
static char *CMN_CMD            = NULL;

static size_t FOLDER_PATH_LEN   = 0;
static size_t FILE_NAME_LEN     = 0;
static size_t EXTENSION_LEN     = 0;

static const char *CMD_PART_1 = "dot ";
static const char *CMD_PART_2 = " -T" DUMP_IMG_EXTENSION " -o ";

static const size_t CMD_PART_1_LEN = strlen( CMD_PART_1 );
static const size_t CMD_PART_2_LEN = strlen( CMD_PART_2 );

static int counter = 0;



inline void update_cmn_file_name( const char *extension )
{
    size_t digits_start_off = FILE_NAME_LEN - EXTENSION_LEN - 1 - DUMP_FILE_DIGITS_COUNT_AS_NUM;
    size_t extens_start_off = FILE_NAME_LEN - EXTENSION_LEN - 1;

    sprintf( CMN_FILE_NAME + digits_start_off, "%03d", counter );
    strcpy( CMN_FILE_NAME + extens_start_off, extension );
}

inline void assemble_cmn_file_name()
{
    FREE(CMN_FILE_NAME);

    CMN_FILE_NAME = (char *) calloc( FILE_NAME_LEN + 1, sizeof(char) );

    char *name_ptr = CMN_FILE_NAME;

    strcpy( name_ptr, FOLDER_PATH );
    name_ptr += FOLDER_PATH_LEN;

    if ( FOLDER_PATH[FOLDER_PATH_LEN - 1] != '\\' )
    {
        *name_ptr = '\\';
        name_ptr += 1;
    }

    sprintf( name_ptr, "%0" DUMP_FILE_DIGITS_COUNT_AS_STR "d", 0 );
    name_ptr += DUMP_FILE_DIGITS_COUNT_AS_NUM;

    *name_ptr = '.';
}

inline void assemble_cmn_cmd()
{
    FREE(CMN_CMD);

    CMN_CMD = (char*) calloc(  CMD_PART_1_LEN + 2*FILE_NAME_LEN + CMD_PART_2_LEN + 1, sizeof(char) );

    char *ptr = CMN_CMD;

    strcpy( ptr, CMD_PART_1 );
    ptr += CMD_PART_1_LEN;

    update_cmn_file_name( "." DUMP_DOT_EXTENSION );
    strcpy( ptr, CMN_FILE_NAME );
    ptr += FILE_NAME_LEN;

    strcpy( ptr, CMD_PART_2 );
    ptr += CMD_PART_2_LEN;

    update_cmn_file_name( "." DUMP_IMG_EXTENSION );
    strcpy( ptr, CMN_FILE_NAME );
}

void init_img_dumps( const char *folder_path )
{
    assert(folder_path);

    if (!FOLDER_PATH)
    {
        mkdir( folder_path );

        DIR *dir = opendir( folder_path );
        if ( !dir )
        {
            WARNING( "Can't create or open folder for image dumps. Image dumps won't be created!" );
            return;
        }

        closedir( dir );

        FOLDER_PATH = folder_path;
        FOLDER_PATH_LEN = strlen(FOLDER_PATH);
        EXTENSION_LEN = strlen(DUMP_DOT_EXTENSION);
        FILE_NAME_LEN = FOLDER_PATH_LEN + ( FOLDER_PATH[FOLDER_PATH_LEN - 1] != '\\' )
                        + DUMP_FILE_DIGITS_COUNT_AS_NUM + 1 + EXTENSION_LEN;

        assemble_cmn_file_name();

        assemble_cmn_cmd();
    }
    else
    {
        WARNING("Trying to reinit folder path for image dumps again. It's strange...");
    }
}

inline FILE *create_and_open_dot_file()
{
    if (FOLDER_PATH)
    {
        counter++;
        update_cmn_file_name( "." DUMP_DOT_EXTENSION );

        FILE *opened_file = fopen( CMN_FILE_NAME, "w" );
        if (!opened_file)
        {
            WARNING( "Can't create or open dot file for image dumps. Image dumps won't be created!" );
        }

        return opened_file;
    }

    return NULL;
}

inline void run_dot_to_create_database_img()
{
    size_t dot_file_digits_start = CMD_PART_1_LEN + FOLDER_PATH_LEN;
    sprintf( CMN_CMD + dot_file_digits_start, "%0" DUMP_FILE_DIGITS_COUNT_AS_STR "d", counter );
    CMN_CMD[dot_file_digits_start + DUMP_FILE_DIGITS_COUNT_AS_NUM] = '.';

    size_t img_file_digits_start = CMD_PART_1_LEN + FILE_NAME_LEN + CMD_PART_2_LEN + FOLDER_PATH_LEN;
    sprintf( CMN_CMD + img_file_digits_start, "%0" DUMP_FILE_DIGITS_COUNT_AS_STR "d", counter );
    CMN_CMD[img_file_digits_start + DUMP_FILE_DIGITS_COUNT_AS_NUM] = '.';

	system(CMN_CMD);
}

inline const char * find_tree_op_str_by_name( CompTreeOpNameEnum name )
{
    for (size_t ind = 0; ind < SIZEARR(COMP_TREE_OPS_NAMES); ind++)
    {
        if ( COMP_TREE_OPS_NAMES[ind].name == name )
            return COMP_TREE_OPS_NAMES[ind].str;
    }

    return NULL;
}

inline void write_dot_file( FILE *dot_file, Tree *tree_ptr )
{
	assert(dot_file);
	assert(tree_ptr);

#define COLOR_BG                "#2D4059"
#define COLOR_NODE_COLOR        "#ECC237"
#define COLOR_NUM_NODE_FILL     "#EA5455"
#define COLOR_ID_NODE_FILL      "#8ccb5e"
#define COLOR_OP_NODE_FILL      "#FFD460"
#define COLOR_LABEL_COLOR       "#EA5455"
#define COLOR_LABEL_FILL        "#ECC237"
#define COLOR_EDGE              "#F07B3F"


    // Dot header
    fprintf(dot_file,   "digraph{\n"
                        "splines=ortho;\n"
                        "bgcolor=\"" COLOR_BG "\";"
                        "\n\n\n");


    // Nodes with data
    TreeNode *curr_node = tree_ptr->head_of_all_nodes;
    TreeNode **nodes_arr = (TreeNode**) calloc( tree_ptr->nodes_count, sizeof(TreeNode) );
    for (size_t ind = 0; ind < tree_ptr->nodes_count; ind++)
    {
        if ( curr_node == NULL )
        {
            ERROR("ERROR DURING TREE DUMP: Something wrong with listing of nodes!\n");
            break;
        }

        TreeNodeData node_data = get_node_data(curr_node);
        if ( node_data.type == TREE_NODE_TYPE_NUM )
        {
            fprintf(dot_file,   "NODE_%llu[shape=\"record\", fontname=\"verdana\",\n"
                                "style=bold, style=filled,\ncolor=\"" COLOR_NODE_COLOR "\""
                                ", fillcolor=\"" COLOR_NUM_NODE_FILL "\",\n"
                                "label = %g];\n\n",
                                ind, node_data.num);
        }
        else if ( node_data.type == TREE_NODE_TYPE_OP )
        {
            fprintf(dot_file,   "NODE_%llu[shape=\"record\", fontname=\"verdana\",\n"
                                "style=bold, style=filled,\ncolor=\"" COLOR_NODE_COLOR "\""
                                ", fillcolor=\"" COLOR_OP_NODE_FILL "\",\n"
                                "label = %s];\n\n",
                                ind, find_tree_op_str_by_name((CompTreeOpNameEnum) node_data.op));
        }
        else if ( node_data.type == TREE_NODE_TYPE_ID )
        {
            fprintf(dot_file,   "NODE_%llu[shape=\"record\", fontname=\"verdana\",\n"
                                "style=bold, style=filled,\ncolor=\"" COLOR_NODE_COLOR "\""
                                ", fillcolor=\"" COLOR_ID_NODE_FILL "\",\n"
                                "label = %d];\n\n",
                                ind, node_data.id);
        }

        nodes_arr[ind] = curr_node;
        curr_node = curr_node->next;
    }


    // Edges
    for (size_t ind = 0; ind < tree_ptr->nodes_count; ind++)
    {
        size_t left = 0;
        size_t right = 0;
        for (size_t i = 0; i < tree_ptr->nodes_count; i++)
        {
            if ( nodes_arr[i] == nodes_arr[ind]->left )
            {
                left = i;
            }
            else if ( nodes_arr[i] == nodes_arr[ind]->right )
            {
                right = i;
            }
        }

        if ( nodes_arr[ind]->left )
        {
            fprintf(dot_file, "NODE_%llu->NODE_%llu"
							  "[color=\"" COLOR_EDGE "\", "
							  "penwidth=2];\n",
                              ind, left);
        }

        if ( nodes_arr[ind]->right )
        {
            fprintf(dot_file, "NODE_%llu->NODE_%llu[color=\"" COLOR_EDGE "\", "
							  "penwidth=2];\n", ind, right);
        }

        if ( nodes_arr[ind]->left && nodes_arr[ind]->right )
        {
            fprintf(dot_file,   "NODE_%llu->NODE_%llu[style=invis];\n"
                                "{rank=same NODE_%llu NODE_%llu}",
                                left, right, left, right);
        }
    }


    fprintf(dot_file, "\n}\n");

#undef COLOR_BG
#undef COLOR_NODE_COLOR
#undef COLOR_NUM_NODE_FILL
#undef COLOR_ID_NODE_FILL
#undef COLOR_OP_NODE_FILL
#undef COLOR_LABEL_COLOR
#undef COLOR_LABEL_FILL
#undef COLOR_EDGE

	free(nodes_arr);
}

inline void show_dump_img()
{
    update_cmn_file_name( "." DUMP_IMG_EXTENSION );
	system(CMN_FILE_NAME);
}

void dump_compiler_tree( Tree *comp_tree )
{
    assert(comp_tree);

    if ( !FOLDER_PATH )
        return;

    FILE *file = create_and_open_dot_file();

    write_dot_file(file, comp_tree);

    fclose( file );

    run_dot_to_create_database_img();

    show_dump_img();
}

void close_img_dumps( void )
{
    FREE(CMN_FILE_NAME);

    FREE(CMN_CMD);

    FOLDER_PATH = NULL;
}
