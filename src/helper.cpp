#define BUILDIT_LOG
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
    BUILDIT_LOG_BOOL;
    
    vector<fs::path> obj_vec = vector<fs::path>();
    execute_cmd(get_compile_cmd(find_cxx_compiler(), {"test/test.cpp"}, &obj_vec, {"test"}, OPTIMIZATION_SPEED, true, true));
    execute_cmd(get_link_cmd(find_cxx_linker(), obj_vec, OPTIMIZATION_SPEED, true));
    return 0;
}
