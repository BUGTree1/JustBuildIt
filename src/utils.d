module utils;
public import std.uni;
public import std.conv;
public import std.file;
public import std.path;
public import std.stdio;
public import std.array;
public import std.ascii;
public import std.traits;
public import std.format;
public import std.process;
public import std.exception;
public import std.algorithm;
public import core.stdc.stdlib;
public import std.range.primitives;
public import std.typecons : Tuple;

void my_exit(int code = 1){
    exit(code);
}

void warning(string desc){
    writefln("WARNING: %s",desc);
}

void warning(int line = __LINE__, string file = __FILE__){
    writefln("WARNING: %s:%d",file,line);
}

void error(string desc){
    writefln("ERROR: %s",desc);
    my_exit();
}

void error(int line = __LINE__, string file = __FILE__){
    writefln("ERROR: %s:%d",file,line);
    my_exit();
}

void todo(string desc,int line = __LINE__, string file = __FILE__){
    writefln("TODO: %s at %s:%d",desc,file,line);
    my_exit();
}

void todo(int line = __LINE__, string file = __FILE__){
    writefln("TODO: %s:%d",file,line);
    my_exit();
}

string slash(T...)(T args) {
    return asNormalizedPath(absolutePath(buildNormalizedPath(args))).array;
}

// REMEMBER not to put thisExePath() as working_dir because it has the FILE in it
int util_exec(string cmd, string[] args = [], string working_dir = getcwd()) {
    string[] cmd_line_array = [cmd] ~ args;
    string cmd_line = escapeShellCommand(cmd_line_array);
    writefln("$ %s",cmd_line);
    Pid shell_pid = spawnShell(
        cmd_line,
        null,
        std.process.Config.none,
        working_dir,
        std.process.nativeShell()
    );
    

    int end_code = wait(shell_pid);
    return end_code;
}