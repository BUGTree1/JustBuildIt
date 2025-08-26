module utils;
import std.stdio;
import std.exception;
import core.stdc.stdlib;

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