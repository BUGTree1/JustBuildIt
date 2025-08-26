import std.stdio;
import std.conv;
import std.process;
import std.file;
import dyaml;
import utils;
import args;

version(Windows){
    bool platform_windows = true;
} else {
    bool platform_windows = false;
}

int main(string[] args){
    Args_info args_info = parse_args(args);

    //config = parseTOML(cast(string)read("project_templates/buildme.toml"));

    //writeln(config["flags"]);
    return 0;
}