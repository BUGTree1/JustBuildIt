
//////////////////////////////////////////////////////////////////////////////////////////////
// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ //
// ░░░░░░   ░░░░░░░░░░░░░░░░░░░░░   ░░░░     ░░░░░░░░░░░░░░░░░░   ░░░░░░   ░░   ░░░   ░░░░░ //
// ▒▒▒▒▒▒   ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒   ▒▒▒▒  ▒▒   ▒▒▒▒▒▒▒▒▒▒▒▒   ▒   ▒▒▒▒▒▒   ▒▒   ▒▒▒   ▒▒▒▒▒ //
// ▒▒▒▒▒▒   ▒▒   ▒▒   ▒▒     ▒▒       ▒▒  ▒▒▒   ▒▒   ▒▒   ▒▒▒▒▒   ▒▒▒▒▒▒   ▒▒   ▒       ▒▒▒ //
// ▓▓▓▓▓▓   ▓▓   ▓▓   ▓   ▓▓▓▓▓▓▓   ▓▓▓▓      ▓▓▓▓   ▓▓   ▓   ▓   ▓▓       ▓▓   ▓▓▓   ▓▓▓▓▓ //
// ▓▓▓▓▓▓   ▓▓   ▓▓   ▓▓▓    ▓▓▓▓   ▓▓▓▓  ▓▓▓▓   ▓   ▓▓   ▓   ▓   ▓  ▓▓▓   ▓▓   ▓▓▓   ▓▓▓▓▓ //
// ▓  ▓▓▓   ▓▓   ▓▓   ▓▓▓▓▓   ▓▓▓   ▓  ▓  ▓▓▓▓▓  ▓   ▓▓   ▓   ▓   ▓  ▓▓▓   ▓▓   ▓▓▓   ▓  ▓▓ //
// ██     ██████      █      █████   ███        ████      █   █   ██       ██   ████   ████ //
// ████████████████████████████████████████████████████████████████████████████████████████ //
//////////////////////////////////////////////////////////////////////////////////////////////

// Simple C++ buildsystem

#pragma once
#ifndef __BUILDIT_H__
#define __BUILDIT_H__

#if defined(_WIN32) || defined(_WIN64)
#define BUILDIT_OS_WINDOWS
#endif
#if defined(__APPLE__)
#define BUILDIT_OS_APPLE
#endif
#if defined(__linux__) || defined(linux) || defined(__linux)
#define BUILDIT_OS_LINUX
#endif
#if defined(BSD)
#define BUILDIT_OS_BSD
#endif
#if defined(__FreeBSD__)
#define BUILDIT_OS_FREEBSD
#endif
#if defined(__OpenBSD__)
#define BUILDIT_OS_OPENBSD
#endif
#if defined(__NetBSD__)
#define BUILDIT_OS_NETBSD
#endif
#if defined(__DragonFly__)
#define BUILDIT_OS_DRAGONFLY_BSD
#endif
#if defined(__sun)
#define BUILDIT_OS_SOLARIS
#endif
#if defined(__ANDROID__)
#define BUILDIT_OS_ANDROID
#endif

#if defined(__AVX512F__)
#define BUILDIT_OS_AVX512
#endif
#if defined(__AVX2__)
#define BUILDIT_OS_AVX2
#endif
#if defined(__AVX__)
#define BUILDIT_OS_AVX
#endif
#if defined(__SSE4_2__)
#define BUILDIT_OS_SSE42
#endif
#if defined(__SSE4_1__)
#define BUILDIT_OS_SSE41
#endif
#if defined(__SSE4__)
#define BUILDIT_OS_SSE4
#endif
#if defined(__SSE3__)
#define BUILDIT_OS_SSE3
#endif
#if defined(__SSE2__)
#define BUILDIT_OS_SSE2
#endif
#if defined(__SSE__)
#define BUILDIT_OS_SSE
#endif

#if defined(__clang__)
#define BUILDIT_COMPILER_CLANG
#endif
#if defined(__GNUC__)
#define BUILDIT_COMPILER_GCC
#endif
#if defined(_MSC_VER)
#define BUILDIT_COMPILER_MSVC
#endif

#ifdef BUILDIT_OS_WINDOWS

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <process.h>
#include <errno.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUILDIT_ENV_PATH_SEPARATOR ";"
#define BUILDIT_OS_ARG_CHAR "/"
#define BUILDIT_OS_OBJ_EXTENSION ".obj"
#define BUILDIT_OS_EXE_EXTENSION ".exe"

#define ASSERT_WINAPI(v) if(!(v)) buildit::error(string(__FILE__) + ":" + to_string(__LINE__) + string(" GetLastError(): ") + to_string(GetLastError()));

#else // !BUILDIT_OS_WINDOWS

#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#define BUILDIT_ENV_PATH_SEPARATOR ":"
#define BUILDIT_OS_ARG_CHAR "-"
#define BUILDIT_OS_OBJ_EXTENSION ".o"
#define BUILDIT_OS_EXE_EXTENSION ""

#endif // BUILDIT_OS_WINDOWS

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <thread>
#include <string>
#include <cstring>
#include <filesystem>

#define UNUSED(v) ((void)v)
#define ASSERT_ERRNO(v) if(!(v)) buildit::error(string(__FILE__) + ":" + to_string(__LINE__) + string(" Errno: ") + to_string(errno) + " - " + strerror(errno));
#define ASSERT_TRUE(v, err_msg) { long long assert_ret_val = (v); if(assert_ret_val) buildit::error(string(__FILE__) + ":" + to_string(__LINE__) + string(" Returned: ") + to_string(assert_ret_val) + string(" ") + (err_msg));}
#define IF_NULL(ptr, than, else) ((ptr) == NULL ? (than) : (else))

#ifdef __cplusplus
#define LIT(t) t
#else
#define LIT(t) (t)
#endif

namespace buildit {

namespace fs = std::filesystem;

enum Log_Level {
    LOG_LEVEL_INFO    = 0,
    LOG_LEVEL_WARNING = 1,
    LOG_LEVEL_ERROR   = 2,
    LOG_LEVEL_DEBUG   = 3,
    LOG_LEVEL_TODO    = 4
};

enum Optimization_Level {
    OPTIMIZATION_NONE  = 0,
    OPTIMIZATION_SIZE  = 1,
    OPTIMIZATION_SPEED = 2
};

struct Pipe {
    #ifdef BUILDIT_OS_WINDOWS
    HANDLE read_handle;
    HANDLE write_handle;
    #else
    int read_fd;
    int write_fd;
    #endif
    // TODO: fields for file closing along with the pipe
};

struct Command {
    fs::path executable;
    std::vector<std::string> arguments;
    fs::path working_dir;
    Pipe stdin;
    Pipe stdout;
    Pipe stderr;
};

struct Process {
    #ifdef BUILDIT_OS_WINDOWS
    PROCESS_INFORMATION process;
    #else
    char** executable_args;
    pid_t pid;
    #endif
};

//////////////////////////////////////////////////////////////////////
// Debugging
// \/
//////////////////////////////////////////////////////////////////////

// Set debug true to enable verbose logging
void set_debug(bool debug);
bool get_debug();

//////////////////////////////////////////////////////////////////////
// Piping
// \/
//////////////////////////////////////////////////////////////////////

Pipe open_pipe();
// \/ Also closes the file if connected
void close_pipe(Pipe pipe);
Pipe pipe_to_file(fs::path file);
Pipe pipe_from_file(fs::path file);
Pipe pipe_from_file_stream(FILE* stream_in, FILE* stream_out);
// \/ pipes to the std handles of the current process
Pipe pipe_stdin();
Pipe pipe_stdout();
Pipe pipe_stderr();
Pipe pipe_invalid();
bool is_pipe_invalid(Pipe pipe);

//////////////////////////////////////////////////////////////////////
// Find common programs
// \/
//////////////////////////////////////////////////////////////////////

fs::path find_compiler(bool cxx);
fs::path find_linker();

//////////////////////////////////////////////////////////////////////
// Rebuild Yourself
// \/
//////////////////////////////////////////////////////////////////////

// Pass argv/string to rebuild file with same name + .cpp with the same compiler as this is compiled with
void rebuild_yourself(char** argv, fs::path compiler = find_compiler(true));
void rebuild_yourself(fs::path exe_path, fs::path compiler = find_compiler(true));

//////////////////////////////////////////////////////////////////////
// Building commands
// \/
//////////////////////////////////////////////////////////////////////

Command default_command(fs::path executable, std::vector<std::string> arguments = std::vector<std::string>(), fs::path working_dir = "", Pipe stdin_pipe = pipe_stdin(), Pipe stdout_pipe = pipe_stdout(), Pipe stderr_pipe = pipe_stderr());
// auto_extensions automatically append .obj for object files and .exe to executable on windows and .o for object and nothing for executable on other platforms
Command get_compile_cmd(fs::path compiler, std::vector<fs::path> source_files, fs::path object, std::vector<fs::path> include_dirs = {}, bool auto_extensions = true, Optimization_Level optimize = OPTIMIZATION_NONE, bool debug = false, int version = 17, bool all_warnings = false, bool pedantic = false, bool native_arch = false);
Command get_link_cmd(fs::path linker, fs::path output_file, std::vector<fs::path> objects, std::vector<fs::path> libraries, std::vector<fs::path> library_dirs, bool auto_extensions = true, Optimization_Level optimize = OPTIMIZATION_NONE, bool debug = false);

//////////////////////////////////////////////////////////////////////
// Executing commands
// \/
//////////////////////////////////////////////////////////////////////

// Returns the exit code if async is NULL and 0 if its not.
// If async is not NULL it just starts a process and pushes it inside.
// If the commands pipes are NULL then they will be opened with the process attached
int execute_cmd(Command cmd, std::vector<Process>* async = NULL);
// Waits for the process to exit and returns the exit code.
int wait_for_process(Process process);

std::vector<int> execute_cmds(std::vector<Command> cmds, std::vector<Process>* async = NULL);
std::vector<int> wait_for_processes(std::vector<Process> processes);

// Chain commands so ones stdout is connected to the next ones stdin.
std::vector<int> chain_cmds(std::vector<Command> cmds, std::vector<Process>* async = NULL);

//////////////////////////////////////////////////////////////////////
// Auto creating ide/editor/lsp settings files
// \/
//////////////////////////////////////////////////////////////////////

// Create settings for ClangD with e.g. include directories
// dir is the directory where clangd settings should be located
void create_clangd_settings(fs::path dir, bool cxx, std::vector<fs::path> include_dirs = {}, int version = 17, bool all_warnings = false, bool pedantic = false);

//////////////////////////////////////////////////////////////////////
// Logging
// \/
//////////////////////////////////////////////////////////////////////

void log(Log_Level level, std::string msg);
void error(std::string desc, int code = 1);
void todo(std::string desc);

//////////////////////////////////////////////////////////////////////
// Paths of common programs
// \/
//////////////////////////////////////////////////////////////////////

fs::path get_system_shell();
fs::path get_current_executable_path();
fs::path get_current_executable_dir();

//////////////////////////////////////////////////////////////////////
// Path utilities
// \/
//////////////////////////////////////////////////////////////////////

// Canonize path (make it absolute and with system separators) if it is not already.
// If executable is true find the path in PATH environment variable.
// If check_empty is true return empty path when input path is empty otherwise on empty input will return base_dir.
// If exec_base_dir is true when the path is not absolute, use the directory of the currently running executable
// as the base otherwise use the working directory.
fs::path canonize_path(fs::path path, bool executable = false, bool check_empty = false, bool exec_base_dir = false);
std::vector<fs::path> canonize_paths(std::vector<fs::path> paths, bool executable = false, bool check_empty = false, bool exec_base_dir = false);
std::vector<fs::path> replace_extensions(std::vector<fs::path> files, fs::path ext);

//////////////////////////////////////////////////////////////////////
// Vector utilities
// \/
//////////////////////////////////////////////////////////////////////

bool vec_any_nonzero(std::vector<int> vec);
template<typename T>
std::vector<T> concat_vec(std::vector<T> vec1, std::vector<T> vec2){
    std::vector<T> vec;
    vec.insert( vec.end(), vec1.begin(), vec1.end() );
    vec.insert( vec.end(), vec2.begin(), vec2.end() );
    return vec;
}

//////////////////////////////////////////////////////////////////////
// String utilities
// \/
//////////////////////////////////////////////////////////////////////

std::string as_string(Pipe pipe);
std::string as_string(Command cmd);
std::string to_lower(std::string str);
std::string to_upper(std::string str);
bool contains(std::string str, std::string token);
bool contains_any(std::string str, std::vector<std::string> tokens);
std::vector<std::string> split_string(std::string str, std::string delim);
std::string concat_str_vec(std::vector<std::string> vec, std::string delim);
// \/ Dont forget to free the array with free_cstr_arr
char** str_vec_to_cstr_arr(std::vector<std::string> vec);
void free_cstr_arr(char** cstr_arr);

//////////////////////////////////////////////////////////////////////
// Path environment variable helpers
// \/
//////////////////////////////////////////////////////////////////////

// Find executable file/program in environment variable PATH 
fs::path find_in_env_path(fs::path filename, bool search_working_dir = true);
fs::path find_first_in_env_path(std::vector<fs::path> filenames, bool search_working_dir = true);

//////////////////////////////////////////////////////////////////////
// OS Specific functions (for crossplatform apps dont use dummy)
// \/
//////////////////////////////////////////////////////////////////////

#ifdef BUILDIT_OS_WINDOWS

std::string command_to_cmd_line(Command cmd);

#else
// Replace fd/fileno (e.g. STDOUT_FILENO) with a file. 
// If input path is empty does nothing.
// Remember to set O_RDONLY, O_WRONLY or O_RDWR in open_flags.
void dup_and_close_fd(int src_fd, int dest_fd, bool ok_if_invalid = false);
#endif

}

#endif
