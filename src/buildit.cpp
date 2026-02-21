#define BUILDIT_DEBUG
#include "buildit.h"

using namespace std;

namespace buildit {

Command get_compile_cmd(fs::path compiler, std::vector<fs::path> source_files, fs::path object, std::vector<fs::path> include_dirs, bool auto_extensions, Optimization_Level optimize, bool all_warnings, bool pedantic, bool native_arch) {
    std::vector<fs::path> abs_source_files = canonize_paths(source_files, false, true);
    std::vector<fs::path> abs_include_dirs = canonize_paths(include_dirs, false, true);
    fs::path abs_object = canonize_path(object, false, true);
    if(auto_extensions) abs_object = fs::path(abs_object.string() + BUILDIT_OS_OBJ_EXTENSION);
    
    if(!compiler.has_filename()) error("The compiler doesent have a filename!");
    fs::path abs_compiler = canonize_path(compiler, true);
    string compiler_name = to_lower(abs_compiler.filename().string());
    
    Command cmd = {abs_compiler, vector<string>()};
    if (contains(compiler_name, "cl.exe")){
        cmd.arguments.push_back("/c");
        cmd.arguments.push_back("/Fo:");
        cmd.arguments.push_back(abs_object.string());
        for(size_t i = 0; i < abs_source_files.size(); i++) {
            cmd.arguments.push_back(abs_source_files[i].string());
        }
        for(size_t i = 0; i < abs_include_dirs.size(); i++) {
            cmd.arguments.push_back("/I");
            cmd.arguments.push_back(abs_include_dirs[i].string());
        }
        if(all_warnings) cmd.arguments.push_back("/Wall");
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
        if(pedantic) cmd.arguments.push_back("-pedantic");
        if(native_arch) cmd.arguments.push_back("-march=native");
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
Command get_link_cmd(fs::path linker, fs::path output_file, std::vector<fs::path> objects, std::vector<fs::path> libraries, std::vector<fs::path> library_dirs, bool auto_extensions, Optimization_Level optimize) {
    std::vector<fs::path> abs_objects = canonize_paths(objects, false, true);
    std::vector<fs::path> abs_library_dirs = canonize_paths(library_dirs, false, true);
    fs::path abs_output_file = canonize_path(output_file, false, true);
    
    if(auto_extensions) for(size_t i = 0; i < abs_objects.size(); i++) abs_objects[i] = fs::path(abs_objects[i].string() + BUILDIT_OS_OBJ_EXTENSION);

    if(!linker.has_filename()) error("The linker doesent have a filename!");
    fs::path abs_linker = canonize_path(linker, true);
    string linker_name = to_lower(abs_linker.filename().string());
    
    Command cmd = {abs_linker, vector<string>()};
    if (contains(linker_name, "cl.exe")){
        cmd.arguments.push_back("/Fe:");
        cmd.arguments.push_back(abs_output_file.string());
        for(size_t i = 0; i < abs_objects.size(); i++) {
            cmd.arguments.push_back(abs_objects[i].string());
        }
        for(size_t i = 0; i < abs_library_dirs.size(); i++) {
            cmd.arguments.push_back("/LIBPATH:");
            cmd.arguments.push_back(abs_library_dirs[i].string());
        }
        for(size_t i = 0; i < libraries.size(); i++) {
            cmd.arguments.push_back(libraries[i].string());
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
    } else {
        cmd.arguments.push_back("-o");
        cmd.arguments.push_back(abs_output_file.string());
        for(size_t i = 0; i < abs_objects.size(); i++) {
            cmd.arguments.push_back(abs_objects[i].string());
        }
        for(size_t i = 0; i < abs_library_dirs.size(); i++) {
            cmd.arguments.push_back("-L");
            cmd.arguments.push_back(abs_library_dirs[i].string());
        }
        for(size_t i = 0; i < libraries.size(); i++) {
            cmd.arguments.push_back("-l");
            cmd.arguments.push_back(libraries[i].string());
        }
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

void create_vscode_settings(std::string cmd, fs::path dir) {
    todo("create_vscode_settings");
}
void create_clangd_settings(std::string cmd, fs::path dir) {
    todo("create_clangd_settings");
}

fs::path find_c_compiler(vector<fs::path> compilers){
    if(compilers.size() == 0) {
        compilers = vector<fs::path>({"gcc", "clang", "cl.exe"});
    }
    fs::path compiler = find_first_in_env_path(compilers);
    if(BUILDIT_DEBUG_BOOL) log(LOG_LEVEL_DEBUG, "Found C compiler: " + compiler.string());
    return compiler;
}
fs::path find_cxx_compiler(vector<fs::path> compilers){
    if(compilers.size() == 0) {
        compilers = vector<fs::path>({"g++", "clang++", "cl.exe"});
    }
    fs::path compiler = find_first_in_env_path(compilers);
    if(BUILDIT_DEBUG_BOOL) log(LOG_LEVEL_DEBUG, "Found C++ compiler: " + compiler.string());
    return compiler;
}

fs::path find_c_linker(vector<fs::path> linkers){
    fs::path linker = find_c_compiler(linkers);
    return linker;
}
fs::path find_cxx_linker(vector<fs::path> linkers){
    fs::path linker = find_cxx_compiler(linkers);
    return linker;
}
fs::path find_linker(vector<fs::path> linkers){
    if(linkers.size() == 0) {
        linkers = vector<fs::path>({"ld", "lld", "link.exe"});
    }
    fs::path linker = find_first_in_env_path(linkers);
    if(BUILDIT_DEBUG_BOOL) log(LOG_LEVEL_DEBUG, "Found linker: " + linker.string());
    return linker;
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
std::string to_lower(std::string str) {
    string str_out = "";
    for(size_t i = 0; i < str.size(); i++){
        str_out += tolower(str[i]);
    }
    return str_out;
}
std::string to_upper(std::string str) {
    string str_out = "";
    for(size_t i = 0; i < str.size(); i++){
        str_out += toupper(str[i]);
    }
    return str_out;
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
bool vec_any_nonzero(std::vector<int> vec) {
    bool nonzero = false;
    for(size_t i = 0; i < vec.size(); i++) {
        nonzero = nonzero || (vec[i] != 0);
    }
    return nonzero;
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
fs::path get_system_shell(){
    vector<fs::path> shells = vector<fs::path>({"bash", "cmd.exe", "zsh", "fish", "dash", "tcsh", "csh", "ksh", "sh"});
    fs::path shell = find_first_in_env_path(shells);
    if(BUILDIT_DEBUG_BOOL) log(LOG_LEVEL_DEBUG, "Found system shell: " + shell.string());
    return shell;
}

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
        case LOG_LEVEL_DEBUG:
        cerr << "[DEBUG] "    << msg << endl;
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
int execute_cmd(Command cmd, vector<Process>* async){
    fs::path abs_executable = canonize_path(cmd.executable, true);
    if (abs_executable == fs::path("")) error("Could not find executable: " + cmd.executable.string() + " !");
    
    fs::path abs_stdin_file  = canonize_path(cmd.stdin_file, false, true);
    if (abs_stdin_file != "" && !fs::exists(abs_stdin_file)) error("stdin file: " + ("\"" + cmd.stdin_file.string() + "\"") + " for command: " + ("\"" + cmd.executable.string() + concat_str_vec(cmd.arguments, " ") + "\"") + " does not exist!");
    
    fs::path abs_stdout_file = canonize_path(cmd.stdout_file, false, true);
    fs::path abs_stderr_file = canonize_path(cmd.stderr_file, false, true);
    
    string abs_stdin_file_str  = abs_stdin_file.string();
    string abs_stdout_file_str = abs_stdout_file.string();
    string abs_stderr_file_str = abs_stderr_file.string();
    
    fs::path abs_working_dir = canonize_path(cmd.working_dir);
    fs::path prev_working_dir = fs::current_path();
    
    if(BUILDIT_DEBUG_BOOL) log(LOG_LEVEL_DEBUG, "Executing: \"" + abs_executable.string() + " " + concat_str_vec(cmd.arguments, " ") + "\" At: \"" + abs_working_dir.string()
     + "\" With stdin as: \"" + abs_stdin_file_str + "\" With stdout as: \"" + abs_stdout_file_str + "\" With stderr as: \"" + abs_stderr_file_str + "\"");
    
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
    
    #else // !BUILDIT_OS_WINDOWS
    
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
    if (abs_stdin_file != "" && !fs::exists(abs_stdin_file)) error("stdin file: " + ("\"" + cmds[0].stdin_file.string() + "\"") + " does not exist!");
    
    fs::path abs_stdout_file = canonize_path(cmds[cmds.size() - 1].stdout_file, false, true);
    
    string abs_stdin_file_str  = abs_stdin_file.string();
    string abs_stdout_file_str = abs_stdout_file.string();
    
    if(BUILDIT_DEBUG_BOOL) { 
        string log_str = "Chaining: " + abs_stdin_file_str + " > ";
        for(size_t i = 0; i < cmds.size(); i++) {
            log_str += canonize_path(cmds[i].executable, true).string() + " " + concat_str_vec(cmds[i].arguments, " ");
            if(i != cmds.size()) log_str += " > ";
        }
        log_str += abs_stdout_file_str;
        log(LOG_LEVEL_DEBUG, log_str);
    };
    
    vector<Process> sync_vec = vector<Process>();
    vector<int> exit_codes = vector<int>();
    
    #ifdef BUILDIT_OS_WINDOWS
    
    fs::path prev_working_dir = fs::current_path();
    HANDLE pipe_read = NULL;

    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

    for (size_t i = 0; i < cmds.size(); ++i) {
        HANDLE in_handle  = NULL;
        HANDLE out_handle = NULL;
        HANDLE err_handle = NULL;
        HANDLE pipe_read_next = NULL;
        
        bool is_first = (i == 0);
        bool is_last  = (i == cmds.size() - 1);

        fs::path abs_executable = canonize_path(cmds[i].executable, true);
        if (abs_executable == fs::path("")) error("Could not find executable: " + cmds[i].executable.string() + " !");

        fs::path abs_working_dir = canonize_path(cmds[i].working_dir);
        fs::current_path(abs_working_dir);

        string cmd_line = command_to_cmd_line(cmds[i]);
        char* cmd_line_cstr = (char*)malloc(cmd_line.size() + 1);
        strcpy(cmd_line_cstr, cmd_line.c_str());
        
        if (is_first) {
            fs::path abs_stdin_file = canonize_path(cmds[i].stdin_file, false, true);
            string abs_stdin_file_str = abs_stdin_file.string();
            
            if (abs_stdin_file_str != "" && !fs::exists(abs_stdin_file)) 
                 error("stdin file not found: " + cmds[i].stdin_file.string());

            if (abs_stdin_file_str != "") {
                in_handle = CreateFileA(abs_stdin_file_str.c_str(), GENERIC_READ, 0, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                ASSERT_WINAPI(in_handle != INVALID_HANDLE_VALUE);
            } else {
                in_handle = GetStdHandle(STD_INPUT_HANDLE);
            }
        } else {
            in_handle = pipe_read;
        }
        
        if (is_last) {
            fs::path abs_stdout_file = canonize_path(cmds[i].stdout_file, false, true);
            string abs_stdout_file_str = abs_stdout_file.string();

            if (abs_stdout_file_str != "") {
                out_handle = CreateFileA(abs_stdout_file_str.c_str(), GENERIC_WRITE, 0, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                ASSERT_WINAPI(out_handle != INVALID_HANDLE_VALUE);
            } else {
                out_handle = GetStdHandle(STD_OUTPUT_HANDLE);
            }
        } else {
            HANDLE pipe_write = NULL; 
            ASSERT_WINAPI(CreatePipe(&pipe_read_next, &pipe_write, &sa, 0) != 0);
            out_handle = pipe_write;
        }

        fs::path abs_stderr_file = canonize_path(cmds[i].stderr_file, false, true);
        string abs_stderr_file_str = abs_stderr_file.string();
        if (abs_stderr_file_str != "") {
            err_handle = CreateFileA(abs_stderr_file_str.c_str(), GENERIC_WRITE, 0, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            ASSERT_WINAPI(err_handle != INVALID_HANDLE_VALUE);
        } else {
            err_handle = GetStdHandle(STD_ERROR_HANDLE);
        }

        STARTUPINFOA si = {};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdInput  = in_handle;
        si.hStdOutput = out_handle;
        si.hStdError  = err_handle;
        
        PROCESS_INFORMATION pi = {};
        ASSERT_WINAPI(CreateProcessA(NULL, cmd_line_cstr, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi) != 0);
        
        if (async == NULL) {
            sync_vec.push_back({pi});
        }else{
            (*async).push_back({pi});
        }

        free(cmd_line_cstr);

        if (is_first && in_handle != GetStdHandle(STD_INPUT_HANDLE)){
            ASSERT_WINAPI(CloseHandle(in_handle) != 0);
        }
        if (is_last  && out_handle != GetStdHandle(STD_OUTPUT_HANDLE)){
            ASSERT_WINAPI(CloseHandle(out_handle) != 0);
        }
        if (err_handle != GetStdHandle(STD_ERROR_HANDLE)){
            ASSERT_WINAPI(CloseHandle(err_handle) != 0);
        }

        if (!is_first) {
            ASSERT_WINAPI(CloseHandle(pipe_read) != 0);
        }
        
        if (!is_last) {
            ASSERT_WINAPI(CloseHandle(out_handle) != 0);
            pipe_read = pipe_read_next; 
        }
    }

    fs::current_path(prev_working_dir);
    
    #else // !BUILDIT_OS_WINDOWS
    
    int pipes[2][cmds.size() - 1];
    for(size_t i = 0; i < cmds.size() - 1; i++) {
        ASSERT_ERRNO(pipe(pipes[i]) != -1);
    }
    
    for(size_t i = 0; i < cmds.size(); i++) {
        fs::path abs_executable = cmds[i].executable;
        if(!abs_executable.is_absolute()) {
            abs_executable = find_in_env_path(abs_executable);
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
            }else if (abs_stdout_file_str != ""){
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
    if(BUILDIT_DEBUG_BOOL) log(LOG_LEVEL_DEBUG, "Waiting for process id: " + to_string(process.process.dwProcessId));
    
    DWORD exit_code = 0;
    
    ASSERT_WINAPI(WaitForSingleObject(process.process.hProcess, INFINITE) == WAIT_OBJECT_0);
    ASSERT_WINAPI(GetExitCodeProcess(process.process.hProcess, &exit_code));

    ASSERT_WINAPI(CloseHandle(process.process.hProcess) != 0);
    ASSERT_WINAPI(CloseHandle(process.process.hThread) != 0);
    
    return exit_code;
    
    #else // !BUILDIT_OS_WINDOWS
    if(BUILDIT_DEBUG_BOOL) log(LOG_LEVEL_DEBUG, "Waiting for process id: " + to_string(process.pid));
    
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
