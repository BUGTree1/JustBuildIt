#include "buildit.h"

using namespace std;

namespace buildit {

int execute_cmd(Command cmd, vector<Process>* async){
    fs::path abs_executable = canonize_path(cmd.executable, true);
    if (abs_executable == fs::path("")) error("Could not find executable: " + cmd.executable.string() + " !");
    
    fs::path abs_stdin_file  = canonize_path(cmd.stdin_file, false, true);
    if (abs_stdin_file != "" && !fs::exists(abs_stdin_file)) error("stdin file: " + ("\"" + cmd.stdin_file.string() + "\"") + " for command: " + ("\"" + cmd.executable.string() + concat_str_vec(cmd.arguments, " ") + "\"") + " doesent exist!");
    
    fs::path abs_stdout_file = canonize_path(cmd.stdout_file, false, true);
    fs::path abs_stderr_file = canonize_path(cmd.stderr_file, false, true);
    
    string abs_stdin_file_str  = abs_stdin_file.string();
    string abs_stdout_file_str = abs_stdout_file.string();
    string abs_stderr_file_str = abs_stderr_file.string();
    
    fs::path abs_working_dir = canonize_path(cmd.working_dir);
    fs::path prev_working_dir = fs::current_path();
    fs::current_path(abs_working_dir);
    
    Process created_process = {};
    
    #ifdef BUILDIT_OS_WINDOWS
    
    string cmd_line = command_to_cmd_line(cmd);
    char* cmd_line_cstr = (char*)malloc(cmd_line.size() + 1);
    strcpy(cmd_line_cstr, cmd_line.c_str());
    
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    
    HANDLE in_file  = NULL;
    HANDLE out_file = NULL;
    HANDLE err_file = NULL;
    cout << abs_stdin_file_str << endl;
    if (abs_stdin_file_str  != "") in_file  = CreateFileA(abs_stdin_file_str.c_str(),  GENERIC_READ,  0, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (abs_stdout_file_str != "") out_file = CreateFileA(abs_stdout_file_str.c_str(), GENERIC_WRITE, 0, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (abs_stderr_file_str != "") err_file = CreateFileA(abs_stderr_file_str.c_str(), GENERIC_WRITE, 0, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (abs_stdin_file_str  != "") ASSERT_WINAPI(in_file  != INVALID_HANDLE_VALUE);
    if (abs_stdout_file_str != "") ASSERT_WINAPI(out_file != INVALID_HANDLE_VALUE);
    if (abs_stderr_file_str != "") ASSERT_WINAPI(err_file != INVALID_HANDLE_VALUE);
    
    STARTUPINFOA si = {};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput  = (abs_stdin_file_str  != "" ? in_file  : GetStdHandle(STD_INPUT_HANDLE));
    si.hStdOutput = (abs_stdout_file_str != "" ? out_file : GetStdHandle(STD_OUTPUT_HANDLE));
    si.hStdError  = (abs_stderr_file_str != "" ? err_file : GetStdHandle(STD_ERROR_HANDLE));
    PROCESS_INFORMATION pi = {};
    ASSERT_WINAPI(CreateProcessA(NULL, cmd_line_cstr, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi) != 0);
    
    free(cmd_line_cstr);
    if (abs_stdin_file_str  != "") ASSERT_WINAPI(CloseHandle(in_file ) != 0);
    if (abs_stdout_file_str != "") ASSERT_WINAPI(CloseHandle(out_file) != 0);
    if (abs_stderr_file_str != "") ASSERT_WINAPI(CloseHandle(err_file) != 0);
    
    created_process = {pi};
    
    #else // BUILDIT_OS_WINDOWS
    
    string abs_executable_string = abs_executable.string();
    char** executable_args = str_vec_to_cstr_arr(concat_vec(vector<string>({abs_executable_string}), cmd.arguments));
        
    pid_t pid = fork();
    ASSERT_ERRNO(pid != -1);
    if(pid == 0) {
        set_fd_as_file(abs_stdin_file_str,  STDIN_FILENO,  O_RDONLY);
        set_fd_as_file(abs_stdout_file_str, STDOUT_FILENO, O_WRONLY | O_CREAT | O_TRUNC);
        set_fd_as_file(abs_stderr_file_str, STDERR_FILENO, O_WRONLY | O_CREAT | O_TRUNC);
        ASSERT_ERRNO(execv(abs_executable_string.c_str(), executable_args) != -1);
        exit(0);
    }
    
    created_process = {executable_args, pid};
    
    #endif // BUILDIT_OS_WINDOWS
    
    fs::current_path(prev_working_dir);
        
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
vector<int> chain_commands(vector<Command> cmds, vector<Process>* async) {
    if(cmds.size() < 2) {
        return vector<int>({execute_cmd(cmds[0], async)});
    }
    
    fs::path abs_stdin_file = canonize_path(cmds[0].stdin_file, false, true);
    if (abs_stdin_file != "" && !fs::exists(abs_stdin_file)) error("stdin file: " + ("\"" + cmds[0].stdin_file.string() + "\"") + " doesent exist!");
    
    fs::path abs_stdout_file = canonize_path(cmds[cmds.size() - 1].stdout_file, false, true);
    
    string abs_stdin_file_str  = abs_stdin_file.string();
    string abs_stdout_file_str = abs_stdout_file.string();
    
    vector<Process> sync_vec = vector<Process>();
    vector<int> exit_codes = vector<int>();
    
    #ifdef BUILDIT_OS_WINDOWS
    
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    struct Pipe_Handles { HANDLE read; HANDLE write; };
    vector<Pipe_Handles> pipes = vector<Pipe_Handles>(cmds.size() - 1);
    HANDLE file_in_handle = NULL;
    HANDLE file_out_handle = NULL;
    
    for(size_t i = 0; i < cmds.size() - 1; i++) {
        ASSERT_WINAPI(CreatePipe(&pipes[i].read, &pipes[i].write, &sa, 0) != 0);
        ASSERT_WINAPI(SetHandleInformation(pipes[i].read,  HANDLE_FLAG_INHERIT, FALSE) != 0);
        ASSERT_WINAPI(SetHandleInformation(pipes[i].write, HANDLE_FLAG_INHERIT, FALSE) != 0);
    }
    
    for(size_t i = 0; i < cmds.size(); i++) {
        HANDLE in_handle  = NULL;
        HANDLE out_handle = NULL;
        if(i > 0) {
            in_handle = pipes[i - 1].read;
            SetHandleInformation(in_handle, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
        }else if (abs_stdin_file_str != ""){
            in_handle = CreateFileA(abs_stdin_file_str.c_str(), GENERIC_READ, 0, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            ASSERT_WINAPI(in_handle != INVALID_HANDLE_VALUE);
            file_in_handle = in_handle;
        }else{
            in_handle = GetStdHandle(STD_INPUT_HANDLE);
            ASSERT_WINAPI(SetHandleInformation(in_handle, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT));
        }
        if(i + 1 < cmds.size()) {
            out_handle = pipes[i].write;
            SetHandleInformation(out_handle, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
        }else if (abs_stdout_file_str != ""){
            out_handle = CreateFileA(abs_stdout_file_str.c_str(), GENERIC_WRITE, 0, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            ASSERT_WINAPI(out_handle != INVALID_HANDLE_VALUE);
            file_out_handle = out_handle;
        }else{
            out_handle = GetStdHandle(STD_OUTPUT_HANDLE);
            ASSERT_WINAPI(SetHandleInformation(out_handle, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT));
        }
    
        string cmd_line = command_to_cmd_line(cmds[i]);
        char* cmd_line_cstr = (char*)malloc(cmd_line.size() + 1);
        strcpy(cmd_line_cstr, cmd_line.c_str());
        
        STARTUPINFOA si = {};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdInput  = in_handle ;
        si.hStdOutput = out_handle;
        si.hStdError  = GetStdHandle(STD_ERROR_HANDLE);
        SetHandleInformation(si.hStdError, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
        PROCESS_INFORMATION pi = {};
        ASSERT_WINAPI(CreateProcessA(NULL, cmd_line_cstr, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi) != 0);
        free(cmd_line_cstr);
        
        if(async == NULL){
            sync_vec.push_back({pi});
        }else{
            (*async).push_back({pi});
        }
    }
    
    for(size_t i = 0; i < cmds.size() - 1; i++) {
        ASSERT_WINAPI(CloseHandle(pipes[i].read) != 0);
        ASSERT_WINAPI(CloseHandle(pipes[i].write) != 0);
    }
    
    if (file_in_handle)  ASSERT_WINAPI(CloseHandle(file_in_handle)  != 0);
    if (file_out_handle) ASSERT_WINAPI(CloseHandle(file_out_handle) != 0);
    
    #else // BUILDIT_OS_WINDOWS
    
    int pipes[2][cmds.size() - 1];
    for(size_t i = 0; i < cmds.size() - 1; i++) {
        ASSERT_ERRNO(pipe(pipes[i]) != -1);
    }
    
    for(size_t i = 0; i < cmds.size(); i++) {
        fs::path abs_executable = cmds[i].executable;
        if(!absolute_executable.is_absolute()) {
            absolute_executable = find_in_env_path(abs_executable);
        }
        abs_executable = (fs::weakly_canonical(abs_executable)).make_preferred();
        
        string abs_executable_string = abs_executable.string();
        char** executable_args = str_vec_to_cstr_arr(concat_vec(vector<string>({abs_executable_string}), cmds[i].arguments));
        
        pid_t pid = fork();
        ASSERT_ERRNO(pid != -1);
        if(pid == 0) {
            fs::path abs_working_dir = canonize_path(cmds[i].working_dir);
            fs::current_path(abs_working_dir);
            if(i > 0) {
                size_t pipe_idx = i - 1;
                ASSERT_ERRNO(close(pipes[pipe_idx][1]) != -1);              //closing pipe write
                ASSERT_ERRNO(dup2(pipes[pipe_idx][0], STDIN_FILENO) != -1); //replacing stdin with pipe read
                ASSERT_ERRNO(close(pipes[pipe_idx][0]) != -1);              //closing pipe read
            }else if (abs_stdin_file_str != ""){
                set_fd_as_file(abs_stdin_file_str, STDIN_FILENO, O_RDONLY);
            }
            if(i + 1 < cmds.size()) {
                size_t pipe_idx = i;
                ASSERT_ERRNO(close(pipes[pipe_idx][0]) != -1);               //closing pipe read
                ASSERT_ERRNO(dup2(pipes[pipe_idx][1], STDOUT_FILENO) != -1); //replacing stdout with pipe write 
                ASSERT_ERRNO(close(pipes[pipe_idx][1]) != -1);               //closing pipe write
            }else if (abs_stdout_file_str != 0){
                set_fd_as_file(abs_stdout_file_str, STDOUT_FILENO, O_WRONLY | O_CREAT | O_TRUNC);
            }
            for(size_t c = 0; c < cmds.size() - 1; c++) {
                if(c == i || c == i - 1) continue;
                for(size_t p = 0; p < 2; p++) {
                    ASSERT_ERRNO(close(pipes[c][p]) != -1);
                }
            }
            
            ASSERT_ERRNO(execv(abs_executable_string.c_str(), executable_args));
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
    
    #endif // BUILDIT_OS_WINDOWS
    
    if(async == NULL) {
        exit_codes = wait_for_processes(sync_vec);
    }
    
    return exit_codes;
}
int wait_for_process(Process process){
    #ifdef BUILDIT_OS_WINDOWS
    DWORD exit_code = 0;
    
    ASSERT_WINAPI(WaitForSingleObject(process.process.hProcess, INFINITE) == WAIT_OBJECT_0);
    ASSERT_WINAPI(GetExitCodeProcess(process.process.hProcess, &exit_code));

    ASSERT_WINAPI(CloseHandle(process.process.hProcess) != 0);
    ASSERT_WINAPI(CloseHandle(process.process.hThread) != 0);
    
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

fs::path find_in_env_path(fs::path filename, bool search_working_dir){
    vector<string> path_dirs = split_string(getenv("PATH"), BUILDIT_ENV_PATH_SEPARATOR);
    if (search_working_dir) path_dirs.push_back(fs::current_path().string());
    for(size_t i = 0; i < path_dirs.size(); i++) {
        for(fs::directory_entry const& file : fs::directory_iterator(path_dirs[i])){
            if(file.exists() && !file.is_directory()) {
                if(file.path().filename() == filename) return file.path();
            }
        }
    }
    return "";
}
fs::path find_first_in_env_path(vector<fs::path> filenames, bool search_working_dir){
    for(size_t i = 0; i < filenames.size(); i++){
        fs::path path = find_in_env_path(filenames[i], search_working_dir);
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
    #if defined(BUILDIT_OS_WINDOWS)
        wchar_t exe_path[FILENAME_MAX] = { 0 };
        GetModuleFileNameW(nullptr, exe_path, FILENAME_MAX);
        return fs::path(exe_path);
    #elif defined(BUILDIT_OS_APPLE)
        char exe_path[16];
        uint32_t size = (uint32_t)sizeof(exe_path);
        if (_NSGetExecutablePath(exe_path, &size) == -1)
        {
            exe_path = (char*)WAI_MALLOC(size);
            _NSGetExecutablePath(exe_path, &size);
        }
        return fs::path(string(exe_path, size));
    #elif defined(BUILDIT_OS_LINUX) || defined(BUILDIT_OS_SOLARIS) || defined(BUILDIT_OS_BSD) || defined(BUILDIT_PROCFS)
        char exe_path[FILENAME_MAX];
        vector<fs::path> self_exe_paths = vector<fs::path>({"/proc/self/exe", "/proc/self/path/a.out", "/proc/curproc/file", "/proc/curproc/exe"});
        for(size_t i = 0; self_exe_paths.size(); i++) {
            if(fs::exists(self_exe_paths[i])){
                string self_exe_path_str = self_exe_paths[i].string();
                ssize_t count = readlink(self_exe_path_str.c_str(), exe_path, FILENAME_MAX);
                return fs::path(string(exe_path, count));
            }
        }
        error("No procfs (or known path) found on this OS!");
    #else
        error("No known method to find the running executable on this OS! (define BUILDIT_PROCFS to try procfs)");
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
string command_to_cmd_line(Command cmd){
    string cmd_line = canonize_path(cmd.executable, true).string() + " ";
    for(size_t i = 0; i < cmd.arguments.size(); i++) {
        if(cmd.arguments[i].find(" ") != string::npos){
            cmd_line += "\"" + cmd.arguments[i] + "\"";
        } else {
            cmd_line += cmd.arguments[i];
        }
        cmd_line += " ";
    }
    return cmd_line;
}
#else
void set_fd_as_file(string file, int fd, int open_flags, int create_file_permissions) {
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
