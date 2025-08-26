import std.process;
import std.traits;
import std.stdio;
import std.conv;
import std.file;
import config;
import utils;
import args;

version(Windows){
    bool platform_windows = true;
} else {
    bool platform_windows = false;
}

int main(string[] args){
    Args_info args_info = parse_args(args);

    string config_path = "./project_templates/cpp/buildme.yaml";
    Config_Parsed cfg = parse_config(config_path);
    writeln(cfg);
    return 0;
}