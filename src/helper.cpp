#include "buildit.h"

using namespace std;
using namespace buildit;

// TODO: HERE ALSO SHOULD BE THE TEMPLATING
// TODO: HERE ALSO SHOULD BE THE GIT HELPER

#ifdef BUILDIT_OS_WINDOWS
#define PWD_TEST_CMD "echo RUNNING IN DIR %cd%"
#define PIPE_TEST_CMD "findstr \"^\""
#else 
#define PWD_TEST_CMD "echo RUNNING IN DIR $PWD"
#define PIPE_TEST_CMD "cat"
#endif

#define EXECUTE_IN_SHELL(cmd) execute_cmd({get_system_shell(), {BUILDIT_OS_ARG_CHAR "c", (cmd)}})

int main() {
    create_clangd_settings(".", true, {"XPP", "ABC", "696969"}, 23, true, true);
    execute_cmd(get_compile_cmd(find_compiler(true), {"../src/buildit.cpp"}, "test", {"../src"}, true, OPTIMIZATION_SPEED, true, 17, true, true, true));
    execute_cmd(get_compile_cmd(find_compiler(true), {"../src/helper.cpp"}, "test2", {"../src"}, true, OPTIMIZATION_SPEED, true, 17, true, true, true));
    execute_cmd(get_link_cmd(find_compiler(true), "test", {"test", "test2"}, {}, {}, true, OPTIMIZATION_SPEED, true));
    EXECUTE_IN_SHELL("echo DONE!");
    return 0;
}
