#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX

#include"nob.h"

#define BUILD_FOLDER "build/"
#define SRC_FOLDER "src/"

int main(int argc, char **argv){

    NOB_GO_REBUILD_URSELF(argc, argv);
    
    Nob_Cmd cmd ={0};
    
    if (!mkdir_if_not_exists(BUILD_FOLDER)) return 1;

    nob_cmd_append(&cmd, "clang");
    nob_cc_flags(&cmd);

    nob_cc_output(&cmd, BUILD_FOLDER"libmuzk.so");
    nob_cmd_append(&cmd, "-fPIC");    
    nob_cmd_append(&cmd, "-shared");    
    nob_cc_inputs(&cmd, SRC_FOLDER"muzk.c");
    nob_cmd_append(&cmd, "-lraylib");    
    nob_cmd_append(&cmd, "-lm");    

    if (!nob_cmd_run_sync(cmd)) return 1;
    // Reset the cmd array so you can use it again for another command
    cmd.count = 0;

    nob_cmd_append(&cmd, "clang");
    nob_cc_flags(&cmd);

    nob_cc_output(&cmd, BUILD_FOLDER"muvi");
    nob_cmd_append(&cmd, SRC_FOLDER"muzk.c");    
    nob_cc_inputs(&cmd, SRC_FOLDER"main.c");
    nob_cmd_append(&cmd, "-lraylib");    
    nob_cmd_append(&cmd, "-lm");    

    if (!nob_cmd_run_sync(cmd)) return 1;
    // Reset the cmd array so you can use it again for another command
    cmd.count = 0;

    
    return 0;
}






