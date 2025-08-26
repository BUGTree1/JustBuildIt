module args;

import std.stdio;
import std.conv;
import commandr;
import buildit;
import utils;

struct Args_info{
    string project_path;
    bool help;
    string template_name;
    bool clean;
    bool rebuild;
    string init_remote_url;
    string puhs_commit_name;
    bool pull;
    bool discard; 
}

Args_info parse_args(string[] args){
    Args_info info;

    // TODO: add all other args
    // TODO: start compiling with std.process
    // TODO: test std.process and paths with unicode characters (wstrings?)
    // REM: to push `git add .` `git commit -m <commit name>` `git push origin main`

    ProgramArgs prog = new Program("test", "1.0")
          .summary("Command line parser")
          .author("John Doe <me@foo.bar.com>")
          .add(new Flag("t", null, "template").name("template"))
          .add(new Option(null, "test", "some teeeest"))
          .add(new Argument("path", "Path to file to edit"))
          .parse(args);

    return info;
}