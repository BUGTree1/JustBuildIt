#define BUILDIT_DEBUG
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

int main() {
    BUILDIT_DEBUG_BOOL;
    
    execute_cmd({"mkdir", {"test"}});
    execute_cmd(get_compile_cmd(find_cxx_compiler(), {"src/buildit.cpp"}, {"test/test"}, {"src"}, true, OPTIMIZATION_SPEED, true, true));
    execute_cmd(get_compile_cmd(find_cxx_compiler(), {"src/helper.cpp"}, {"test/test2"}, {"src"}, true, OPTIMIZATION_SPEED, true, true));
    execute_cmd(get_link_cmd(find_cxx_linker(), "test/test", {"test/test", "test/test2"}, {}, {}, true, OPTIMIZATION_SPEED));
    return 0;
}
