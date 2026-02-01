#include "buildit.h"

using namespace std;

namespace buildit {

int execute_cmd(Command cmd, vector<Process>* async){
    fs::path absolute_executable = canonize_path(cmd.executable, true);
    
    fs::path absolute_stdin_file = canonize_path(cmd.stdin_file, false, true);
    if (absolute_stdin_file != "" && !fs::exists(absolute_stdin_file)) error("stdin file: " + ("\"" + cmd.stdin_file.string() + "\"") + " for command: " + ("\"" + cmd.executable.string() + concat_str_vec(cmd.arguments, " ") + "\"") + " doesent exist!");
    
    fs::path absolute_stdout_file = canonize_path(cmd.stdout_file, false, true);
    fs::path absolute_stderr_file = canonize_path(cmd.stderr_file, false, true);
    
    fs::path absolute_working_dir = canonize_path(cmd.working_dir);
    fs::path previous_working_dir = fs::current_path();
    fs::current_path(absolute_working_dir);
    
    Process created_process = {};
    
    #ifdef BUILDIT_OS_WINDOWS
    
    // TODO: rewrite in UCRT
    todo("WINDOWS UCRT execute_cmd");
    
    #else // BUILDIT_OS_WINDOWS
        
    string absolute_executable_string = absolute_executable.string();
    char** executable_args = str_vec_to_cstr_arr(concat_vec(vector<string>({absolute_executable_string}), cmd.arguments));
    pid_t pid = fork();
    ASSERT_ERRNO(pid != -1);
    if(pid == 0) {
        set_fd_as_file(absolute_stdin_file,  STDIN_FILENO,  O_RDONLY);
        set_fd_as_file(absolute_stdout_file, STDOUT_FILENO, O_WRONLY | O_CREAT | O_TRUNC);
        set_fd_as_file(absolute_stderr_file, STDERR_FILENO, O_WRONLY | O_CREAT | O_TRUNC);
        ASSERT_ERRNO(execv(absolute_executable_string.c_str(), executable_args) != -1);
        exit(0);
    }
    
    created_process = {executable_args, pid};
    
    #endif // BUILDIT_OS_WINDOWS
    
    fs::current_path(previous_working_dir);
        
    if(async == NULL){
        return wait_for_process(created_process);
    } else {
        (*async).push_back(created_process);
    }
    
    return 0;
}
vector<int> execute_cmds(vector<Command> cmds, vector<Process>* async){
    vector<int> return_vec = vector<int>();
    for(size_t i = 0; i < cmds.size(); i++) {
        return_vec.push_back(execute_cmd(cmds[i], async));
    }
    return return_vec;
}
int wait_for_process(Process process){
    #ifdef BUILDIT_OS_WINDOWS
    DWORD exit_code = 0;
    
    ASSERT_WINAPI(WaitForSingleObject(process.pi.hProcess, INFINITE) != WAIT_FAILED);
    ASSERT_WINAPI(GetExitCodeProcess(process.pi.hProcess, &exit_code));

    CloseHandle(process.pi.hProcess);
    CloseHandle(process.pi.hThread);
    
    return exit_code;
    
    #else // BUILDIT_OS_WINDOWS
    
    int child_status = 0;
    ASSERT_ERRNO(waitpid(process.pid, &child_status, 0) != -1);
    free_cstr_arr(process.executable_args);
    return WEXITSTATUS(child_status);
    
    #endif // BUILDIT_OS_WINDOWS
}
vector<int> wait_for_processes(vector<Process> processes){
    vector<int> vec = vector<int>();
    for(size_t i = 0; i < processes.size(); i++) {
        vec.push_back(wait_for_process(processes[i]));
    }
    return vec;
}
vector<int> chain_commands(vector<Command> cmds, vector<Process>* async) {
    if(cmds.size() < 2) {
        return vector<int>({execute_cmd(cmds[0], async)});
    }
    
    fs::path absolute_stdin_file = canonize_path(cmds[0].stdin_file, false, true);
    if (absolute_stdin_file != "" && !fs::exists(absolute_stdin_file)) error("stdin file: " + ("\"" + cmds[0].stdin_file.string() + "\"") + " doesent exist!");
    
    fs::path absolute_stdout_file = canonize_path(cmds[cmds.size() - 1].stdout_file, false, true);
    
    vector<int> exit_codes = vector<int>();
    
    #ifdef BUILDIT_OS_WINDOWS
    
    todo("chain_commands WINDOWS");
    return vector<int>();
    
    #else // BUILDIT_OS_WINDOWS
    
    vector<Process> sync_vec = vector<Process>();
    
    int pipes[2][cmds.size() - 1];
    for(size_t i = 0; i < cmds.size() - 1; i++) {
        ASSERT_ERRNO(pipe(pipes[i]) != -1);
    }
    
    for(size_t i = 0; i < cmds.size(); i++) {
        fs::path absolute_executable = cmds[i].executable;
        if(!absolute_executable.is_absolute()) {
            absolute_executable = find_in_env_path(absolute_executable);
        }
        absolute_executable = (fs::weakly_canonical(absolute_executable)).make_preferred();
        
        string absolute_executable_string = absolute_executable.string();
        char** executable_args = str_vec_to_cstr_arr(concat_vec(vector<string>({absolute_executable_string}), cmds[i].arguments));
        
        pid_t pid = fork();
        ASSERT_ERRNO(pid != -1);
        if(pid == 0) {
            fs::path absolute_working_dir = canonize_path(cmds[i].working_dir);
            fs::current_path(absolute_working_dir);
            if(i > 0) {
                size_t pipe_idx = i - 1;
                ASSERT_ERRNO(close(pipes[pipe_idx][1]) != -1);              //closing pipe write
                ASSERT_ERRNO(dup2(pipes[pipe_idx][0], STDIN_FILENO) != -1); //replacing stdin with pipe read
                ASSERT_ERRNO(close(pipes[pipe_idx][0]) != -1);              //closing pipe read
            }else{
                set_fd_as_file(absolute_stdin_file, STDIN_FILENO, O_RDONLY);
            }
            if(i + 1 < cmds.size()) {
                size_t pipe_idx = i;
                ASSERT_ERRNO(close(pipes[pipe_idx][0]) != -1);               //closing pipe read
                ASSERT_ERRNO(dup2(pipes[pipe_idx][1], STDOUT_FILENO) != -1); //replacing stdout with pipe write 
                ASSERT_ERRNO(close(pipes[pipe_idx][1]) != -1);               //closing pipe write
            }else{
                set_fd_as_file(absolute_stdout_file, STDOUT_FILENO, O_WRONLY | O_CREAT | O_TRUNC);
            }
            for(size_t c = 0; c < cmds.size() - 1; c++) {
                if(c == i || c == i - 1) continue;
                for(size_t p = 0; p < 2; p++) {
                    ASSERT_ERRNO(close(pipes[c][p]) != -1);
                }
            }
            
            ASSERT_ERRNO(execv(absolute_executable_string.c_str(), executable_args));
            exit(0);
        }
        
        if(async == NULL){
            sync_vec.push_back({executable_args, pid});
        }else{
            (*async).push_back({executable_args, pid});
        }
    }
    
    for(size_t i = 0; i < cmds.size() - 1; i++) {
        for(size_t j = 0; j < 2; j++) {
            ASSERT_ERRNO(close(pipes[i][j]) != -1);
        }
    }
    
    if(async == NULL) {
        exit_codes = wait_for_processes(sync_vec);
    }
    
    #endif // BUILDIT_OS_WINDOWS
    
    return exit_codes;
}

Command get_compile_cmd(fs::path compiler, std::vector<fs::path> source_files, std::vector<fs::path> include_dirs, bool all_warnings, bool pedantic) {
    todo("get_compile_cmd");
    return {};
}
Command get_link_cmd(fs::path linker, std::vector<fs::path> objects, Optimization_Level optimize, bool native_arc) {
    todo("get_link_cmd");
    return {};
}

void create_vscode_settings(std::string cmd, fs::path dir) {
    todo("create_vscode_settings");
}
void create_clangd_settings(std::string cmd, fs::path dir) {
    todo("create_clangd_settings");
}

bool vec_any_nonzero(std::vector<int> vec) {
    bool nonzero = false;
    for(size_t i = 0; i < vec.size(); i++) {
        nonzero = nonzero || (vec[i] != 0);
    }
    return nonzero;
}

fs::path find_c_compiler(vector<fs::path> compilers){
    if(compilers.size() == 0) {
        compilers = vector<fs::path>({"gcc", "clang", "cl.exe"});
    }
    return find_first_in_env_path(compilers);
}
fs::path find_cxx_compiler(vector<fs::path> compilers){
    if(compilers.size() == 0) {
        compilers = vector<fs::path>({"g++", "clang++", "cl.exe"});
    }
    return find_first_in_env_path(compilers);
}

fs::path find_c_linker(vector<fs::path> linkers){
    return find_c_compiler(linkers);
}
fs::path find_cxx_linker(vector<fs::path> linkers){
    return find_cxx_compiler(linkers);
}
fs::path find_linker(vector<fs::path> linkers){
    if(linkers.size() == 0) {
        linkers = vector<fs::path>({"ld", "lld", "link.exe"});
    }
    return find_first_in_env_path(linkers);
}

string concat_str_vec(vector<string> vec, string delim){
    string str = "";
    for(size_t i = 0; i < vec.size(); i++) {
        if(i > 0) str += delim;
        str += vec[i];
    }
    return str;
}
vector<string> split_string(string str, string delim){
    vector<string> vec = vector<string>();
    size_t pos = 0;
    size_t pos_prev = 0;
    while (pos != string::npos) {
        pos = str.find(delim, pos_prev);
        vec.push_back(str.substr(pos_prev, pos - pos_prev));
        pos_prev = pos + delim.size();
    }
    return vec;
}
char** str_vec_to_cstr_arr(vector<string> vec){
    char** cstr_arr = (char**)malloc((vec.size() + 1) * sizeof(char**));
    for (size_t i = 0; i < vec.size(); i++){
        size_t cstr_size = vec[i].size() * sizeof(char);
        char* cstr = (char*)malloc(cstr_size + (1 * sizeof(char)));
        memcpy((void*)cstr, (void*)&vec[i][0], cstr_size);
        cstr[cstr_size] = 0;
        cstr_arr[i] = cstr;
    }
    cstr_arr[vec.size()] = 0;
    return cstr_arr;
}
void free_cstr_arr(char** cstr_arr){
    size_t len = 0;
    while(cstr_arr[len] != 0) { len++; };
    for(size_t i = 0; i < len; i++) {
        free(cstr_arr[i]);
    }
    free(cstr_arr);
}

fs::path find_in_env_path(fs::path filename){
    vector<string> path_dirs = split_string(getenv("PATH"), BUILDIT_ENV_PATH_SEPARATOR);
    for(size_t i = 0; i < path_dirs.size(); i++) {
        for(fs::directory_entry const& file : fs::directory_iterator(path_dirs[i])){
            if(file.exists() && !file.is_directory()) {
                if(file.path().filename() == filename) return file.path();
            }
        }
    }
    return "";
}
fs::path find_first_in_env_path(vector<fs::path> filenames){
    for(size_t i = 0; i < filenames.size(); i++){
        fs::path path = find_in_env_path(filenames[i]);
        if(path != "") {
            return path;
        }
    }
    return "";
}
fs::path canonize_path(fs::path path, bool executable, bool check_empty, bool exec_base_dir) {
    if(check_empty && path == "") return path;
    fs::path absolute_path = "";
    if(!path.is_absolute()) {
        if(executable) {
            absolute_path = find_in_env_path(path);
        }else{
            if(exec_base_dir){
                absolute_path = get_current_executable_dir() / path;
            }else{
                absolute_path = fs::current_path() / path;
            }
        }
    } else {
        absolute_path = path;
    }
    return (fs::weakly_canonical(absolute_path)).make_preferred();
}
fs::path get_system_shell(){
    vector<fs::path> shells = vector<fs::path>({"bash", "cmd.exe", "zsh", "fish", "dash", "tcsh", "csh", "ksh", "sh"});
    return find_first_in_env_path(shells);
}
fs::path get_current_executable_path() {
    #ifdef BUILDIT_OS_WINDOWS
        wchar_t path[FILENAME_MAX] = { 0 };
        GetModuleFileNameW(nullptr, path, FILENAME_MAX);
        return fs::path(path);
    #else
        char path[FILENAME_MAX];
        ssize_t count = readlink("/proc/self/exe", path, FILENAME_MAX);
        return fs::path(string(path, count));
    #endif
}
fs::path get_current_executable_dir() {
    fs::path exec_path = get_current_executable_path();
    if(!exec_path.has_parent_path()) error("wtf your executable path doesnt have a parent (directory)");
    return exec_path.parent_path();
}

void error(string desc, int code){
    cerr << "ERROR: " << desc << endl;
    exit(code);
}
void warning(string desc){
    cout << "WARNING: " << desc << endl;
}
void todo(string desc){
    cerr << "TODO: " << desc << endl;
    exit(0);
}

#ifdef BUILDIT_OS_WINDOWS

#else
void set_fd_as_file(fs::path file, int fd, int open_flags, int create_file_permissions) {
    if(file != "") {
        ASSERT_ERRNO(fd != -1);
        int file_fd = open(file.c_str(), open_flags, create_file_permissions);
        ASSERT_ERRNO(file_fd != -1);
        ASSERT_ERRNO(dup2(file_fd, fd) != -1);
        ASSERT_ERRNO(close(file_fd) != -1);
    }
}
#endif

}
