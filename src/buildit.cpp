#include "buildit.h"

using namespace std;

namespace buildit {

//////////////////////////////////////////////////////////////////////
// Debugging
// \/
//////////////////////////////////////////////////////////////////////

static bool buildit_global_debug = false;

void set_debug(bool debug){
    buildit_global_debug = debug;
}
bool get_debug(){
    return buildit_global_debug;
}

//////////////////////////////////////////////////////////////////////
// Piping
// \/
//////////////////////////////////////////////////////////////////////

Pipe open_pipe(){
    Pipe p = pipe_invalid();
#ifdef BUILDIT_OS_WINDOWS
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
    
    ASSERT_WINAPI(CreatePipe(&p.read_handle, &p.write_handle, &sa, 0));
#else
    int fds[2];
    ASSERT_ERRNO(pipe(fds) == 0);
    p.read_fd = fds[0];
    p.write_fd = fds[1];
#endif
    if(get_debug()) log(LOG_LEVEL_DEBUG, "Opened pipe: \"" + as_string(p) + "\"");
    return p;
}
void close_pipe(Pipe pipe) {
    if(get_debug()) log(LOG_LEVEL_DEBUG, "Closing pipe: \"" + as_string(pipe) + "\"");
    #ifdef BUILDIT_OS_WINDOWS
    if (pipe.read_handle != INVALID_HANDLE_VALUE && pipe.read_handle != NULL) {
        CloseHandle(pipe.read_handle);
    }
    if (pipe.write_handle != INVALID_HANDLE_VALUE && pipe.write_handle != NULL) {
        CloseHandle(pipe.write_handle);
    }
    #else
    if (pipe.read_fd > -1) close(pipe.read_fd);
    if (pipe.write_fd > -1) close(pipe.write_fd);
    #endif
}
Pipe pipe_to_file(fs::path file) {
    fs::path abs_file = canonize_path(file);
    Pipe p = pipe_invalid();
#ifdef BUILDIT_OS_WINDOWS
    p.read_handle = INVALID_HANDLE_VALUE;
    p.write_handle = CreateFileA(abs_file.string().c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    ASSERT_WINAPI(p.write_handle != INVALID_HANDLE_VALUE);
#else
    p.read_fd = -1;
    p.write_fd = open(abs_file.string().c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    ASSERT_ERRNO(p.write_fd > -1);
#endif
    if(get_debug()) log(LOG_LEVEL_DEBUG, "Opened pipe to file: \"" + abs_file.string() + "\" : \"" + as_string(p) + "\"");
    return p;
}
Pipe pipe_from_file(fs::path file) {
    fs::path abs_file = canonize_path(file);
    Pipe p = pipe_invalid();
#ifdef BUILDIT_OS_WINDOWS
    p.write_handle = INVALID_HANDLE_VALUE;
    p.read_handle = CreateFileA(abs_file.string().c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    ASSERT_WINAPI(p.read_handle != INVALID_HANDLE_VALUE);
#else
    p.write_fd = -1;
    p.read_fd = open(abs_file.string().c_str(), O_RDONLY);
    ASSERT_ERRNO(p.read_fd > -1);
#endif
    if(get_debug()) log(LOG_LEVEL_DEBUG, "Opened pipe from file: \"" + abs_file.string() + "\" : \"" + as_string(p) + "\"");
    return p;
}
Pipe pipe_from_file_stream(FILE* stream_in, FILE* stream_out) {
    Pipe p = pipe_invalid();
    #ifdef BUILDIT_OS_WINDOWS
    if (stream_in != NULL ) {
        int fd = _fileno(stream_in);
        p.read_handle = (HANDLE)_get_osfhandle(fd);
        ASSERT_WINAPI(p.read_handle != INVALID_HANDLE_VALUE);
    }
    if (stream_out != NULL ) {
        int fd = _fileno(stream_out);
        p.write_handle = (HANDLE)_get_osfhandle(fd);
        ASSERT_WINAPI(p.read_handle != INVALID_HANDLE_VALUE);
    }
    #else
    if (stream_in != NULL ) {
        p.read_fd = fileno(stream_in);
        ASSERT_ERRNO(p.read_fd > -1);
    }
    if (stream_out != NULL ) {
        p.write_fd = fileno(stream_out);
        ASSERT_ERRNO(p.write_fd > -1);
    }
    #endif
    return p;
}
Pipe pipe_stdin(){
    return pipe_from_file_stream(stdin,  NULL);
}
Pipe pipe_stdout(){
    return pipe_from_file_stream(stdout, NULL);
}
Pipe pipe_stderr(){
    return pipe_from_file_stream(stderr, NULL);
}
Pipe pipe_invalid(){
    #ifdef BUILDIT_OS_WINDOWS
    return LIT(Pipe){INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE};
    #else
    return LIT(Pipe){-1, -1};
    #endif
}
bool is_pipe_invalid(Pipe pipe) {
    #ifdef BUILDIT_OS_WINDOWS
    return pipe.write_handle == INVALID_HANDLE_VALUE && pipe.read_handle == INVALID_HANDLE_VALUE;
    #else
    return pipe.write_fd < 0 && pipe.read_fd < 0;
    #endif
}

//////////////////////////////////////////////////////////////////////
// Find common programs
// \/
//////////////////////////////////////////////////////////////////////

fs::path find_compiler(bool cxx){
    #if defined(BUILDIT_COMPILER_CLANG)
    if(cxx) return find_in_env_path("clang++");
    else return find_in_env_path("clang");
    #elif defined(BUILDIT_COMPILER_GCC)
    if(cxx) return find_in_env_path("g++");
    else return find_in_env_path("gcc");
    #elif defined(BUILDIT_COMPILER_MSVC)
    return find_in_env_path("cl");
    #endif
    error("Could not find the compiler this is compiled with!");
    return "";
}
fs::path find_linker(){
    #if defined(BUILDIT_COMPILER_CLANG)
    return find_in_env_path("lld");
    #elif defined(BUILDIT_OS_WINDOWS)
    return find_in_env_path("link");
    #else
    return find_in_env_path("ld");
    #endif
}

//////////////////////////////////////////////////////////////////////
// Rebuild Yourself
// \/
//////////////////////////////////////////////////////////////////////

void rebuild_yourself(char** argv, fs::path compiler) {
    todo("rebuild_yourself argv");
}
void rebuild_yourself(fs::path exe_path, fs::path compiler){
    todo("rebuild_yourself path");
}

//////////////////////////////////////////////////////////////////////
// Building commands
// \/
//////////////////////////////////////////////////////////////////////

Command default_command(fs::path executable, std::vector<std::string> arguments, fs::path working_dir, Pipe stdin_pipe, Pipe stdout_pipe, Pipe stderr_pipe) {
    return LIT(Command){executable, arguments, working_dir, stdin_pipe, stdout_pipe, stderr_pipe};
}
Command get_compile_cmd(fs::path compiler, std::vector<fs::path> source_files, fs::path object, std::vector<fs::path> include_dirs, bool auto_extensions, Optimization_Level optimize, bool debug, int version, bool all_warnings, bool pedantic, bool native_arch) {
    std::vector<fs::path> abs_source_files = canonize_paths(source_files, false, true);
    std::vector<fs::path> abs_include_dirs = canonize_paths(include_dirs, false, true);
    fs::path abs_object = canonize_path(object, false, true);
    if(auto_extensions) abs_object = fs::path(abs_object.string() + BUILDIT_OS_OBJ_EXTENSION);
    
    if(!compiler.has_filename()) error("The compiler doesent have a filename!");
    fs::path abs_compiler = canonize_path(compiler, true);
    string compiler_name = to_lower(abs_compiler.filename().string());
    
    bool cxx = contains(compiler_name, "++");
    
    Command cmd = default_command(abs_compiler, vector<string>());
    if (contains(compiler_name, "cl.exe")){
        cmd.arguments.push_back("/nologo");
        cmd.arguments.push_back("/c");
        cmd.arguments.push_back("/EHsc");
        cmd.arguments.push_back("/Z7");
        cmd.arguments.push_back("/std:c++" + to_string(version));
        cmd.arguments.push_back("/Fo:");
        cmd.arguments.push_back(abs_object.string());
        for(size_t i = 0; i < abs_include_dirs.size(); i++) {
            cmd.arguments.push_back("/I");
            cmd.arguments.push_back(abs_include_dirs[i].string());
        }
        if(all_warnings) cmd.arguments.push_back("/W4"); // not /Wall to ignore system headers warnings XPP
        if(pedantic) log(LOG_LEVEL_WARNING, "MSVC does not have a pedantic flag!");
        if(native_arch) {
            #if defined(BUILDIT_OS_AVX512)
            cmd.arguments.push_back("/arch:AVX512");
            #elif defined(BUILDIT_OS_AVX2)
            cmd.arguments.push_back("/arch:AVX2");
            #elif defined(BUILDIT_OS_AVX)
            cmd.arguments.push_back("/arch:AVX");
            #elif defined(BUILDIT_OS_SSE42)
            cmd.arguments.push_back("/arch:SSE4.2");
            #elif defined(BUILDIT_OS_SSE2)
            cmd.arguments.push_back("/arch:SSE2");
            #endif
        }
        switch(optimize) {
            case OPTIMIZATION_NONE:
                cmd.arguments.push_back("/Od");
            break;
            case OPTIMIZATION_SIZE:
                cmd.arguments.push_back("/O1");
            break;
            case OPTIMIZATION_SPEED:
                cmd.arguments.push_back("/O2");
            break;
            default:
                error("Unknown optimization level!");
            break;
        }
        for(size_t i = 0; i < abs_source_files.size(); i++) {
            cmd.arguments.push_back(abs_source_files[i].string());
        }
    } else {
        cmd.arguments.push_back("-c");
        cmd.arguments.push_back("-o");
        cmd.arguments.push_back(abs_object.string());
        for(size_t i = 0; i < abs_source_files.size(); i++) {
            cmd.arguments.push_back(abs_source_files[i].string());
        }
        for(size_t i = 0; i < abs_include_dirs.size(); i++) {
            cmd.arguments.push_back("-I");
            cmd.arguments.push_back(abs_include_dirs[i].string());
        }
        if(all_warnings) { 
            cmd.arguments.push_back("-Wall");
            cmd.arguments.push_back("-Wextra");
        }
        if(cxx) cmd.arguments.push_back("-std=c++" + to_string(version));
        else cmd.arguments.push_back("-std=c" + to_string(version));
        if(pedantic) cmd.arguments.push_back("-pedantic");
        if(native_arch) cmd.arguments.push_back("-march=native");
        if(debug) cmd.arguments.push_back("-g");
        switch(optimize) {
            case OPTIMIZATION_NONE:
                cmd.arguments.push_back("-O0");
            break;
            case OPTIMIZATION_SIZE:
                cmd.arguments.push_back("-Os");
            break;
            case OPTIMIZATION_SPEED:
                cmd.arguments.push_back("-O3");
            break;
            default:
                error("Unknown optimization level!");
            break;
        }
    }
    return cmd;
}
Command get_link_cmd(fs::path linker, fs::path output_file, std::vector<fs::path> objects, std::vector<fs::path> libraries, std::vector<fs::path> library_dirs, bool auto_extensions, Optimization_Level optimize, bool debug) {
    std::vector<fs::path> abs_objects = canonize_paths(objects, false, true);
    std::vector<fs::path> abs_library_dirs = canonize_paths(library_dirs, false, true);
    fs::path abs_output_file = canonize_path(output_file, false, true);
    
    if(auto_extensions) {
        for(size_t i = 0; i < abs_objects.size(); i++) abs_objects[i] = fs::path(abs_objects[i].string() + BUILDIT_OS_OBJ_EXTENSION);
        abs_output_file = fs::path(abs_output_file.string() + BUILDIT_OS_EXE_EXTENSION);
    }
    
    fs::path abs_linker = canonize_path(linker, true);
    string linker_name = to_lower(abs_linker.filename().string());
    
    Command cmd = default_command(abs_linker, vector<string>());
    if (contains(linker_name, "link.exe")){
        cmd.arguments.push_back("/NOLOGO");
        cmd.arguments.push_back("/OUT:" + abs_output_file.string());
        if(debug) cmd.arguments.push_back("/DEBUG:FULL");
        else  cmd.arguments.push_back("/DEBUG:NONE");
        for(size_t i = 0; i < abs_library_dirs.size(); i++) {
            cmd.arguments.push_back("/LIBPATH:" + abs_library_dirs[i].string());
        }
        switch(optimize) {
            case OPTIMIZATION_NONE:
                cmd.arguments.push_back("/OPT:NOREF,NOICF,NOLBR");
            break;
            case OPTIMIZATION_SIZE:
                cmd.arguments.push_back("/OPT:REF,ICF,LBR");
            break;
            case OPTIMIZATION_SPEED:
                cmd.arguments.push_back("/OPT:REF,ICF,LBR");
            break;
            default:
                error("Unknown optimization level!");
            break;
        }
        for(size_t i = 0; i < abs_objects.size(); i++) {
            cmd.arguments.push_back(abs_objects[i].string());
        }
        for(size_t i = 0; i < libraries.size(); i++) {
            cmd.arguments.push_back(libraries[i].string());
        }
    } else {
        cmd.arguments.push_back("-o");
        cmd.arguments.push_back(abs_output_file.string());
        for(size_t i = 0; i < abs_library_dirs.size(); i++) {
            cmd.arguments.push_back("-L");
            cmd.arguments.push_back(abs_library_dirs[i].string());
        }
        for(size_t i = 0; i < libraries.size(); i++) {
            cmd.arguments.push_back("-l");
            cmd.arguments.push_back(libraries[i].string());
        }
        if(debug) cmd.arguments.push_back("-g");
        switch(optimize) {
            case OPTIMIZATION_NONE:
                cmd.arguments.push_back("-O0");
            break;
            case OPTIMIZATION_SIZE:
                cmd.arguments.push_back("-Os");
            break;
            case OPTIMIZATION_SPEED:
                cmd.arguments.push_back("-O3");
            break;
            default:
                error("Unknown optimization level!");
            break;
        }
        for(size_t i = 0; i < abs_objects.size(); i++) {
            cmd.arguments.push_back(abs_objects[i].string());
        }
    }
    return cmd;
}

//////////////////////////////////////////////////////////////////////
// Executing/Waiting/Getting processes
// \/
//////////////////////////////////////////////////////////////////////

int execute_cmd(Command cmd, vector<Process>* async){
    fs::path abs_executable = canonize_path(cmd.executable, true);
    if (abs_executable == fs::path("")) error("Could not find executable: " + cmd.executable.string() + " !");
    
    fs::path abs_working_dir = canonize_path(cmd.working_dir);
    fs::path prev_working_dir = fs::current_path();
    
    if(get_debug()) log(LOG_LEVEL_DEBUG, "Executing: \"" + as_string(LIT(Command){abs_executable, cmd.arguments, cmd.working_dir, cmd.in_pipe, cmd.out_pipe, cmd.err_pipe}) + "\"");
    
    Pipe local_pipes[3] = {cmd.in_pipe, cmd.out_pipe, cmd.err_pipe};

    for (int i = 0; i < 3; ++i) {
        if (is_pipe_invalid(local_pipes[i])) {
            error(string("Invalid ") + (i == 0 ? "stdin" : (i == 1 ? "stdout" : (i == 2 ? "stderr" : ""))) + "pipe for process!");
        }
    }

    fs::current_path(abs_working_dir);
    
    Process created_process = {};
    
    #ifdef BUILDIT_OS_WINDOWS
    
    string cmd_line = command_to_cmd_line(cmd);
    char* cmd_line_cstr = (char*)malloc(cmd_line.size() + 1);
    strcpy(cmd_line_cstr, cmd_line.c_str());
    
    STARTUPINFOA si = {};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput  = local_pipes[0].read_handle;
    si.hStdOutput = local_pipes[1].write_handle;
    si.hStdError  = local_pipes[2].write_handle;
    PROCESS_INFORMATION pi = {};
    ASSERT_WINAPI(CreateProcessA(NULL, cmd_line_cstr, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi) != 0);

    if (local_pipes[0].read_handle != INVALID_HANDLE_VALUE)  CloseHandle(local_pipes[0].read_handle);
    if (local_pipes[1].write_handle != INVALID_HANDLE_VALUE) CloseHandle(local_pipes[1].write_handle);
    if (local_pipes[2].write_handle != INVALID_HANDLE_VALUE) CloseHandle(local_pipes[2].write_handle);

    local_pipes[0].read_handle = INVALID_HANDLE_VALUE;
    local_pipes[1].write_handle = INVALID_HANDLE_VALUE;
    local_pipes[2].write_handle = INVALID_HANDLE_VALUE;
    
    free(cmd_line_cstr);
    
    created_process = {pi};
    
    #else // !BUILDIT_OS_WINDOWS
    
    string abs_executable_string = abs_executable.string();
    char** executable_args = str_vec_to_cstr_arr(concat_vec(vector<string>({abs_executable_string}), cmd.arguments));
        
    pid_t pid = fork();
    ASSERT_ERRNO(pid != -1);
    if(pid == 0) {
        if (local_pipes[0].read_fd  > -1) dup2(local_pipes[0].read_fd,  STDIN_FILENO);
        if (local_pipes[1].write_fd > -1) dup2(local_pipes[1].write_fd, STDOUT_FILENO);
        if (local_pipes[2].write_fd > -1) dup2(local_pipes[2].write_fd, STDERR_FILENO);

        // TODO: is this good?
        //for (int i = 0; i < 3; ++i) {
        //    close_pipe(local_pipes[i]);
        //}

        ASSERT_ERRNO(execv(abs_executable_string.c_str(), executable_args) != -1);
        exit(0);
    }
    
    created_process = {pid, executable_args};
    
    #endif // BUILDIT_OS_WINDOWS
    
    fs::current_path(prev_working_dir);

    if(get_debug()) log(LOG_LEVEL_DEBUG, "Executed: " + as_string(created_process));
        
    if(async == NULL){
        return wait_for_process(created_process);
    } else {
        (*async).push_back(created_process);
    }

    return 0;
}
int wait_for_process(Process process){
    if(get_debug()) log(LOG_LEVEL_DEBUG, "Waiting for process: " + as_string(process));
    int ret_code = 0;
    #ifdef BUILDIT_OS_WINDOWS
    
    DWORD exit_code = 0;
    
    ASSERT_WINAPI(WaitForSingleObject(process.pi.hProcess, INFINITE) == WAIT_OBJECT_0);
    ASSERT_WINAPI(GetExitCodeProcess(process.pi.hProcess, &exit_code));

    ASSERT_WINAPI(CloseHandle(process.pi.hProcess) != 0);
    ASSERT_WINAPI(CloseHandle(process.pi.hThread) != 0);
    
    ret_code = exit_code;
    
    #else // !BUILDIT_OS_WINDOWS
    
    int child_status = 0;
    ASSERT_ERRNO(waitpid(process.pid, &child_status, 0) != -1);
    free_cstr_arr(process.executable_args);
    ret_code = WEXITSTATUS(child_status);
    
    #endif // BUILDIT_OS_WINDOWS

    if(get_debug()) log(LOG_LEVEL_DEBUG, "Process: " + as_string(process) + " Exited with: " + to_string(ret_code));
    return ret_code;
}
vector<int> execute_cmds(vector<Command> cmds, vector<Process>* async){
    vector<int> return_vec = vector<int>();
    for(size_t i = 0; i < cmds.size(); i++) {
        return_vec.push_back(execute_cmd(cmds[i], async));
    }
    return return_vec;
}
vector<int> wait_for_processes(vector<Process> processes){
    vector<int> vec = vector<int>();
    for(size_t i = 0; i < processes.size(); i++) {
        vec.push_back(wait_for_process(processes[i]));
    }
    return vec;
}
vector<int> chain_cmds(vector<Command> cmds, vector<Process>* async) {
    if(cmds.size() < 2) execute_cmd(cmds[0], async);
    vector<Process> async_local = vector<Process>();
    vector<int> returns = vector<int>();
    vector<Pipe> pipes = vector<Pipe>();
    for(size_t i = 0; i < cmds.size() - 1; i++) {
        pipes.push_back(open_pipe());
    }
    for(size_t i = 0; i < cmds.size(); i++) {
        if(i > 0) cmds[i].in_pipe = pipes[i - 1];
        if(i < cmds.size() - 1) cmds[i].out_pipe = pipes[i];
        returns.push_back(execute_cmd(cmds[i], IF_NULL(async, &async_local, async)));
    }
    if(async == NULL) {
        returns = wait_for_processes(async_local);
    }
    // TODO: If async then should this be here or after wait? 
    //for(size_t i = 0; i < pipes.size(); i++) {
    //    close_pipe(pipes[i]);
    //}
    return returns;
}
Process get_current_process() {
    #ifdef BUILDIT_OS_WINDOWS
    return LIT(Process){ LIT(PROCESS_INFORMATION){ GetCurrentProcess(), GetCurrentThread(), GetCurrentProcessId() , GetCurrentThreadId() }};
    #else
    return LIT(Process){ getpid(), NULL};
    #endif
}

//////////////////////////////////////////////////////////////////////
// Auto creating ide/editor/lsp setting files
// \/
//////////////////////////////////////////////////////////////////////

void create_clangd_settings(fs::path dir, bool cxx, std::vector<fs::path> include_dirs, int version, bool all_warnings, bool pedantic) {
    if(get_debug()) {
        string debug_msg = "Creating ClangD settings in directory: \"" + dir.string() + "\" with include dirs: [";
        for(size_t i = 0; i < include_dirs.size(); i++) {
            if(i > 0) debug_msg += ", ";
            debug_msg += "\"" + include_dirs[i].string() + "\"";
        }
        debug_msg += string("] ") + (cxx ? "c++" : "c") + " version: \"" + to_string(version) + "\" all warnings: \"" + (all_warnings ? "true" : "false") + "\" pedantic: \"" + (pedantic ? "true" : "false") + "\"";
        log(LOG_LEVEL_DEBUG, debug_msg);
    }
    
    vector<string> add_flags = vector<string>();
    for(size_t i = 0; i < include_dirs.size(); i++) {
        add_flags.push_back("\"-I" + include_dirs[i].string() + "\"");
    }
    if(cxx) add_flags.push_back("-std=c++" + to_string(version));
    else add_flags.push_back("-std=c" + to_string(version));
    if(all_warnings) {
        add_flags.push_back("-Wall");
        add_flags.push_back("-Wextra");
    }
    if(pedantic) add_flags.push_back("-pedantic");
    
    ofstream config_file = ofstream(canonize_path(dir) / ".clangd", ofstream::binary);
    config_file << "CompileFlags:\n    Compiler: ";
    config_file << (cxx ? "\"clang++\"" : "\"clang\"") << "\n";
    config_file << "    Add: [" << concat_str_vec(add_flags, ", ") << "]\n";
}

//////////////////////////////////////////////////////////////////////
// Logging
// \/
//////////////////////////////////////////////////////////////////////

void log(Log_Level level, string msg) {
    switch(level) {
        case LOG_LEVEL_INFO:
        cout << "[INFO] "    << msg << endl;
        break;
        case LOG_LEVEL_WARNING:
        cout << "[WARNING] " << msg << endl;
        break;
        case LOG_LEVEL_ERROR:
        cerr << "[ERROR] "   << msg << endl;
        break;
        case LOG_LEVEL_DEBUG: {
            cerr << "[DEBUG PID: " + as_string(get_current_process()) + "] " << msg << endl;
        }
        break;
        case LOG_LEVEL_TODO:
        cerr << "[TODO] "    << msg << endl;
        break;
        default:
        cerr << "WRONG LOG LEVEL!" << endl;
        exit(1);
        break;
    }
}
void error(string desc, int code){
    log(LOG_LEVEL_ERROR, desc);
    exit(code);
}
void todo(string desc){
    log(LOG_LEVEL_TODO, desc);
    exit(0);
}

//////////////////////////////////////////////////////////////////////
// Paths of common programs
// \/
//////////////////////////////////////////////////////////////////////

fs::path get_system_shell(){
    vector<fs::path> shells = vector<fs::path>({"bash", "cmd.exe", "zsh", "fish", "dash", "tcsh", "csh", "ksh", "sh"});
    fs::path shell = find_first_in_env_path(shells);
    if(get_debug()) log(LOG_LEVEL_DEBUG, "Found system shell: " + shell.string());
    return shell;
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
    return "";
}
fs::path get_current_executable_dir() {
    fs::path exec_path = get_current_executable_path();
    if(!exec_path.has_parent_path()) error("wtf your executable path doesnt have a parent (directory)");
    return exec_path.parent_path();
}

//////////////////////////////////////////////////////////////////////
// Path utilities
// \/
//////////////////////////////////////////////////////////////////////

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
vector<fs::path> canonize_paths(vector<fs::path> paths, bool executable, bool check_empty, bool exec_base_dir) {
    vector<fs::path> out_paths = vector<fs::path>();
    for(size_t i = 0; i < paths.size(); i++){
        out_paths.push_back(canonize_path(paths[i], executable, check_empty, exec_base_dir));
    }
    return out_paths;
}
std::vector<fs::path> replace_extensions(std::vector<fs::path> files, fs::path ext){
    vector<fs::path> out_files = vector<fs::path>();
    for(size_t i = 0; i < files.size(); i++) {
        out_files.push_back(files[i].replace_extension(ext));
    }
    return out_files;
}

//////////////////////////////////////////////////////////////////////
// Vector utilities
// \/
//////////////////////////////////////////////////////////////////////

bool vec_any_nonzero(std::vector<int> vec) {
    bool nonzero = false;
    for(size_t i = 0; i < vec.size(); i++) {
        nonzero = nonzero || (vec[i] != 0);
    }
    return nonzero;
}

//////////////////////////////////////////////////////////////////////
// String utilities
// \/
//////////////////////////////////////////////////////////////////////

string as_string(Command cmd){
    return cmd.executable.string() + " " + concat_str_vec(cmd.arguments, " ") + " " + cmd.working_dir.string() + " " + as_string(cmd.in_pipe) + " " + as_string(cmd.out_pipe) + " " + as_string(cmd.err_pipe);
}
string as_string(Pipe pipe){
    #ifdef BUILDIT_OS_WINDOWS
    return to_string((uintptr_t)pipe.read_handle) + " " + to_string((uintptr_t)pipe.write_handle);
    #else
    return to_string(pipe.read_fd) + " " + to_string(pipe.write_fd);
    #endif
}
string as_string(Process process){
    #ifdef BUILDIT_OS_WINDOWS
    return to_string(process.pi.dwProcessId);
    #else
    return to_string(process.pid);
    #endif
}

std::string to_lower(std::string str) {
    string str_out = "";
    for(size_t i = 0; i < str.size(); i++){
        str_out += (char)tolower((int)str[i]);
    }
    return str_out;
}
std::string to_upper(std::string str) {
    string str_out = "";
    for(size_t i = 0; i < str.size(); i++){
        str_out += (char)toupper((int)str[i]);
    }
    return str_out;
}
bool contains(std::string str, std::string token){
    return str.find(token) != string::npos;
}
bool contains_any(std::string str, std::vector<std::string> tokens){
    for(size_t i = 0; i < tokens.size(); i++){
        if(contains(str, tokens[i])) return true;
    }
    return false;
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
string concat_str_vec(vector<string> vec, string delim){
    string str = "";
    for(size_t i = 0; i < vec.size(); i++) {
        if(i > 0) str += delim;
        str += vec[i];
    }
    return str;
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

//////////////////////////////////////////////////////////////////////
// Path environment variable helpers
// \/
//////////////////////////////////////////////////////////////////////

fs::path find_in_env_path(fs::path filename, bool search_working_dir){
    vector<string> path_dirs = split_string(getenv("PATH"), BUILDIT_ENV_PATH_SEPARATOR);
    if (search_working_dir) path_dirs.push_back(fs::current_path().string());
    for(size_t i = 0; i < path_dirs.size(); i++) {
        for(fs::directory_entry const& file : fs::directory_iterator(path_dirs[i])){
            if(file.exists() && !file.is_directory()) {
                #if defined(BUILDIT_OS_WINDOWS)
                if(file.path().filename().string() == filename.string() + ".exe") return file.path();
                #endif
                if(file.path().filename().string() == filename.string()) return file.path();
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

//////////////////////////////////////////////////////////////////////
// OS Specific functions (for crossplatform apps dont use dummy)
// \/
//////////////////////////////////////////////////////////////////////

#ifdef BUILDIT_OS_WINDOWS
string command_to_cmd_line(Command cmd){
    string cmd_line = canonize_path(cmd.executable, true).string() + " ";
    for(size_t i = 0; i < cmd.arguments.size(); i++) {
        if(contains(cmd.arguments[i], " ")){
            cmd_line += "\"" + cmd.arguments[i] + "\"";
        } else {
            cmd_line += cmd.arguments[i];
        }
        cmd_line += " ";
    }
    return cmd_line;
}
#else // !BUILDIT_OS_WINDOWS
void dup_and_close_fd(int src_fd, int dest_fd, bool ok_if_invalid) {
    if (ok_if_invalid) {
        if(src_fd != -1 || dest_fd != -1) {
            return;
        }
    }

    ASSERT_ERRNO(src_fd != -1);
    ASSERT_ERRNO(dest_fd != -1);
    ASSERT_ERRNO(dup2(src_fd, dest_fd) != -1);
    ASSERT_ERRNO(close(src_fd) != -1);
}
#endif

}
