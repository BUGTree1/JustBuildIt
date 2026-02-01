#pragma once
#ifndef __BUILDIT_H__
#define __BUILDIT_H__

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <thread>
#include <string>
#include <cstring>
#include <filesystem>

#if defined(_WIN32) || defined(_WIN64)
#define BUILDIT_OS_WINDOWS
#endif
#if defined(__APPLE__) || defined(__MACH__)
#define BUILDIT_OS_MACOS
#endif
#if defined(__linux__) || defined(linux) || defined(__linux)
#define BUILDIT_OS_LINUX
#endif
#if defined(BSD) || defined(__FreeBSD__)
#define BUILDIT_OS_BSD
#endif
#if defined(__sun)
#define BUILDIT_OS_SOLARIS
#endif
#if defined(__ANDROID__)
#define BUILDIT_OS_ANDROID
#endif

#ifdef BUILDIT_OS_WINDOWS
#include <windows.h>

#define BUILDIT_ENV_PATH_SEPARATOR ";"
#define BUILDIT_OS_ARG_CHAR "/"

#define ASSERT_WINAPI(v) if(!(v)) buildit::error(string(__FILE__) + ":" + to_string(__LINE__) + string(" GetLastError(): ") + to_string(GetLastError()));
#else // BUILDIT_OS_WINDOWS
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#define BUILDIT_ENV_PATH_SEPARATOR ":"
#define BUILDIT_OS_ARG_CHAR "-"

#define ASSERT_ERRNO(v) if(!(v)) buildit::error(string(__FILE__) + ":" + to_string(__LINE__) + string(" Errno: ") + to_string(errno) + " - " + strerror(errno));
#endif // BUILDIT_OS_WINDOWS

namespace buildit {

namespace fs = std::filesystem;

struct Process {
    #ifdef BUILDIT_OS_WINDOWS
    PROCESS_INFORMATION pi;
    #else // BUILDIT_OS_WINDOWS
    char** executable_args;
    pid_t pid;
    #endif // BUILDIT_OS_WINDOWS
};

enum Optimization_Level {
    OPTIMIZATION_NONE  = 0,
    OPTIMIZATION_SIZE  = 1,
    OPTIMIZATION_SPEED = 2
};

struct Command {
    fs::path executable;
    std::vector<std::string> arguments;
    fs::path working_dir = "";
    fs::path stdin_file = "";
    fs::path stdout_file = "";
    fs::path stderr_file = "";
};

// The compilers and linkers can be specified to e.g. override the priority. Always the first ones have priority.
fs::path find_c_compiler(std::vector<fs::path> compilers = {});
fs::path find_cxx_compiler(std::vector<fs::path> compilers = {});

fs::path find_c_linker(std::vector<fs::path> linkers = {});
fs::path find_cxx_linker(std::vector<fs::path> linkers = {});
fs::path find_linker(std::vector<fs::path> linkers = {});

Command get_compile_cmd(fs::path compiler, std::vector<fs::path> source_files, std::vector<fs::path> include_dirs = {}, bool all_warnings = false, bool pedantic = false);
Command get_link_cmd(fs::path linker, std::vector<fs::path> objects, Optimization_Level optimize = OPTIMIZATION_NONE, bool native_arch = false);

// Executes a Command. Returns the exit code if async is NULL and 0 if its not. If async is not NULL it just starts a process and pushes it inside.
int execute_cmd(Command cmd, std::vector<Process>* async = NULL);
// Waits for the process to exit and returns the exit code.
int wait_for_process(Process process);

std::vector<int> execute_cmds(std::vector<Command> cmds, std::vector<Process>* async = NULL);
std::vector<int> wait_for_processes(std::vector<Process> processes);
// chain commands so ones stdout is connected to the next ones stdin than wait for all them to exit. Returns the vector of all exit codes.
std::vector<int> chain_commands(std::vector<Command> cmds, std::vector<Process>* async = NULL);

//create settings with e.g. include directories
void create_vscode_settings(std::string cmd, fs::path dir);
void create_clangd_settings(std::string cmd, fs::path dir);

bool vec_any_nonzero(std::vector<int> vec);
std::string concat_str_vec(std::vector<std::string> vec, std::string delim);
std::vector<std::string> split_string(std::string str, std::string delim);
// \/ Dont forget to free the array with the next function
char** str_vec_to_cstr_arr(std::vector<std::string> vec);
void free_cstr_arr(char** cstr_arr);

template<typename T>
std::vector<T> concat_vec(std::vector<T> vec1, std::vector<T> vec2){
    std::vector<T> vec;
    vec.insert( vec.end(), vec1.begin(), vec1.end() );
    vec.insert( vec.end(), vec2.begin(), vec2.end() );
    return vec;
}

// Find executable file/program in environment variable PATH
fs::path find_in_env_path(fs::path filename);
fs::path find_first_in_env_path(std::vector<fs::path> filenames);
// Canonize path (make it absolute and with system separators) if it is not already.
// If executable is true find the path in PATH environment variable.
// If check_empty is true return empty path when input path is empty otherwise on empty input will return base_dir.
// If exec_base_dir is true when the path is not absolute, use the directory of the currently running executable
// as the base otherwise use the working directory.
fs::path canonize_path(fs::path path, bool executable = false, bool check_empty = false, bool exec_base_dir = false);
fs::path get_system_shell();
fs::path get_current_executable_path();
fs::path get_current_executable_dir();

void error(std::string desc, int code = 1);
void warning(std::string desc);
void todo(std::string desc);

#ifdef BUILDIT_OS_WINDOWS

#else
// Replace fd/fileno (e.g. STDOUT_FILENO) with a file. 
// If input path is empty does nothing.
// Remember to set O_RDONLY, O_WRONLY or O_RDWR in open_flags.
void set_fd_as_file(fs::path file, int fd, int open_flags, int create_file_permissions = 0666);
#endif

}

#endif
