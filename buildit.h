#pragma once
#ifndef __BUILDIT_H__
#define __BUILDIT_H__

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

namespace buildit {

namespace fs = std::filesystem;
using Process = void*;
using Pipe = void*;

enum Optimization_Level {
    OPTIMIZATION_NONE  = 0,
    OPTIMIZATION_SIZE  = 1,
    OPTIMIZATION_SPEED = 2
};

struct Command {
    fs::path executable;
    std::vector<std::string> arguments;
};

// The compilers and linkers can be specified to e.g. override the priority. Always the first ones have priority.

fs::path find_c_compiler(std::vector<fs::path> compilers = {});
fs::path find_cxx_compiler(std::vector<fs::path> compilers = {});

fs::path find_c_linker(std::vector<fs::path> linkers = {});
fs::path find_cxx_linker(std::vector<fs::path> linkers = {});
fs::path find_linker(std::vector<fs::path> linkers = {});

Command get_compile_cmd(fs::path compiler, std::vector<fs::path> source_files, std::vector<fs::path> include_dirs = {}, bool all_warnings = false, bool pedantic = false);
Command get_link_cmd(fs::path linker, std::vector<fs::path> objects, Optimization_Level optimize = OPTIMIZATION_NONE, bool native_arch = false);

int execute_cmd(Command cmd                            , std::vector<Process>* async = NULL, fs::path working_dir = "", Pipe stdin_pipe = NULL, Pipe stdout_pipe = NULL, Pipe stderr_pipe = NULL);
std::vector<int> execute_cmds(std::vector<Command> cmds, std::vector<Process>* async = NULL, fs::path working_dir = "", Pipe stdin_pipe = NULL, Pipe stdout_pipe = NULL, Pipe stderr_pipe = NULL);

int wait_for_process(Process process);
int wait_for_processes(std::vector<Process> processes);

//create settings with e.g. include directories
void create_vscode_settings(std::string cmd, fs::path dir);
void create_clangd_settings(std::string cmd, fs::path dir);

std::string concat_string_vector(std::vector<std::string> vec, std::string delim);
std::vector<std::string> split_string(std::string str, std::string delim);

//find file in environment variable PATH
fs::path find_in_env_path(fs::path path);
fs::path find_first_in_env_path(std::vector<fs::path> paths);
fs::path get_system_shell();

void error(std::string desc, int code = 1);
void warning(std::string desc);
void todo(std::string desc);

}

#if defined(_WIN32) || defined(_WIN64)
#define BUILDIT_OS_WINDOWS
#endif
#if defined(__APPLE__) || defined(__MACH__)
#define BUILDIT_OS_MACOS
#define BUILDIT_OS_POSIX
#endif
#if defined(__linux__) || defined(linux) || defined(__linux)
#define BUILDIT_OS_LINUX
#define BUILDIT_OS_POSIX
#endif
#if defined(BSD) || defined(__FreeBSD__)
#define BUILDIT_OS_BSD
#define BUILDIT_OS_POSIX
#endif
#if defined(__sun)
#define BUILDIT_OS_SOLARIS
#define BUILDIT_OS_POSIX
#endif
#if defined(__ANDROID__)
#define BUILDIT_OS_ANDROID
#endif

#ifdef BUILDIT_OS_WINDOWS
#define BUILDIT_ENV_PATH_SEPARATOR ";"
#define BUILDIT_OS_ARG_CHAR "/"
#else
#define BUILDIT_ENV_PATH_SEPARATOR ":"
#define BUILDIT_OS_ARG_CHAR "-"
#endif

#endif
