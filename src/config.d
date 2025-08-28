module config;

import dyaml;
import utils;
import args;

// Converts Nodes "n" SubNode "str" to type "T"
// ONLY if it is an array and its not a string
T node_to_val(T)(Node n,string str)
if (isArray!T && !(isSomeString!T)) {
    T arr;
    alias Elem = ElementType!T;
    foreach(Elem child;n[str]){
        arr ~= child;
    }
    return arr;
}

// Converts Nodes "n" SubNode "str" to type "T"
// ONLY if it is not an array or its a string
T node_to_val(T)(Node n,string str)
if (!(isArray!T) || (isSomeString!T)) {
    return n[str].as!T;
}

struct Config_Parsed{
    bool     create_vscode_settings;
    bool     run_after_build;
    bool     auto_out_file_ext;
    string   build_type;
    string   proj_name;
    string   proj_version;
    string   file_name;
    string   compiler;
    string   linker;
    string[] flags;
    string[] compiler_flags;
    string[] linker_flags;
    string   output_path;
    string   source_path;
    string[] libs;
    string[] pkgconf_libs;
    string[] lib_paths;
    string[] include_paths;
    string[] run_args;
    string[] exec_postbuild;
    string[] exec_prebuild;
}

Config_Parsed parse_config(string path){
    Node config = Loader.fromFile(path).load();

    Config_Parsed cfg;

    cfg.create_vscode_settings = node_to_val!(bool    )(config, "create_vscode_settings");
    cfg.run_after_build        = node_to_val!(bool    )(config, "run_after_build");
    cfg.auto_out_file_ext      = node_to_val!(bool    )(config, "auto_out_file_ext");
    cfg.build_type             = node_to_val!(string  )(config, "build_type");
    cfg.proj_name              = node_to_val!(string  )(config, "proj_name");
    cfg.proj_version           = node_to_val!(string  )(config, "proj_version");
    cfg.file_name              = node_to_val!(string  )(config, "file_name");
    cfg.compiler               = node_to_val!(string  )(config, "compiler");
    cfg.linker                 = node_to_val!(string  )(config, "linker");
    cfg.flags                  = node_to_val!(string[])(config, "flags");
    cfg.compiler_flags         = node_to_val!(string[])(config, "compiler_flags");
    cfg.linker_flags           = node_to_val!(string[])(config, "linker_flags");
    cfg.output_path            = node_to_val!(string  )(config, "output_path");
    cfg.source_path            = node_to_val!(string  )(config, "source_path");
    cfg.libs                   = node_to_val!(string[])(config, "libs");
    cfg.pkgconf_libs           = node_to_val!(string[])(config, "pkgconf_libs");
    cfg.lib_paths              = node_to_val!(string[])(config, "lib_paths");
    cfg.include_paths          = node_to_val!(string[])(config, "include_paths");
    cfg.run_args               = node_to_val!(string[])(config, "run_args");
    cfg.exec_postbuild         = node_to_val!(string[])(config, "exec_postbuild");
    cfg.exec_prebuild          = node_to_val!(string[])(config, "exec_prebuild");
    return cfg;
}