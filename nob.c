#define NOB_IMPLEMENTATION

#include"nob.h"

int main(int argc, char **argv){

    NOB_GO_REBUILD_URSELF(argc, argv);
    
    Nob_Cmd cmd ={0};

    nob_cmd_append(&cmd, "clang");
    nob_cc_flags(&cmd);

    nob_cc_output(&cmd, "./main");
    nob_cc_inputs(&cmd, "./main.c");
    nob_cmd_append(&cmd, "-lraylib");    
    nob_cmd_append(&cmd, "-lm");    

    if (!nob_cmd_run_sync(cmd)) return 1;
    // Reset the cmd array so you can use it again for another command
    cmd.count = 0;

    
    return 0;
}






