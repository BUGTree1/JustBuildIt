module utils;
public import std.stdio;
public import std.exception;
public import std.process;
public import std.traits;
public import std.conv;
public import std.file;
public import std.path;
public import std.format;
public import std.range.primitives;
public import core.stdc.stdlib;

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
    return buildNormalizedPath(args);
}