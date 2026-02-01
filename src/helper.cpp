#include "buildit.h"

using namespace std;
using namespace buildit;

// TODO: HERE ALSO SHOULD BE THE TEMPLATING
// TODO: HERE ALSO SHOULD BE THE GIT HELPER

int main() {
    //cout << "SHELL: " << get_system_shell() << endl;
    //execute_cmd({"cowsay", {"ABOBA"}});
    execute_cmd({get_system_shell(), {BUILDIT_OS_ARG_CHAR"c", "echo RUNNING IN DIR $PWD"}, "bin"});
    //cout << "SHELL EXIT 69: " << execute_cmd({get_system_shell(), {BUILDIT_OS_ARG_CHAR"c", "exit 69"}}) << endl;
    //vector<Process> processes = vector<Process>();
    //execute_cmds({{"cowsay", {"ABOBA"}}, {"cowsay", {"XPP"}}, {get_system_shell(), {BUILDIT_OS_ARG_CHAR"c", "exit 69"}}}, &processes);
    //cout << (vec_any_nonzero(wait_for_processes(processes)) ? "FAILED" : "SUCCESS") << endl;
    //execute_cmd({"touch", {"XPP.txt"}, "bin"});
    //vector<int> returns = chain_commands({{"echo", {"ABOBA"}}, {"cat", {}}});
    //cout << (vec_any_nonzero(returns) ? "FAILED" : "SUCCESS") << endl;
    //chain_commands({{"bash", {"-c", "echo RUNNING IN DIR $PWD"}, "bin"}, {"cat", {}}});
    //execute_cmd({"cat", {}, "", "stdin.txt", "stdout.txt", "stderr.txt"});
    //chain_commands({{"cat", {}, "", "stdin.txt"}, {"cat", {}, "", "", "stdout.txt"}});
    return 0;
}
